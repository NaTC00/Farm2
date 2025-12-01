//
// Created by Natalia Ceccarini
//


#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 2001112L
#endif


#ifndef _UTIL_H
#define _UTIL_H


// include
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <limits.h>
#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>
#include <stddef.h>
#include <getopt.h>


#define EXTRA_LEN_PRINT_ERROR   512
#define NTHREAD 4 // default number threads
#define QLEN 8    // default length of the concurrent queue of pending tasks
#define DELAY 0   // task submission distance from the master to the workers expressed in ms



// MAX_SIZE
#if !defined(MAX_SIZE)
#define MAX_SIZE 256
#endif



#define HANDLE_ERROR(msg)                                       \
    perror("ERROR: "msg" | errno");

#define SYSCALL_EXIT(name, r, sc, str, ...)	\
    if((r=sc) == -1) {						\
		perror(#name);						\
		int errno_copy = errno;				\
		print_error(str, __VA_ARGS__);		\
		exit(errno_copy);					\
    }

static inline void print_error(const char * str, ...) {
    const char err[] = "ERROR: ";
    va_list argp;
    char * p=(char *)malloc(strlen(str)+strlen(err)+EXTRA_LEN_PRINT_ERROR);
    if (!p) {
        perror("malloc");
        fprintf(stderr,"FATAL ERROR in the function 'print_error'\n");
        return;
    }
    strcpy(p,err);
    strcpy(p+strlen(err), str);
    va_start(argp, str);
    vfprintf(stderr, p, argp);
    va_end(argp);
    free(p);
}


/**
 * @brief Checks whether the string passed as the first argument, s, represents a number and possibly converts it to a number by storing it in the 				second argument n
 * @param s is a string
 * @param n is a pointer to int
 * @return  0 ok,  1 not is number,   2 overflow/underflow
 */
static inline int isNumber(const char* s, int* n) {
    if (s==NULL) return 0;
    if (strlen(s)==0) return 0;
    errno=0;
    int val = atoi(s);
    if (errno == ERANGE) return 2;    // overflow/underflow
    if (val != 0 || (s[0] == '0' && strlen(s) == 1)) {
        *n = val;
        return 1;   // successo
    }
    return 0;   // non e' un numero
}

/**
 * funzione isdot
 * @brief utility function that checks if the directory passed as a parameter is '.' or '..'
 * @param dir pathname of a directory
 * @return 1 if the string passed as a parameter identifies the directory '.' or '..', otherwise returns 0
 */
int isdot(const char dir[]);

/**
 * funzione msleep
 * @brief pauses execution of the calling thread for a specified number of milliseconds.
 * @param tms number of milliseconds
 * @return  0 if nanosleep completed successfully. Otherwise, -1 and errno will be set to indicate the error.
 */
int msleep(long tms);

#endif //FARM2_UTILS_H
