#include <pthread.h>
#include <unistd.h>
#include "timer.h"

// shared tick
static volatile int global_tick = 0;

// synchronization
static pthread_mutex_t tick_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t tick_cond = PTHREAD_COND_INITIALIZER;

// timer thread function
static void *timer_thread_func(void *arg)
{
  (void)arg;

  while (1)
  {
    usleep(100000); // 100ms per tick

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
  pthread_t tid;
  pthread_create(&tid, NULL, timer_thread_func, NULL);
  pthread_detach(tid); // optional but cleaner (no need to join)
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

// ✅ ADD THIS (THIS WAS MISSING → CAUSED YOUR LINKER ERROR)
int get_global_tick()
{
  pthread_mutex_lock(&tick_lock);
  int tick = global_tick;
  pthread_mutex_unlock(&tick_lock);
  return tick;
}