
#include <pthread.h>
#include "pthread_barrier.h"

#define PTHREAD_BARRIER_SERIAL_THREAD 1


int pthread_barrier_init(pthread_barrier_t *barrier, void *attr, int count) {
    barrier->threads_required = count;
    barrier->threads_left = count;
    barrier->cycle = 0;
    pthread_mutex_init(&barrier->mutex, NULL);
    pthread_cond_init(&barrier->condition_variable, NULL);
    return 0;
}


int pthread_barrier_wait(pthread_barrier_t *barrier) {
    pthread_mutex_lock(&barrier->mutex);

    if (--barrier->threads_left == 0) {
        barrier->cycle++;
        barrier->threads_left = barrier->threads_required;

        pthread_cond_broadcast(&barrier->condition_variable);
        pthread_mutex_unlock(&barrier->mutex);

        return PTHREAD_BARRIER_SERIAL_THREAD;
    } else {
        unsigned int cycle = barrier->cycle;

        while (cycle == barrier->cycle)
            pthread_cond_wait(&barrier->condition_variable, &barrier->mutex);

        pthread_mutex_unlock(&barrier->mutex);
        return 0;
    }
}