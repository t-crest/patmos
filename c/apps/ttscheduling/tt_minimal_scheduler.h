#pragma once
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>	
#include <stdbool.h>

#define schedtime_t uint64_t

typedef struct {
    uint16_t id;
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

typedef struct rm_task_node
{
    MinimalTTTask task;
    struct rm_task_node *next;
} MinimalTTTaskNode;

typedef struct {
    schedtime_t hyper_period;
    schedtime_t (*get_time)(void);
    schedtime_t start_time;
    uint32_t task_count;
    MinimalTTTaskNode *head, *tail;
} MinimalTTSchedule;

void init_minimal_tttask(MinimalTTTask *newTask, const uint16_t id, const schedtime_t period, schedtime_t *release_times, const schedtime_t nr_releases, void (*func)(const void *self));
MinimalTTSchedule init_minimal_ttschedule(const schedtime_t hyperperiod, const uint32_t num_tasks, MinimalTTTask *tasks, schedtime_t (*get_time)(void));
uint32_t tt_minimal_schedule_loop(MinimalTTSchedule *schedule, const uint32_t noLoops, const bool infinite);
uint8_t tt_minimal_dispatcher(MinimalTTSchedule *schedule, const schedtime_t schedule_ime);
MinimalTTTaskNode* create_tttasknode(MinimalTTTask task);
void ttschedule_enqueue(MinimalTTSchedule* schedule, MinimalTTTask task);
void ttschedule_sortedinsert_period(MinimalTTSchedule *schedule, MinimalTTTaskNode* new_node);
void ttschedule_sortedinsert_release(MinimalTTSchedule *schedule, MinimalTTTaskNode* new_node);
MinimalTTTaskNode* ttschedule_dequeue(MinimalTTSchedule* schedule);
void print_ttschedule(MinimalTTTaskNode* n);
