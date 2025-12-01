//
//Created by Natalia Ceccarini
//

#include <util.h>
#include <myThreadPool.h>

/**
 * @function thread_manager
 * @brief function that runs the thread manager to handle incrementing and removing threads dynamically
 * @param pl thread pool object
 * @return NULL
 */
static void* thread_manager(void *pl) {
    ThreadPool *pool = (ThreadPool *)pl;
    
    LOCK_RETURN(&(pool->lockSignal), NULL);
    
    while (1){

        while(pool->ctrAddThreads == 0 && pool->ctrRemoveThreads == 0 && (!pool->exiting))
        {
            COND_WAIT_RETURN(&pool->cond_counter, &pool->lockSignal, NULL);

        }


        if (pool->exiting>0)
        {
            break;
        }

        //check the thread increment counter is greater than 0 because a sigusr1 has arrived
        if (pool->ctrAddThreads > 0)
        {
            int n = pool->ctrAddThreads;
            pool->ctrAddThreads = 0;
            UNLOCK_RETURN(&(pool->lockSignal), NULL);
            addThreadsToPool(pool, n);

        }

        //check the thread decrement counter is greater than 0 because a sigusr2 has arrived
        //check the number of live threads is greater than 1
        if (pool->ctrRemoveThreads > 0 && pool->numthreadsalive > 1)
        {

            int n;

            //check the number of alive threads minus the number of threads to delete is < 1
            // because the number of threads alive must not be 0
            if(pool->numthreadsalive - pool->ctrRemoveThreads < 1)
            {
                n = pool->numthreadsalive - 1;
            }
            else
            {
                n = pool->ctrRemoveThreads;
            }

            pool->ctrRemoveThreads = 0;
            UNLOCK_RETURN(&(pool->lockSignal), NULL);
            removeThreadFromPool(pool, n);

        }
        else
        {
            pool->ctrRemoveThreads = 0;
            UNLOCK_RETURN(&(pool->lockSignal), NULL);
        }

        LOCK_RETURN(&(pool->lockSignal), NULL);
    }
    UNLOCK_RETURN(&(pool->lockSignal), NULL);
#ifdef DEBUG
    //printf("Thread manager exiting\n");
#endif
    return NULL;
}

ThreadPool * createPool(int numThreads, int jobSize){
    if(numThreads <= 0 || jobSize < 0)
    {
    
        printf("[THREADPOOL:INIT:ERROR] invalid agument!\n");

        errno = EINVAL;
        return NULL;
    }
    ThreadPool *pool = (ThreadPool *) calloc(1, sizeof(ThreadPool));

    if(pool==NULL){ // Oops!

        printf("[THREADPOOL:INIT:ERROR] Unable to allocate memory for the pool!\n");

        return NULL;
    }

    #ifdef DEBUG
        printf("[THREADPOOL:INIT:INFO] Allocated %lu bytes for new pool!\n", sizeof(ThreadPool));
    #endif
    // Initialize members with default values
    pool->exiting = 0;
    pool->ctrAddThreads = 0;
    pool->ctrRemoveThreads = 0;
    pool->countJob = 0;
    pool->numThreads = 0;
    pool->numthreadsalive = 0;
    pool->queueSize = jobSize;
    pool->removeThreads = 0;
    pool->FRONT = NULL;
    pool->REAR = NULL;
    pool->threads = NULL;
    pool->rearThreads = NULL;

    #ifdef DEBUG
        printf("[THREADPOOL:INIT:INFO] Initializing mutexes!\n");
    #endif

    pthread_mutex_init(&pool->lock, NULL); // Initialize mutex
    pthread_mutex_init(&pool->lockSignal, NULL); // Initialize mutex



    #ifdef DEBUG
        printf("[THREADPOOL:INIT:INFO] Initiliazing conditionals!\n");
    #endif

    pthread_cond_init(&pool->cond_consumer, NULL); // Initialize idle conditional
    pthread_cond_init(&pool->cond_productor, NULL); // Initialize idle conditional
    pthread_cond_init(&pool->cond_counter, NULL); // Initialize idle conditional




#ifdef DEBUG
    printf("THREADPOOL:INIT:INFO] Successfully initialized all members of the pool!\n");
        printf("[THREADPOOL:INIT:INFO] Initializing thread manager\n");
#endif

    if(pthread_create(&(pool->tid_manager), NULL, thread_manager, (void*)pool) != 0)
    {
        #ifdef DEBUG
            printf("[THREADPOOL:INIT:INFO] thread manager not initialized successfully!\n");
        #endif
        pthread_mutex_destroy(&(pool->lock));
        pthread_mutex_destroy(&(pool->lockSignal));
        pthread_cond_destroy(&(pool->cond_productor));
        pthread_cond_destroy(&(pool->cond_consumer));
        pthread_cond_destroy(&(pool->cond_counter));
        free(pool);
        pool = NULL;
        return NULL;
    }

#ifdef DEBUG
        printf("[THREADPOOL:INIT:INFO] Initializing %u threads..\n",numThreads);
#endif
    int res = 0;
    res = addThreadsToPool(pool, numThreads); // Add threads to the pool

    if(res == 0)
    {

        #ifdef DEBUG
            printf("[THREADPOOL:INIT:INFO] New threadpool initialized successfully!\n");
        #endif
        return pool;


    }
    else
    {

        printf("[THREADPOOL:INIT:INFO] New threadpool not initialized successfully!\n");

        pthread_mutex_destroy(&(pool->lock));
        pthread_mutex_destroy(&(pool->lockSignal));
        pthread_cond_destroy(&(pool->cond_productor));
        pthread_cond_destroy(&(pool->cond_consumer));
        pthread_cond_destroy(&(pool->cond_counter));
        free(pool);
        pool = NULL;
        return NULL;

    }

}

