#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

#include <semaphore.h>
#include <pthread.h>

typedef struct
{
    int capacity;

    sem_t empty_slots;
    sem_t full_slots;

    pthread_mutex_t lock;

    // usage tracking
    int current_usage;
    int peak_usage;

    // statistics
    int total_loads;
    int total_evictions;

} BufferPool;

void init_buffer_pool(BufferPool *bp, int capacity); // initialize buffer pool with given capacity
void load_account(BufferPool *bp);                   // simulate loading an account into buffer
void unload_account(BufferPool *bp);                 // simulate releasing an account from buffer
void destroy_buffer_pool(BufferPool *bp);            // clean up buffer pool resources

#endif