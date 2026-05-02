#include <pthread.h>
#include <unistd.h>
#include "timer.h"

// shared tick
static int global_tick = 0;

// control flag
static int timer_running = 0;

// timer thread handle
static pthread_t timer_tid;

// synchronization
static pthread_mutex_t tick_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t tick_cond = PTHREAD_COND_INITIALIZER;

static int tick_duration = 100000; // default 100ms (microseconds)

// timer thread
static void *timer_thread_func(void *arg)
{
  (void)arg;

  while (1)
  {
    usleep(tick_duration);

    pthread_mutex_lock(&tick_lock);

    if (!timer_running)
    {
      pthread_mutex_unlock(&tick_lock);
      break;
    }

    global_tick++;
    pthread_cond_broadcast(&tick_cond);

    pthread_mutex_unlock(&tick_lock);
  }

  return NULL;
}

// start timer
void start_timer(int tick_ms)
{
  pthread_mutex_lock(&tick_lock);

  global_tick = 0; // reset each run
  timer_running = 1;
  tick_duration = tick_ms * 1000;

  pthread_mutex_unlock(&tick_lock);

  pthread_create(&timer_tid, NULL, timer_thread_func, NULL);
}

// stop timer
void stop_timer()
{
  pthread_mutex_lock(&tick_lock);
  timer_running = 0;
  pthread_cond_broadcast(&tick_cond); // wake sleepers
  pthread_mutex_unlock(&tick_lock);

  pthread_join(timer_tid, NULL);
}

// wait until tick
void wait_until_tick(int target_tick)
{
  pthread_mutex_lock(&tick_lock);

  while (global_tick < target_tick)
  {
    pthread_cond_wait(&tick_cond, &tick_lock);
  }

  pthread_mutex_unlock(&tick_lock);
}

// get current tick
int get_global_tick()
{
  pthread_mutex_lock(&tick_lock);
  int tick = global_tick;
  pthread_mutex_unlock(&tick_lock);
  return tick;
}