/*
    This program tests POSIX mutexes.

    Author: Torur Biskopsto Strom
    Copyright: DTU, BSD License
*/

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <machine/patmos.h>

// Uncomment the line below to see the test fail
// when not using a mutex and cpucnt > 1
// #define WITHOUT_MUTEX 1
#define ITERATIONS 1000

#ifdef WITHOUT_MUTEX
_UNCACHED
#endif
int cnt = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void * work(void * arg) {
  int id = get_cpuid();
  pthread_mutex_lock(&mutex);
  pthread_mutex_unlock(&mutex);
  for(int i = 0; i < ITERATIONS; i++) {
    asm volatile ("" : : : "memory");
#ifndef WITHOUT_MUTEX
    pthread_mutex_lock(&mutex);
#endif
    cnt++;
#ifndef WITHOUT_MUTEX
    pthread_mutex_unlock(&mutex);
#endif
  }
  return NULL;
}

int main() {

  int cpucnt = get_cpucnt();
  const int exp = cpucnt*ITERATIONS;
  
  printf("Started using %d threads\n",cpucnt);
  
  pthread_t *threads = malloc(sizeof(pthread_t) * cpucnt);
  
  // No thread starts before all are initialized;
  pthread_mutex_lock(&mutex);
  for(int i = 1; i < cpucnt; i++)
  {
    int retval = pthread_create(threads+i, NULL, work, NULL);
    if(retval != 0)
    {
      printf("Unable to start thread %d, error code %d\n", i, retval);
      return retval;
    }
  }
  pthread_mutex_unlock(&mutex);
  work(NULL);
  
  for(int i = 1; i < cpucnt; i++) {
    void * dummy;
    int retval = pthread_join(*(threads+i), &dummy);
    if(retval != 0)
    {
      printf("Unable to join thread %d, error code %d\n", i, retval);
      return retval;
    }
  }
  
  free(threads);
#ifndef WITHOUT_MUTEX
  // Locking to update the global state (get the newest value of cnt)
  pthread_mutex_lock(&mutex);
#endif
  printf("Expected count=%d, actual count=%d\n", exp, cnt);
#ifndef WITHOUT_MUTEX
  pthread_mutex_unlock(&mutex);
#endif
}
