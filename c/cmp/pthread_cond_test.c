/*
    This program tests POSIX conditional variables.

    Author: Torur Biskopsto Strom
    Copyright: DTU, BSD License
*/

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t startcond = PTHREAD_COND_INITIALIZER;
int id;

void * work(void * arg) {
  pthread_mutex_lock(&mutex);
  pthread_cond_signal(&startcond);
  pthread_cond_wait(&cond, &mutex);
  printf("Core %d notified\n",get_cpuid());
  pthread_mutex_unlock(&mutex);
  return NULL;
}

int main() {
  int cpucnt = get_cpucnt();

  printf("Started using %d cores\n",cpucnt);

  pthread_t *threads = malloc(sizeof(pthread_t) * cpucnt);
  pthread_mutex_lock(&mutex);
  for(int i = 1; i < cpucnt; i++) {
    pthread_create(threads+i,NULL,work,NULL);
    pthread_cond_wait(&startcond, &mutex);
    printf("Thread %d ready waiting for broadcast\n",i);
  }
  pthread_mutex_unlock(&mutex);
  pthread_cond_broadcast(&cond);

  for(int i = 1; i < cpucnt; i++)
    pthread_join(*(threads+i),NULL);
}
