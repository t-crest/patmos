#include <stdio.h>
#include "barrier_counter.h"

static pthread_mutex_t common_l;

void barrier_counter_init(barrier_counter_t *barrier,  pthread_mutex_t *lock, unsigned limit)
{
    barrier->count = 0;
    barrier->lock = lock;
    barrier->limit = limit;
    barrier->state = LOCKED;
}


void barrier_counter_wait(barrier_counter_t *barrier) 
{
  pthread_mutex_lock(barrier->lock);
  if(barrier->state == LOCKED)
    barrier->count++;
  pthread_mutex_unlock(barrier->lock);


  // pthread_mutex_lock(&common_l);
  // printf("tid %u wait for %u === %u \n", get_cpuid(), barrier->count, barrier->limit);
  // pthread_mutex_unlock(&common_l);

  while(1)
  {   
    pthread_mutex_lock(barrier->lock);
    if (LOCKED){
      if (barrier->count >= barrier->limit){
        barrier->state = UNLOCKED;
      }
    } else {
      // printf("tid %u free \n", get_cpuid());
      barrier->count--;
      break;
    }
  }
  return;
}