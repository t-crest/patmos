#ifndef BARRIER_COUNTER_H
#define BARRIER_COUNTER_H

#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/rtc.h>
#include <pthread.h>

typedef enum {LOCKED, UNLOCKED} barrier_state_t;

typedef struct {
    unsigned count;
    pthread_mutex_t *lock;
    barrier_state_t state;
    unsigned limit;
} barrier_counter_t;

// extern pthread_mutex_t common_l;

void barrier_counter_init(barrier_counter_t *barrier, pthread_mutex_t *lock, unsigned limit);
void barrier_counter_wait(barrier_counter_t *barrier);

#endif