/**
 * @functions threadExecutor
 * @brief execute by worker threads, which remove a job from the list and execute it
 * @param pl threadpool object
 * @return NULL
 */
static void *threadExecutor(void *pl){
    ThreadPool *pool = (ThreadPool *)pl; // Get the pool
    LOCK_RETURN(&pool->lock, NULL); // Lock the mutex
 #ifdef DEBUG
    int id = ++pool->threadID; // Get an id
#endif


    #ifdef DEBUG
        printf("[THREADPOOL:THREAD%u:INFO] Starting execution loop!\n", id);
    #endif
    //Start the core execution loop
    while(1){

        while(pool->FRONT == NULL && pool->removeThreads < 1 && (!pool->exiting))
        {
            COND_WAIT_RETURN(&pool->cond_consumer, &pool->lock, NULL);

        }
        if(pool->removeThreads > 0) // A thread is needed to be removed
        {
            #ifdef DEBUG
                printf("[THREADPOOL:THREAD%d:INFO] Removal signalled! Exiting the thread!\n", id);
            #endif
            pool->removeThreads--;
            
            break;
        }

        if(pool->exiting > 1 || (pool->exiting == 1 && !pool->countJob)){
            #ifdef DEBUG
                printf("[THREADPOOL:THREAD%u:INFO] Pool has been stopped! Exiting now..\n", id);
            #endif
            break; // Exit the loop
        }
        Job *presentJob = pool->FRONT; // Get the first job
        pool->FRONT = pool->FRONT->next; // Shift FRONT to right
        if(pool->FRONT==NULL){
            // No jobs next
            pool->REAR = NULL; // Reset the REAR
        }
        pool->countJob--;

        SIGNAL_RETURN(&pool->cond_productor, NULL);

        UNLOCK_RETURN(&pool->lock, NULL);

        #ifdef DEBUG
        printf("[THREADPOOL:THREAD%d:INFO] Executing the job now!\n", id);
        #endif
        char *arg1 = (char *)calloc(strlen(presentJob->arg1) + 1, sizeof(char));
        strncpy(arg1, presentJob->arg1, strlen(presentJob->arg1));
        presentJob->function(arg1, presentJob->arg2); // Execute the job
        #ifdef DEBUG
            printf("[THREADPOOL:THREAD%u:INFO] Job completed! Releasing memory for the job!\n", id);
        #endif
        free(arg1);
        free(presentJob->arg1);
        free(presentJob); // Release memory for the job

        LOCK_RETURN(&pool->lock, NULL);
    }

    pool->numthreadsalive--; //decrease the threads alive
    
            
    UNLOCK_RETURN(&pool->lock, NULL);



    return NULL; //Exit

}

