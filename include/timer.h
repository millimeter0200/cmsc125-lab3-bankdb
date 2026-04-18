#ifndef TIMER_H
#define TIMER_H

// starts the global timer thread
void start_timer();

// stops the timer (optional for cleanup)
void stop_timer();

// returns current global tick
int get_global_tick();

#endif