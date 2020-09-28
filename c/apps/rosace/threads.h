#ifndef __DEF_ROSACE_THREADS_H
#define __DEF_ROSACE_THREADS_H
#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/rtc.h>
#include <pthread.h>
#include "assemblage_includes.h"
#include "barrier_counter.h"
#include "pthread_barrier.h"

// Barriers
extern pthread_barrier_t cycle_start_b;
extern pthread_barrier_t engine_elevator_b;
extern pthread_barrier_t filter_b;
extern pthread_barrier_t control_b;
extern pthread_barrier_t output_b;

// extern barrier_counter_t cycle_start_b;
// extern barrier_counter_t engine_elevator_b;
// extern barrier_counter_t filter_b;
// extern barrier_counter_t control_b;
// extern barrier_counter_t output_b;

extern uint64_t step_simu;

// Threads ...
void* thread1(void* arg);
void* thread2(void* arg);
void* thread3(void* arg);
void* thread4(void* arg);
void* thread5(void* arg);

void rosace_init();

int run_rosace();

#endif