int addThreadsToPool(ThreadPool *pool, int threads){
    if(pool==NULL){ // Sanity check
        printf("[THREADPOOL:ADD:ERROR] Pool is not initialized!\n");
        return -1;
    }

    if(threads<1){
        printf("[THREADPOOL:ADD:WARNING] Tried to add invalid number of threads %d!\n", threads);
        return -1;
    }

    int rc = 0;

    LOCK_RETURN(&pool->lock, -1);



    #ifdef DEBUG
        printf("[THREADPOOL:ADD:INFO] Speculative increment done!\n");
    #endif

    for(int i=0; i<threads;i++){

        ThreadList *newThread = (ThreadList *)malloc(sizeof(ThreadList)); // Allocate a new thread
        newThread->next = NULL;
        rc = pthread_create(&newThread->thread, NULL, threadExecutor, (void *)pool); // Start the thread
        if(rc){
            printf("[THREADPOOL:ADD:ERROR] Unable to create thread %d(error code %d)!\n", (i+1), rc);
            break;
        }
        else{
            pool->numThreads++;
            #ifdef DEBUG
                printf("[THREADPOOL:ADD:INFO] Initialized thread %d!\n", pool->numThreads);
            #endif
            pool->numthreadsalive++;
            if(pool->rearThreads==NULL) // This is the first thread
                pool->threads = pool->rearThreads = newThread;
            else // There are threads in the pool
                pool->rearThreads->next = newThread;
            pool->rearThreads = newThread; // This is definitely the last thread

        }
    }
    UNLOCK_RETURN(&pool->lock, -1);
    if (rc == 1) return -1;
    else return 0;

}

int removeThreadFromPool(ThreadPool *pool, int num){
    if(pool==NULL){
        printf("[THREADPOOL:REM:ERROR] Pool is not initialized!\n");
        return -1;
    }


    LOCK_RETURN(&pool->lock, -1)

    pool->removeThreads += num; // Indicate the willingness of removal
#ifdef DEBUG
    printf("[THREADPOOL:REM:INFO] Incremented the removal count: %d\n", pool->removeThreads);
#endif

    #ifdef DEBUG
        printf("[THREADPOOL:REM:INFO] Waking up any sleeping threads!\n");
    #endif

    SIGNAL_RETURN(&pool->cond_consumer, -1);
    UNLOCK_RETURN(&pool->lock, -1);

    #ifdef DEBUG
        printf("[THREADPOOL:REM:INFO] Signalling complete!\n");
    #endif
    return 0;
}

