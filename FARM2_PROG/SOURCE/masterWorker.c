//
//  Created by Natalia Ceccarini
//

#define _GNU_SOURCE


#include <sys/socket.h>
#include <sys/un.h>
#include <worker.h>
#include <myThreadPool.h>
#include <util.h>
#include <collector.h>


int termina = 0;
int fd_skt;
pid_t pid;

ThreadPool *tpool = NULL;

/**
 * @function sigHandler
 * @brief function executed by the signal handler thread listening for signals.
 * @param arg bit mask
 */
static void *sigHandler(void *arg)
{
    sigset_t *set = (sigset_t*)arg;

    while(!termina)
    {
        int sig;
        int r = sigwait(set, &sig);

        if (r != 0) {
            errno = r;
            perror("FATAL ERROR 'sigwait'");
            return NULL;
        }

        switch(sig)
        {
            case SIGINT:
            case SIGTERM:
            case SIGQUIT:
            case SIGHUP:
                termina = 1;
                break;
                //return NULL;
            case SIGUSR1:
#ifdef DEBUG
                printf("signal SIGUSR1\n");
#endif
                if (tpool != NULL)
                {
                    LOCK_RETURN(&tpool->lockSignal, NULL);
                    tpool->ctrAddThreads++;
                    SIGNAL_RETURN(&tpool->cond_counter, NULL);
                    UNLOCK_RETURN(&tpool->lockSignal, NULL);

                }
                break;

            case SIGUSR2:
#ifdef DEBUG
                printf("signal SIGUSR2\n");
#endif
                if (tpool != NULL)
                {
                    LOCK_RETURN(&tpool->lockSignal, NULL);
                    tpool->ctrRemoveThreads++;
                    SIGNAL_RETURN(&tpool->cond_counter, NULL);
                    UNLOCK_RETURN(&tpool->lockSignal, NULL);


                }
                break;
            default:  ;
        }
    }


    fflush(stdout);

    return NULL;
}

/**
 * @function find
 * @brief explore the file system starting from the directory passed as a parameter,
 *           searching for regular files to submit to the thread pool
 * @param dir_name name directory
 * @param delay interval of time that must pass between one request and the next
 * @param fd_skt descriptor of socket file
 * @return 1 success, -1 failed
 */

int find(char *dir_name, long delay){

    DIR *dir;
    if((dir = opendir(dir_name)) == NULL)
    {
        print_error("Errore aprendo la directory %s\n", dir_name);
        return -1;
    }
    else
    {
        struct dirent *entry;
        while((errno = 0, entry = readdir(dir)) != NULL)
        {
            struct stat statbuf;
            char *new_path = calloc(MAX_SIZE, sizeof(char *));
            new_path[0] = '\0';
            strcat(new_path, dir_name);
            if (new_path[strlen(new_path) - 1] != '/')
            {
                strcat(new_path, "/");
            }

            strcat(new_path, entry->d_name);

            if(!termina)
            {
                if(stat(new_path, &statbuf) == -1)
                {
                    perror("stat");
                    print_error("Errore facendo stat di %s\n", entry->d_name);
                    return -1;
                }
                if (S_ISDIR(statbuf.st_mode))
                {
                    // se è una directory
                    if (!isdot(entry->d_name) )
                    { // se non è la directory corrente o padre
                        find(new_path, delay);
                        // chiamo la find sul sottoalbero
                    }
                }
                else
                {
                    if(S_ISREG(statbuf.st_mode))
                    {
                        msleep(delay);
                        //devo fare la sleep
                        if(!termina)
                        {
                            //printf("il file trovato è regolare e lo aggiungo al pool thread %s \n", new_path);
                            fflush(stdout);
                            if (addJobToPool(tpool, &calculate_sum, new_path, &fd_skt) != 0)
                            {
                                return -1;
                            }

                        }
                    }
                }
            }
            free(new_path);
        }
        if (errno != 0)
        {
            perror("readdir");
            closedir(dir);
            return -1;
        }
        closedir(dir);
        return 1;
    }
}






/**
 * @function exitHandler
 * @brief Function executed at program exit,
 *        which destroies threadpool, sends termination message to collector,writes the number of active threads at the moment of thread pool destruction to the file 'nworkeratexit' and cancel socket file.
 */
void exitHandler()
{
    int numThreads;
    numThreads= destroyPool(tpool, 0);
    
    int terminate = 1;
    writen(fd_skt, &terminate, sizeof(int));
    close(fd_skt);


    int status;
    pid_t child = waitpid(pid, &status, 0); // Wait for the child process to terminate

    if (child == -1)
    {
        perror("waitpid");

    }
    // open file in write mode
    FILE *file = fopen("nworkeratexit.txt", "w");
    if (file == NULL) {
        HANDLE_ERROR("nell'apertura del file 'nworkeratexit.txt'");

    }
    else
    {
        // write the number threads in the file
        fprintf(file, "%d", numThreads);


        // close file
        fclose(file);
    }

    //cancel socket file
    unlink(SOCKNAME);
#ifdef DEBUG
    printf("cancellato file socket \n");
#endif



}

