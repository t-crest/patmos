#pragma once
#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/rtc.h>
#include <pthread.h>
#include "assemblage_includes.h"
#include "../helpers/pthread_barrier.h"
#include "../helpers/printf.h"

// Barriers
extern pthread_barrier_t cycle_start_b;
extern pthread_barrier_t engine_elevator_b;
extern pthread_barrier_t filter_b;
extern pthread_barrier_t control_b;
extern pthread_barrier_t output_b;

extern uint64_t step_simu;

// Threads ...
void* thread1(void* arg);
void* thread2(void* arg);
void* thread3(void* arg);
void* thread4(void* arg);
void* thread5(void* arg);

void rosace_init();

int run_rosace();