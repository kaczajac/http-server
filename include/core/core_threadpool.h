#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <core_allocator.h>
#include <core_types.h> 

#include <pthread.h>        // thread management

typedef void* (*JobFunction)(void*);

struct Job {
    JobFunction function;
    void        *arg;
};

struct WorkerArgs {
    i64             receiver;
    pthread_mutex_t *job_mutex;
};

struct ThreadPool {
    i64             sender;
    i64             receiver;
    pthread_mutex_t job_mutex;
    u64             worker_count;
    pthread_t       *workers;
};

struct ThreadPool *threadpool_create(struct MemoryBlock *block, u64 size);
void threadpool_execute(struct ThreadPool *pool, JobFunction function, void *arg);
void threadpool_destroy(struct ThreadPool *pool);
void *threadpool_worker_routine(void *args);

#endif /* __THREADPOOL_H__ */
