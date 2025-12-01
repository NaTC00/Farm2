//
//  Created by Natalia Ceccarini
//

#ifndef FARM2_MYTHREADPOOL_H
#define FARM2_MYTHREADPOOL_H

#include<pthread.h>




/* A singly linked list of threads. This list
 * gives tremendous flexibility managing the
 * threads at runtime.
 */
typedef struct ThreadList {
    pthread_t thread; // The thread object
    struct ThreadList *next; // Link to next thread
} ThreadList;

/* A singly linked list of worker functions. This
 * list is implemented as a queue to manage the
 * execution in the pool.
 */
typedef struct Job {
    void (*function)(void *, void *); // The worker function
    void *arg1; // First argument to the function
    void *arg2; // Second argument to the function
    struct Job *next; // Link to next Job
} Job;



/* The core pool structure. This is the only
 * user accessible structure in the API. It contains
 * all the primitives necessary to provide
 * synchronization between the threads, along with
 * dynamic management and execution control.
 */
struct threadPool {
    /* The FRONT of the thread queue in the pool.
     * It typically points to the first thread
     * created in the pool.
     */
    ThreadList * threads;

    /* The REAR of the thread queue in the pool.
     * Points to the last, and most young thread
     * added to the pool.
     */
    ThreadList * rearThreads;

    /*
     * Number of total threads in the pool.
     */
    int numThreads;

    /*
     * number of worker threads still active
     */
    int numthreadsalive;

    /*
     * Job queue size
     */
    int queueSize;

    /* The indicator which indicates the number
     * of threads to remove.
     */
    int removeThreads;

    /*
     * The counter represents the count of SIGUSR2 signals,
     * indicating that threads should be decremented by the value of the counter.
     */
    int ctrRemoveThreads;

    /*
     * The counter represents the count of SIGUSR1 signals,
     * indicating that threads should be increased by the value of the counter.
     */
    int ctrAddThreads;


    /* The main mutex for the job queue. All
     * operations on the queue is done after locking
     * this mutex to ensure consistency.
     */
    pthread_mutex_t lock;

    /*
     *  This mutex ensures thread safety
     *  for accessing and updating the signal counter variables.
     */
    pthread_mutex_t lockSignal;


    /*
    * condition variable on which the consuming threads wait
    */
    pthread_cond_t cond_consumer;

    /*
     * condition variable on which the productor threads wait
     */

    pthread_cond_t cond_productor;

    /*
     * condition variable on which the manger threads wait to signal
     */

    pthread_cond_t cond_counter;

    /* Used to assign unique thread IDs to each
     * running threads. It is an always incremental
     * counter.
     */
    int threadID;

    /*
     * tid of the thread that handles increasing and removing threads
     */

    pthread_t tid_manager;

    /* The FRONT of the job queue, which typically
     * points to the job to be executed next.
     */
    Job *FRONT;

    /* The REAR of the job queue, which points
     * to the job last added in the pool.
     */
    Job *REAR;

    /*
     * number of jobs in the queue
     */
    int countJob;

    /* variable for thread termination.
     * If > 0 the exit protocol has started,
     * if 1 the thread waits until there are no more jobs in the queue
     */
    int exiting;
};

/* The main pool structure
 *
 * To find member descriptions, see mythreads.c .
 */
typedef struct threadPool ThreadPool;


// LOCK_RETURN
#define LOCK_RETURN(l, r)                    \
  if (pthread_mutex_lock(l) != 0)            \
  {                                          \
    fprintf(stderr, "ERRORE FATALE lock\n"); \
    return r;                                \
  }

// UNLOCK_RETURN
#define UNLOCK_RETURN(l, r)                    \
  if (pthread_mutex_unlock(l) != 0)            \
  {                                            \
    fprintf(stderr, "ERRORE FATALE unlock\n"); \
    return r;                                  \
  }

// SIGNAL_RETURN
#define SIGNAL_RETURN(c, r)                    \
  if (pthread_cond_signal(c) != 0)            \
  {                                            \
    fprintf(stderr, "ERRORE FATALE signal\n"); \
    return r;                                  \
  }

#define COND_WAIT_RETURN(c, l,  r)                    \
  if (pthread_cond_wait(c, l) != 0)            \
  {                                            \
    fprintf(stderr, "ERRORE FATALE cond wait\n"); \
    return r;                                  \
  }

#define BROADCAST_RETURN(l,  r)                    \
  if (pthread_cond_broadcast(l) != 0)            \
  {                                            \
    fprintf(stderr, "ERRORE FATALE  broadcast\n"); \
    return r;                                  \
  }
/**
 * @function createThreadPool
 * @brief Creates a thread pool for an object.
 * @param numThreads is the number of threads in the pool
 * @param jobSize is the size of the job list
 *
 * @return a new thread pool or NULL
 */
ThreadPool * createPool(int numThreads, int jobSize);


/**
 * @function destroyThreadPool
 * @brief stops all threads and destroys the pool object
 * @param pool  object to be released
 * @param force if 1 it forces all threads to exit immediately and frees resources immediately,
 *              if 0 it waits for the threads to finish all and only the pending jobs (does not accept other jobs).
 *
 * @return number of threads still active before the threadpool was destroyed
 */
int destroyPool(ThreadPool *pool, int force);


/**
 * @function addJobToThreadPool
 * @brief adds a job to the job queue if the queue is full waits until a job is freed from the queue
 * @param pool thread pool object
 * @param func  function to perform to complete the job
 * @param arg1  first argument of function
 * @param arg2  second argument of functions
 * @return 0 success, -1 if there have been function failures or unavailable memory
 */
int addJobToPool(ThreadPool *pool, void (*func)(void *arg1, void *arg2), void *arg1, void *arg2);


/**
 * @function addThreadsToPool
 * @brief this function adds specified number of new threads to the threadpool
 * @param pool thread pool object
 * @param threads number of threads to add to the threadpool
 * @return 0 if all threads have been added, -1 otherwise
 */
int addThreadsToPool(ThreadPool *pool, int threads);

/**
 * @function removeThreadFromPool
 * @brief remove an existing thread from the pool
 * @param pool thread pool object
 * @param num number of threads to remove to the threadpool
 * @return 0 success, -1 failed
 */
int removeThreadFromPool(ThreadPool *pool, int num);
#endif //FARM2_MYTHREADPOOL_H

