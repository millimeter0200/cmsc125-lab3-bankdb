#ifndef TIMER_H
#define TIMER_H
#include <pthread.h>

// start timer thread
void start_timer();

// get current tick
int get_global_tick();

// wait until a specific tick
void wait_until_tick(int target_tick);

#endif