#include "buffer_pool.h"

// initialize buffer pool
void init_buffer_pool(BufferPool *bp, int capacity)
{
    bp->capacity = capacity;
    sem_init(&bp->empty_slots, 0, capacity);
    sem_init(&bp->full_slots, 0, 0);
}

// simulate loading an account into buffer
void load_account(BufferPool *bp)
{
    sem_wait(&bp->empty_slots);
    sem_post(&bp->full_slots);
}

// simulate releasing an account
void unload_account(BufferPool *bp)
{
    sem_wait(&bp->full_slots);
    sem_post(&bp->empty_slots);
}
