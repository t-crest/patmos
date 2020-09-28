#ifndef BARRIER_COUNTER_H
#define BARRIER_COUNTER_H

#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/rtc.h>
#include <pthread.h>

typedef enum {UNLOCKED, LOCKED} barrier_state_t;

typedef struct {
    char id[8];
    int arrived;
    int required;
    pthread_mutex_t mutex;
    pthread_cond_t condition_var;
    int cycle;
    barrier_state_t state;
} barrier_counter_t;

extern pthread_mutex_t common_l;

void barrier_counter_init(barrier_counter_t *barrier,  unsigned required, const char* id);
int barrier_counter_wait(barrier_counter_t *barrier);

#endif