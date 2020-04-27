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

#define NS_TO_SEC 1/SEC_TO_NS
#define NT_TO_MS 1/MS_TO_NS
#define NS_TO_US 1/US_TO_NS

#define CPU_PERIOD 12.5
#define WCET_DISPATCHER 17323
#define WCET_DISPATCHER_US (uint64_t) ((WCET_DISPATCHER * CPU_PERIOD) * NS_TO_US)

typedef struct {
    uint64_t period;
    uint64_t *release_times;
    uint32_t release_inst;
    uint32_t nr_releases;
	uint64_t delta_sum;
    uint64_t last_release_time;
    uint32_t exec_count;
    void (*func)(const void *self);
} MinimalTTTask;

typedef struct {
    double hyper_period;
    uint16_t num_tasks;
    uint64_t (*get_time)(void);
    uint64_t start_time;
    MinimalTTTask *tasks;
} MinimalTTSchedule;

void init_minimal_tttask(MinimalTTTask *newTask, const uint64_t period, uint64_t *release_times, const uint64_t nr_releases, void (*func)(const void *self));
MinimalTTSchedule init_minimal_ttschedule(const uint64_t hyperperiod, const uint16_t num_tasks, MinimalTTTask *tasks, uint64_t (*get_time)(void));
uint32_t tt_minimal_schedule_loop(MinimalTTSchedule *schedule, const uint16_t noLoops, const bool infinite);
uint8_t tt_minimal_dispatcher(MinimalTTSchedule *schedule, const uint64_t schedule_ime);
void sort_asc_minimal_tttasks(MinimalTTTask *tasks, const uint16_t num_tasks);
