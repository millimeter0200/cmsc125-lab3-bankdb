#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

#include <semaphore.h>

typedef struct {
    int capacity;
    sem_t empty_slots;
    sem_t full_slots;
} BufferPool;

void init_buffer_pool(BufferPool *bp, int capacity);
void load_account(BufferPool *bp);
void unload_account(BufferPool *bp);

#endif
