#ifndef TIMER_H
#define TIMER_H
#include <pthread.h>
#include <stdio.h>

// start timer thread
void start_timer(int tick_ms);

// get current tick
int get_global_tick();

// wait until a specific tick
void wait_until_tick(int target_tick);

void stop_timer();

#endif