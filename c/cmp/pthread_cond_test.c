/*
    This program tests POSIX conditional variables.

    Author: Torur Biskopsto Strom
    Copyright: DTU, BSD License
*/

#include "libcorethread/corethread.h"
#include <stdio.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int id;

void work() {
  pthread_mutex_lock(&mutex);
  id = get_cpuid();
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);
}

int main() {

  int cpucnt = get_cpucnt();
  
  printf("Started using %d threads\n",cpucnt);
  
  for(int i = 1; i < cpucnt; i++)
  {
    pthread_mutex_lock(&mutex);
    corethread_create(i,&work,NULL);
    pthread_cond_wait(&cond, &mutex);
    printf("Signal received from cpu=%d\n",id);
    pthread_mutex_unlock(&mutex);
    void * res;
    corethread_join(i, &res);
  }
}
