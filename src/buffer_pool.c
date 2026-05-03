#include "buffer_pool.h"
#include <stdio.h>

void init_buffer_pool(BufferPool *bp, int capacity)
{
    bp->capacity = capacity;

    sem_init(&bp->empty_slots, 0, capacity);
    sem_init(&bp->full_slots, 0, 0);

    bp->total_loads = 0;
    bp->total_unloads = 0;
    bp->peak_usage = 0;
    bp->current_usage = 0;
    bp->blocked_count = 0;
}

void load_account(BufferPool *bp)
{
    if (sem_trywait(&bp->empty_slots) != 0)
    {
        bp->blocked_count++;
        sem_wait(&bp->empty_slots);
    }

    bp->total_loads++;
    bp->current_usage++;

    if (bp->current_usage > bp->peak_usage)
        bp->peak_usage = bp->current_usage;
}

void unload_account(BufferPool *bp)
{
    bp->total_unloads++;
    bp->current_usage--;

    sem_post(&bp->empty_slots);
}

void print_buffer_pool_report(BufferPool *bp)
{
    printf("\n=== Buffer Pool Report ===\n");
    printf("Total Loads: %d\n", bp->total_loads);
    printf("Total Unloads: %d\n", bp->total_unloads);
    printf("Peak Usage: %d\n", bp->peak_usage);
    printf("Blocked Operations: %d\n", bp->blocked_count);
}


