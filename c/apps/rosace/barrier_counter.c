#include "barrier_counter.h"


void barrier_counter_init(barrier_counter_t *barrier,  pthread_mutex_t *lock, unsigned limit)
{
    barrier->count = 0;
    barrier->lock = lock;
    barrier->limit = limit;
}


void barrier_counter_wait(barrier_counter_t *barrier) 
{
  pthread_mutex_lock(barrier->lock);
  barrier->count++;
  pthread_mutex_unlock(barrier->lock);

  while(1)
  {   
    pthread_mutex_lock(barrier->lock);
    if(barrier->count >= barrier->limit){
        pthread_mutex_unlock(barrier->lock);
        break;
    }
  }
  return;
}