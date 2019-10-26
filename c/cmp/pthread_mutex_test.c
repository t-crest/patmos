/*
    This program tests POSIX mutexes.

    Author: Torur Biskopsto Strom
    Copyright: DTU, BSD License
*/

#include "libcorethread/corethread.h"
#include <stdio.h>

// Uncomment the line below to see the test fail
// when not using a mutex and cpucnt > 1
//#define WITHOUT_MUTEX

volatile int cnt = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void work() {
  for(int i = 0; i < 1000; i++) {
#ifndef WITHOUT_MUTEX
    pthread_mutex_lock(&mutex);
#endif
    cnt++;
#ifndef WITHOUT_MUTEX
    pthread_mutex_unlock(&mutex);
#endif
  }
}

int main() {

  int cpucnt = get_cpucnt();
  
  printf("Started using %d threads\n",cpucnt);
  
  // No thread starts before all are initialized;
  pthread_mutex_lock(&mutex);
  
  for(int i = 1; i < cpucnt; i++)
    corethread_create(i,&work,NULL);
  
  pthread_mutex_unlock(&mutex);
  
  work();
  
  for(int i = 1; i < cpucnt; i++) {
    void * res;
    corethread_join(i, &res);
  }
  
  // Locking to update the global state (get the newest value of cnt)
  pthread_mutex_lock(&mutex);
  printf("Expected count=%d, actual count=%d\n",cpucnt*1000,cnt);
  pthread_mutex_unlock(&mutex);
}
