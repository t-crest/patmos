#pragma once
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>	
#include <stdbool.h>

#define SET_TO_MS 1000
#define SEC_TO_US 1000000
#define SEC_TO_NS 1000000000.0
#define MS_TO_US 1000
#define MS_TO_NS 1000000
#define US_TO_NS 1000

#define NS_TO_SEC 1.0/SEC_TO_NS
#define NT_TO_MS 1.0/MS_TO_NS
#define NS_TO_US 1.0/US_TO_NS

#define CLKS_TO_US CPU_PERIOD * NS_TO_US 
#define US_TO_CLKS (US_TO_NS/CPU_PERIOD)

#define CPU_PERIOD 12.5
#define WCET_DISPATCHER 2046

#define schedtime_t uint64_t

typedef struct {
    schedtime_t period;
    schedtime_t *release_times;
    schedtime_t deadline;
    uint32_t wcet;
    uint32_t release_inst;
    uint32_t nr_releases;
	schedtime_t delta_sum;
    schedtime_t last_release_time;
    uint32_t exec_count;
    void (*func)(const void *self);
} MinimalTTTask;

typedef struct {
    schedtime_t hyper_period;
    uint32_t num_tasks;
    schedtime_t (*get_time)(void);
    schedtime_t start_time;
    MinimalTTTask *tasks;
} MinimalTTSchedule;

void init_minimal_tttask(MinimalTTTask *newTask, const schedtime_t period, schedtime_t *release_times, const schedtime_t nr_releases, void (*func)(const void *self));
MinimalTTSchedule init_minimal_ttschedule(const schedtime_t hyperperiod, const uint32_t num_tasks, MinimalTTTask *tasks, schedtime_t (*get_time)(void));
uint32_t tt_minimal_schedule_loop(MinimalTTSchedule *schedule, const uint32_t noLoops, const bool infinite);
uint8_t tt_minimal_dispatcher(MinimalTTSchedule *schedule, const schedtime_t schedule_ime);
void sort_asc_minimal_tttasks(MinimalTTTask *tasks, const uint32_t num_tasks);
