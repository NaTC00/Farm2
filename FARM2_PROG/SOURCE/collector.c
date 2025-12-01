//
// Created by Natalia Ceccarini
//
#include <collector.h>
#include <list.h>
#include <util.h>


int termina = 0;
FileNodePtr head = NULL;

int aggiorna(fd_set *set)
{
    int max_fd = -1; // Initialize max_fd to an invalid value

    for (int i = 0; i < FD_SETSIZE; ++i)
    {
        if (FD_ISSET(i, set))
        {
            if (i > max_fd)
            {
                max_fd = i; // Update max_fd if the current FD is greater
            }
        }
    }

    return max_fd; // Return the maximum file descriptor value
}

//function that prints the list of data received every second of execution
void *printListPerSecond()
{


    while (1)
    {
        msleep(1000);

        if(termina){
            printList(head);
            fflush(stdout);

            break;
        }else
        {

            printList(head);
            fflush(stdout);
        }
    }
    return NULL;
}

/**
 * @function exitHandler
 * @brief Function executed at program exit,
 *        free list and cancel socket file.
 */
void exitHandler()
{
    freeList(&head);
    //cancel socket file
    unlink(SOCKNAME);
#ifdef DEBUG
    printf("delete file socket \n");
#endif

}


int main(int argc, char *argv[])
{
    if (atexit(exitHandler) != 0)
    {
        HANDLE_ERROR("atexit");
        exit(EXIT_FAILURE);
    }

    int listenfd;
    int fdmax;



    // create a new socket of type SOCK_STREAM in domain AF_UNIX, if protocol we use the default protocol
    if((listenfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        HANDLE_ERROR("socket");
        unlink(SOCKNAME);
        return EXIT_FAILURE;
    }
    struct sockaddr_un serv_addr;
    memset(&serv_addr, 0, sizeof(struct sockaddr_un));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, SOCKNAME, UNIX_PATH_MAX);

    if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        HANDLE_ERROR("bind");
        unlink(SOCKNAME);
        exit(EXIT_FAILURE);
    }
    if(listen(listenfd, MAXBACKLOG) == -1)
    {
        HANDLE_ERROR("listen");
        unlink(SOCKNAME);
        exit(EXIT_FAILURE);
    }
    fd_set set, tmpset;
    int msglength = 0;
    long result = 0;
    char *msg = NULL;
    int n;


    FD_ZERO(&set);
    FD_ZERO(&tmpset);

    FD_SET(listenfd, &set); // aggiungo il listener fd al master set

    fdmax = listenfd;

    pthread_t tid;
    if(pthread_create(&(tid), NULL, printListPerSecond, NULL) != 0)
    {
        errno = EFAULT;
        exit(EXIT_FAILURE);
    }

    while (!termina)
    {
        tmpset = set;

        // use select to check if there are any file descriptors ready for reading
        if(select(fdmax+1, &tmpset, NULL, NULL, NULL) == -1)
        {
            HANDLE_ERROR("select");
            break;
        }

        //understand from which fd we received a request
        for (int i = 0; i <= fdmax; i++)
        {
            if(FD_ISSET(i, &tmpset))
            {
                int fd_conn;
                if(i == listenfd)
                {
                    //a new connection request
                    if((fd_conn = accept(listenfd, NULL, NULL)) == -1)
                    {
                        HANDLE_ERROR("accept");
                        break;
                    }
                    FD_SET(fd_conn, &set);
                    if(fd_conn > fdmax)
                    {
                        fdmax = fd_conn;
                    }
                }
                else
                {
                    if ((n = readn(i, &termina, sizeof(int))) == -1)
                    {
                        HANDLE_ERROR("readn");
                        break;
                    }

                    // Check for EOF condition (client closed the connection)
                    if (n == 0)
                    {
                        //printf("Connection closed by client on socket %d\n", i);
                        //  Remove the socket from the set and update fdmax
                        FD_CLR(i, &set);
                        fdmax = aggiorna(&set);
                        close(i); // Close the socket
                        break;
                        termina = 1;



                    }

                    //if the termina value is not 1 I read the other data received
                    if(!termina)
                    {
                        //read the result corresponding to the sum of the long integers read from the file
                        if((n = readn(i, &result, sizeof(long))) == -1)
                        {
                            HANDLE_ERROR("readn");
                            break;
                        }

                        //read the length of pathname
                        if((n = readn(i, &msglength, sizeof(int))) == -1)
                        {
                            HANDLE_ERROR("readn");
                            break;
                        }
                        msg = malloc(sizeof(char)*msglength);
                        if(msg == NULL)
                        {
                            break;
                        }

                        //read pathname
                        if((n = readn(i, msg, msglength)) == -1)
                        {
                            HANDLE_ERROR("readn");
                            free(msg);
                            break;
                        }


                        FileNodePtr node = newNode(result, msglength, msg);
                        if(node == NULL)
                        {
                            break;
                        }

                        //insert new node in list
                        insertNode(&head, node);


                        free(msg);
                    }
                    else
                    {
                       break;
                    }
                }

            }
        }
    }
    if(pthread_join(tid, NULL) != 0)
    {
        errno = EFAULT;

        return EXIT_FAILURE;
    }

    exit(EXIT_SUCCESS);

}
