#include "timer.h"

// global tick counter
static int global_tick = 0;

void start_timer()
{
  // TODO: create a thread that increments global_tick periodically
}

void stop_timer()
{
  // TODO: stop timer thread safely
}

int get_global_tick()
{
  return global_tick;
}