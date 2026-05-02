#include <pthread.h>
#include <unistd.h>
#include "timer.h"

// shared tick
static volatile int global_tick = 0;

// control flag
static volatile int timer_running = 1;

// timer thread handle
static pthread_t timer_tid;

// synchronization
static pthread_mutex_t tick_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t tick_cond = PTHREAD_COND_INITIALIZER;

// timer thread function
static void *timer_thread_func(void *arg)
{
  (void)arg;

  while (timer_running)
  {
    usleep(10000); // 10ms per tick

    pthread_mutex_lock(&tick_lock);
    global_tick++;
    pthread_cond_broadcast(&tick_cond);
    pthread_mutex_unlock(&tick_lock);
  }

  return NULL;
}

// start timer
void start_timer()
{
  timer_running = 1; // reset in case reused
  pthread_create(&timer_tid, NULL, timer_thread_func, NULL);
}

// stop timer
void stop_timer()
{
  timer_running = 0;

  // wake any waiting threads so they don't block forever
  pthread_mutex_lock(&tick_lock);
  pthread_cond_broadcast(&tick_cond);
  pthread_mutex_unlock(&tick_lock);

  // wait for timer thread to finish
  pthread_join(timer_tid, NULL);
}

// wait for specific tick
void wait_until_tick(int target_tick)
{
  pthread_mutex_lock(&tick_lock);

  while (global_tick < target_tick)
  {
    pthread_cond_wait(&tick_cond, &tick_lock);
  }

  pthread_mutex_unlock(&tick_lock);
}

// get current tick safely
int get_global_tick()
{
  pthread_mutex_lock(&tick_lock);
  int tick = global_tick;
  pthread_mutex_unlock(&tick_lock);
  return tick;
}