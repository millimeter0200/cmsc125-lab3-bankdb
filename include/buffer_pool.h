#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

#include <semaphore.h>

typedef struct {
    int capacity;
    int total_loads;
    int total_unloads;
    int peak_usage;
    int current_usage;
    int blocked_count;
    sem_t empty_slots;
} BufferPool;

void init_buffer_pool(BufferPool *bp, int capacity);
void load_account(BufferPool *bp);
void unload_account(BufferPool *bp);
void print_buffer_pool_report(BufferPool *bp);

#endif
