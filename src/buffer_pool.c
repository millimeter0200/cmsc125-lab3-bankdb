#include "buffer_pool.h"

// initialize buffer pool
void init_buffer_pool(BufferPool *bp, int capacity)
{
    bp->capacity = capacity;

    sem_init(&bp->empty_slots, 0, capacity);
    sem_init(&bp->full_slots, 0, 0);

    pthread_mutex_init(&bp->lock, NULL);

    // initialize tracking values
    bp->current_usage = 0;
    bp->peak_usage = 0;

    bp->total_loads = 0;
    bp->total_evictions = 0;
}

// simulate loading an account into buffer
void load_account(BufferPool *bp)
{
    sem_wait(&bp->empty_slots);

    pthread_mutex_lock(&bp->lock);

    bp->current_usage++;
    bp->total_loads++;

    if (bp->current_usage > bp->peak_usage)
    {
        bp->peak_usage = bp->current_usage;
    }

    pthread_mutex_unlock(&bp->lock);

    sem_post(&bp->full_slots);
}

// simulate releasing an account
void unload_account(BufferPool *bp)
{
    sem_wait(&bp->full_slots);

    pthread_mutex_lock(&bp->lock);

    if (bp->current_usage > 0)
    {
        bp->current_usage--;
    }

    bp->total_evictions++;

    pthread_mutex_unlock(&bp->lock);

    sem_post(&bp->empty_slots);
}