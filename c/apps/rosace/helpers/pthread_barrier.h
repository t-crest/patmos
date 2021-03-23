#ifndef PTHREAD_BARRIER_H
#define PTHREAD_BARRIER_H

#include <pthread.h>

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t condition_variable;
    int threads_required;
    int threads_left;
    unsigned int cycle;
} pthread_barrier_t;

int pthread_barrier_init(pthread_barrier_t *barrier, void *attr, int count);
int pthread_barrier_wait(pthread_barrier_t *barrier);

#endif //PTHREAD_BARRIER_H