int main(int argc, char* argv[]) {


    if (atexit(exitHandler) != 0)
    {
        HANDLE_ERROR("atexit");
        exit(EXIT_FAILURE);
    }

    //set the bit mask of signals to be masked.
    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGHUP);
    sigaddset(&mask, SIGPIPE);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);



    struct sigaction sa;
    // reset the struct
    memset(&sa, 0, sizeof(sa));

    // ignore sigpipe
    int notused;
    sa.sa_handler = SIG_IGN;
    SYSCALL_EXIT("sigaction", notused, sigaction(SIGPIPE, &sa, NULL), "sigaction", "");


    if (pthread_sigmask(SIG_BLOCK, &mask, &oldmask) != 0)
    {
        fprintf(stderr, "ERRORE FATALE -> pthread_sigmask\n");
        return EXIT_FAILURE;
    }


    // create the child process
    pid = fork();

    if (pid == -1) {
        HANDLE_ERROR("fork()");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    { // child process
        // Declaration of arguments for the new program
        char *argv_for_program[] = {"collector", NULL};

        // Execute collector program
        if (execvp("./collector", argv_for_program) == -1)
        {
            // Error execvp
            HANDLE_ERROR("execvp()");
            exit(EXIT_FAILURE);
        }
    }
    else
    {

        pthread_t sighandler_thread;

        //Thread handler as the signal handler.
        if(pthread_create(&sighandler_thread, NULL, sigHandler, &mask) != 0)
        {
            fprintf(stderr, "Error in creating the signal handler thread\n");
            exit(EXIT_FAILURE);
        }

#ifdef DEBUG
            printf("[SOCKET:INFO] socket inizializing\n");
#endif
        SYSCALL_EXIT("socket", fd_skt, socket(AF_UNIX, SOCK_STREAM, 0), "socket", "");
        // connect socket
        struct sockaddr_un serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));

        serv_addr.sun_family = AF_UNIX;
        strncpy(serv_addr.sun_path, SOCKNAME, UNIX_PATH_MAX);
        while (connect(fd_skt, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        {
#ifdef DEBUG
            printf("[SOCKET:INFO] try connect socket\n");
#endif

            if (errno == ENOENT)
                msleep(50); /* sock non esiste */
            else

                exit(EXIT_FAILURE);
        }

        int opt;
        int numThreads = NTHREAD;
        int qlen = QLEN;
        char *directory = NULL;
        int delay = DELAY;
        int res;



        while((opt = getopt(argc, argv, "n:q:d:t:")) != -1){
            switch (opt) {
                case 'n':
                    res = isNumber(optarg, &numThreads);

                    if (res == 0){
                        HANDLE_ERROR("the -n argument must be a number");
                        exit(EXIT_FAILURE);

                    }
                    if (res == 2){
                        fprintf(stderr, "ERROR: %d", errno);
                        exit(EXIT_FAILURE);
                    }
                    if (numThreads < 0)
                    {
                        HANDLE_ERROR("the -n argument must be a positive number");
                        exit(EXIT_FAILURE);
                    }

                    break;
                case 'q':
                    res = isNumber(optarg, &qlen);
                    if (res == 0){
                        HANDLE_ERROR("the -q argument must be a number");
                        exit(EXIT_FAILURE);
                    }
                    if (res == 2){
                        fprintf(stderr, "ERROR: %d", errno);
                        exit(EXIT_FAILURE);
                    }
                    if (qlen < 0)
                    {
                        HANDLE_ERROR("the -q argument must be a positive number");
                        exit(EXIT_FAILURE);
                    }

                    break;
                case 'd':
                    directory = optarg;
                    break;
                case 't':
                    res = isNumber(optarg, &delay);
                    if (res == 0){
                        HANDLE_ERROR("the -t argument must be a number");
                        exit(EXIT_FAILURE);
                    }
                    if (res == 2){
                        fprintf(stderr, "ERROR: %d", errno);
                        exit(EXIT_FAILURE);
                    }
                    if (delay < 0)
                    {
                        HANDLE_ERROR("the -n argument must be a positive number");
                        exit(EXIT_FAILURE);
                    }

                    break;
                case '?':
                    HANDLE_ERROR("the -c option is not handled");
                    exit(EXIT_FAILURE);
                    break;
                case ':':
                    HANDLE_ERROR("the '-%c' option requires an argument");
                    exit(EXIT_FAILURE);
                    break;
                default:;

            }
        }

    #ifdef DEBUG
        printf("Number of threads: %d\n", numThreads);
        printf("Queue length: %d\n", qlen);
        printf("Directory name: %s\n", (directory == NULL) ? "Not specified" : directory);
        printf("Delay between requests: %d milliseconds\n", delay);
    #endif




        fflush(stdout);
        tpool = createPool(numThreads, qlen);

        if(!tpool)
        {

            HANDLE_ERROR("createThreadPool");
            exit(EXIT_FAILURE);
        }

#ifdef DEBUG
            printf("[SOCKET:INFO] socket connected\n");
#endif
        // socket connecetd
        for (int index = optind; index < argc; index++)
        {

            msleep(delay);
            struct stat statbuf;

            if(stat(argv[index], &statbuf) == -1)
            {


                print_error("Errore Facendo stat del nome argv %s: ", argv[index]);
                exit(EXIT_FAILURE);

               

            }


            if (!termina && S_ISREG(statbuf.st_mode))
            {
                if(addJobToPool(tpool, &calculate_sum, argv[index], &fd_skt) != 0)
                {
               		exit(EXIT_FAILURE);
                }

                //aggiungere il task al threadpool
            }
            if(termina)break;

        }



        if(directory != NULL && !termina)
        {
            struct stat statbuf;
            if(stat(directory, &statbuf) == -1)
            {

                print_error("Errore Facendo stat del nome %s: ", directory);
                exit(EXIT_FAILURE);

               

            }
            if(!S_ISDIR(statbuf.st_mode)){

                fprintf(stderr, "%s non è una directory\n", directory);
                exit(EXIT_FAILURE);

                
            }
            else
            {
                if(find(directory, delay) == -1)
                {
                	exit(EXIT_FAILURE);
                }
            }



        }

        pthread_kill(sighandler_thread, SIGINT);
        pthread_join(sighandler_thread, NULL);


       

        
        fflush(stdout);


        exit(EXIT_SUCCESS);
    }


}

