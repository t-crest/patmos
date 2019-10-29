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
#define WCET_DISPATCHER 1454
#define WCET_DISPATCHER_US (int) ((WCET_DISPATCHER * CPU_PERIOD) * NS_TO_US)

typedef struct {
    unsigned long long period;
    unsigned long long activation_time;
    unsigned long long last_start_time;
    long long (*func)(const void *self);
} MinimalTTTask;

typedef struct {
    double hyper_period;
    uint16_t num_tasks;
    MinimalTTTask *tasks;
    unsigned long long (*get_time)(void);
    unsigned long long start_time;
} MinimalTTSchedule;

void init_minimal_tttask(MinimalTTTask *newTask, const unsigned long long period, const unsigned long long activation_time, long long (*func)(const void *self));
MinimalTTSchedule init_minimal_ttschedule(const unsigned long long hyperperiod, const uint16_t num_tasks, MinimalTTTask *tasks, unsigned long long (*get_time)(void));
uint32_t tt_minimal_schedule_loop(MinimalTTSchedule *schedule, const uint16_t noLoops, const bool infinite);
uint8_t tt_minimal_dispatcher(MinimalTTSchedule *schedule, const unsigned long long schedule_ime);
void sort_asc_minimal_tttasks(MinimalTTTask *tasks, const uint16_t num_tasks);