int addJobToPool(ThreadPool *pool, void (*func)(void *arg1, void *arg2), void *arg1, void *arg2){
    if(pool==NULL){ // Sanity check

        printf("[THREADPOOL:EXEC:ERROR] Pool is not initialized!\n");

        return -1;
    }
    if (func == NULL || arg1 == NULL)
    {

        printf("[THREADPOOL:EXEC:ERROR] Arguments null\n");

    }


    LOCK_RETURN(&pool->lock, -1);
    while((pool->countJob >= pool->queueSize) && (!pool->exiting))
    {
        COND_WAIT_RETURN(&pool->cond_productor, &pool->lock, -1);


    }

    if(pool->exiting)
    {
        UNLOCK_RETURN(&(pool->lock), -1);
        return -1;
    }

    Job *newJob = (Job *)malloc(sizeof(Job)); // Allocate memory
    if(newJob==NULL){

        printf("[THREADPOOL:EXEC:ERROR] Unable to allocate memory for new job!\n");

        return -1;
    }

    #ifdef DEBUG
        printf("\n[THREADPOOL:EXEC:INFO] Allocated %lu bytes for new job!", sizeof(Job));
    #endif

    newJob->function = func; // Initialize the function
    newJob->arg1 = (char *)calloc(strlen(arg1) + 1, sizeof(char));
    strncpy(newJob->arg1, arg1, strlen(arg1));
    newJob->arg2 = arg2; // Initialize the argument2
    newJob->next = NULL; // Reset the link

    #ifdef DEBUG
        printf("[THREADPOOL:EXEC:INFO] Locking the queue for insertion of the job!\n");
    #endif


    if(pool->FRONT==NULL) // This is the first job
        pool->FRONT = pool->REAR = newJob;
    else // There are other jobs
        pool->REAR->next = newJob;
    pool->REAR = newJob; // This is the last job
    pool->countJob++;
    #ifdef DEBUG
        printf("[THREADPOOL:EXEC:INFO] Inserted the job at the end of the queue!\n");
    #endif

    SIGNAL_RETURN(&pool->cond_consumer, -1)


    UNLOCK_RETURN(&(pool->lock), -1);


    #ifdef DEBUG
        printf("[THREADPOOL:EXEC:INFO] Unlocked the mutex!\n");
    #endif
    return 0;
}
int destroyPool(ThreadPool *pool, int force){
    if(pool==NULL){ // Sanity check

        printf("[THREADPOOL:EXIT:ERROR] Pool is not initialized!\n");

        return 0;
    }

    #ifdef DEBUG
        printf("[THREADPOOL:EXIT:INFO] Trying to wakeup all waiting threads..\n");
    #endif


    LOCK_RETURN(&pool->lockSignal, -1)
    int numThread = pool->numthreadsalive;
    pool->exiting = 1 + force;
    SIGNAL_RETURN(&pool->cond_counter, -1);
    UNLOCK_RETURN(&pool->lockSignal, -1);
    int rc = 0;
    rc = pthread_join(pool->tid_manager, NULL); //  Wait for thread manager to join


        if(rc)
        {
            printf("[THREADPOOL:EXIT:WARNING] Unable to join thread manager!\n");
        }
        else
        {
#ifdef DEBUG
            printf("[THREADPOOL:EXIT:INFO] Thread manager joined!\n");
#endif
        }


    LOCK_RETURN(&pool->lock, -1);
    SIGNAL_RETURN(&pool->cond_productor, -1);
    BROADCAST_RETURN(&pool->cond_consumer, -1);

    #ifdef DEBUG
        printf("[THREADPOOL:EXIT:INFO] Waiting for all threads to exit..\n");
    #endif


    UNLOCK_RETURN(&pool->lock, -1);

    ThreadList *currThread = NULL;
    int i = 0;
    while(pool->threads!=NULL){

        #ifdef DEBUG
            printf("[THREADPOOL:EXIT:INFO] Joining thread %d..\n", i+1);
        #endif

        rc = pthread_join(pool->threads->thread, NULL); //  Wait for ith thread to join
        if(rc)
        {

            printf("[THREADPOOL:EXIT:WARNING] Unable to join THREAD%d!\n", i+1);
        }
        else
        {
#ifdef DEBUG
            printf("[THREADPOOL:EXIT:INFO] THREAD%d joined!\n", i+1);
#endif
        }


        currThread = pool->threads;
        pool->threads = pool->threads->next; // Continue

        #ifdef DEBUG
            printf("[THREADPOOL:EXIT:INFO] Releasing memory for THREAD%u..\n", i+1);
        #endif

        free(currThread); // Free ith thread
        i++;

    }
    if(pool->rearThreads != NULL){

        fflush(stdout);
    }
    pool->rearThreads = NULL;
    #ifdef DEBUG
        printf("[THREADPOOL:EXIT:INFO] Destroying remaining jobs..\n");
    #endif
    Job *j = NULL;
    // Delete remaining jobs
    while(pool->FRONT!=NULL){
        j = pool->FRONT;
        pool->FRONT = pool->FRONT->next;
        free(j);
    }
    if(pool->REAR != NULL){

        fflush(stdout);
    }
    pool->REAR = NULL;
    #ifdef DEBUG
        printf("[THREADPOOL:EXIT:INFO] Destroying conditionals..\n");
    #endif
    pthread_cond_destroy(&pool->cond_consumer); // Destroying idle conditional
    pthread_cond_destroy(&pool->cond_productor); // Destroying end conditional
    pthread_cond_destroy(&pool->cond_counter); // Destroying end conditional


    pthread_mutex_destroy(&pool->lock); // Destroying queue lock
    pthread_mutex_destroy(&pool->lockSignal); // Destroying queue lock


    #ifdef DEBUG
        printf("[THREADPOOL:EXIT:INFO] Releasing memory for the pool..\n");
    #endif

    free(pool); // Release the pool
    pool = NULL;
    #ifdef DEBUG
        printf("[THREADPOOL:EXIT:INFO] Pool destruction completed!\n");
    #endif
    return numThread;
}

