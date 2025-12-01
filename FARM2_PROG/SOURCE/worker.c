//
//  Created by Natalia Ceccarini
//

#include <worker.h>
#include <collector.h>
#include <myThreadPool.h>
#include <util.h>
#include <string.h>

pthread_mutex_t socket_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @function calculate_sum
 * @brief The function compute implements the work that a worker thread must perform.
 * The function takes as input the pathname of a regular file and socket file descriptor.
 * The file is interpreted as a binary file containing N long integers.
 * The long integers contained in the file are read one by one, and a computation is performed with them.
 * The calculation that must be performed on each file is as follows:
 * result = summation (for i ranging from 0 to N-1) of (i * file[i]).
 * N is the number of long integers present in the file, and file[i] is the i-th long integer.
 * The result is sent to the collector by writing it to the socket
 * @param arg1 pathname of a regular file
 * @param arg2 socket file descriptor
 */
void calculate_sum(void *arg1, void *arg2)
{
    char *filename = (char *)arg1;
    int fd_sock = *(int *)arg2;
    //controllo se l'argomento è valdo
    if (filename == NULL) {
#ifdef DEBUG
        HANDLE_ERROR("argomento non valido nella funzione calculateSum");
#endif
        return;
    }
    fflush(stdout);

#ifdef DEBUG
    printf("nome file: %s\n", filename);
#endif

    //apro il file in lettura
    FILE *filePtr;
    filePtr = fopen(filename, "rb");

    if (filePtr == NULL) {
#ifdef DEBUG
        HANDLE_ERROR("fopen nella funzione calculateSum");
#endif
        return;
    }

    // Variabili per leggere dal file
    long currentNumber;
    long sum = 0;
    long index = 0;

    // Reset errno
    errno = 0;

    // Leggi uno alla volta dal file binario
    while ((fread(&currentNumber, sizeof(long), 1, filePtr)) != 0) {
        fflush(stdout);
        sum += index * currentNumber;
        index++;
    }


    if(feof(filePtr) != 0)
    {
        //numero di caratteri nel nome del file
        int length = strlen(filename)+1;

        LOCK_RETURN(&socket_mutex, );
        int terminazione = 0;
        if (writen(fd_sock, &terminazione, sizeof(int)) <= 0)
        {
            UNLOCK_RETURN(&socket_mutex,);
            fclose(filePtr);
#ifdef DEBUG
            HANDLE_ERROR("write termina on socket");
#endif
            return;
        }
        if(writen(fd_sock, &sum, sizeof(long)) <= 0) {
            UNLOCK_RETURN(&socket_mutex, );
            fclose(filePtr);
#ifdef DEBUG
            HANDLE_ERROR("write sum on socket");
#endif
            return ;
        }
        if( writen(fd_sock, &length, sizeof(int)) <= 0 )
        {

            UNLOCK_RETURN(&socket_mutex, );
            fclose(filePtr);
#ifdef DEBUG
            HANDLE_ERROR("write lenght on socket");
#endif
            return;
        }
        if (writen(fd_sock, filename, sizeof(char) * length) <= 0)
        {
            UNLOCK_RETURN(&socket_mutex, );
            fclose(filePtr);
#ifdef DEBUG
            HANDLE_ERROR("write filename on socket");
#endif
            return;
        }

        UNLOCK_RETURN(&socket_mutex, );

        fclose(filePtr);
        return ;
    }
#ifdef DEBUG
    HANDLE_ERROR("fread in calculateSum");
#endif
    fclose(filePtr);



}
