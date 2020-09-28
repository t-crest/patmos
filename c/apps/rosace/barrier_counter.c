#include <stdio.h>
#include "barrier_counter.h"

void barrier_counter_init(barrier_counter_t *barrier, unsigned required, const char* id)
{
  for(int i=0; i<8; i++)
    barrier->id[i] = id[i];
  barrier->arrived = 0;
  barrier->required = required;
  barrier->state = LOCKED;
  barrier->cycle = 0;
  pthread_mutex_init(&barrier->mutex, NULL);
  pthread_cond_init(&barrier->condition_var, NULL);
}


int barrier_counter_wait(barrier_counter_t *barrier) 
{
  volatile unsigned char spin = 0;

  pthread_mutex_lock(&barrier->mutex);
  if(barrier->state == LOCKED)
  {
    barrier->arrived++;
    #ifdef DEBUG_PRINTS
    pthread_mutex_lock(&common_l);
    printf("#%u--%s--LCKD\n", get_cpuid(), barrier->id);
    pthread_mutex_unlock(&common_l);
    #endif
  }
  spin = barrier->state;
  pthread_mutex_unlock(&barrier->mutex);  
 
  while(spin)
  {   
    pthread_mutex_lock(&barrier->mutex);
    if (barrier->state == UNLOCKED || barrier->arrived >= barrier->required)
    {
      barrier->state = UNLOCKED;
    }
    spin = barrier->state;
    pthread_mutex_unlock(&barrier->mutex);
  }
  
  pthread_mutex_lock(&barrier->mutex);
  barrier->arrived--;
  if(barrier->arrived <= 0)
  {
    barrier->arrived = 0;
    barrier->state = LOCKED;
  }
  #ifdef DEBUG_PRINTS
  pthread_mutex_lock(&common_l);
  printf("#%u--%s-->FREE\n", get_cpuid(), barrier->id);
  pthread_mutex_unlock(&common_l);
  #endif
  pthread_mutex_unlock(&barrier->mutex);
  return 0;
}