#include <core_threadpool.h>

#include <unistd.h>

struct ThreadPool *threadpool_create(struct MemoryBlock *block, u64 worker_count) {

    if (block == NULL || worker_count == 0) {
        return NULL;
    }

    u64 pool_memory     = sizeof(struct ThreadPool);
    u64 workers_memory  = (sizeof(pthread_t) + sizeof(struct WorkerArgs)) * worker_count;
    u64 needed_memory   = pool_memory + workers_memory;

    if (block->size + needed_memory > MEMPOOL_BLKCAP) {
        return NULL;
    }

    struct ThreadPool *pool = memblock_alloc(block, sizeof(struct ThreadPool));
    pool->worker_count      = worker_count;
    pthread_mutex_init(&(pool->job_mutex), 0);

    i32 channel[2];
    pipe(channel);
    pool->receiver  = channel[0];
    pool->sender    = channel[1];

    pool->workers = memblock_alloc(block, sizeof(pthread_t) * pool->worker_count);
    for (u64 worker_id = 0; worker_id < pool->worker_count; worker_id++) {

        struct WorkerArgs *args = memblock_alloc(block, sizeof(struct WorkerArgs));
        args->receiver          = channel[0];
        args->job_mutex         = &(pool->job_mutex);

        pthread_create(
            &(pool->workers[worker_id]), 
            NULL, 
            threadpool_worker_routine, 
            (void *) args
        );

    }

    return pool;

}

void threadpool_execute(struct ThreadPool *pool, JobFunction function, void *arg) {

    if (pool == NULL) {
        return;
    }

    struct Job job; 
    job.function    = function;
    job.arg         = arg;

    write(pool->sender, (void *) &job, sizeof(struct Job));

}

void threadpool_destroy(struct ThreadPool *pool) {

    close(pool->sender);
    for (u64 worker_id = 0; worker_id < pool->worker_count; worker_id++) {
        pthread_join(pool->workers[worker_id], NULL);
    }
    close(pool->receiver);

    pthread_mutex_destroy(&(pool->job_mutex));

}

void *threadpool_worker_routine(void *args) {

    struct WorkerArgs *worker_args = args;
    struct Job job = {};

    for (;;) {

        pthread_mutex_lock(worker_args->job_mutex);
        i64 bytes = read(worker_args->receiver, (void *) &job, sizeof(struct Job));
        pthread_mutex_unlock(worker_args->job_mutex);
    
        if (bytes > 0) {
            job.function(job.arg);
        } else if (bytes == 0) {
            break;
        }

    }

    return NULL;

}
