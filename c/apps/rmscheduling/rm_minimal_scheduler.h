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

typedef enum {
    ELECTED,
    BLOCKED,
    READY,
    PASSIVE,
    NONEXIST
} TaskState;

typedef struct rm_task {
    TaskState state;
    uint16_t id;
    schedtime_t period;
    schedtime_t deadline;
    uint32_t wcet;
    schedtime_t earliest_release;
    schedtime_t release_time;
	schedtime_t delta_sum;
    schedtime_t last_release_time;
    uint32_t exec_count;
    uint16_t overruns;
    void (*func)(const void *self);
} MinimalRMTask;

typedef struct rm_task_node
{
    MinimalRMTask task;
    struct rm_task_node *next;
} MinimalRMTaskNode;

typedef struct rm_schedule {
    schedtime_t hyper_period;
    schedtime_t (*get_time)(void);
    schedtime_t start_time;
    uint32_t task_count;
    MinimalRMTaskNode *head, *tail;
} MinimalRMSchedule;

void init_minimal_rmtask(MinimalRMTask *new_task, const uint16_t id, const schedtime_t period, const schedtime_t deadline, const uint32_t wcet, const schedtime_t earliest_release, void (*func)(const void *self));
MinimalRMSchedule init_minimal_rmschedule(const schedtime_t hyperperiod, const uint32_t num_tasks, schedtime_t (*get_time)(void));
MinimalRMTaskNode* create_rmtasknode(MinimalRMTask task);
void rmschedule_enqueue(MinimalRMSchedule* schedule, MinimalRMTask task);
void rmschedule_sortedinsert(MinimalRMTaskNode ** head_ref, MinimalRMTaskNode* new_node);
MinimalRMTaskNode* rmschedule_dequeue(MinimalRMSchedule* schedule);
void minimal_rm_scheduler(MinimalRMSchedule *schedule);
void sort_period_rmtasks(MinimalRMTask tasks_queue[], const uint32_t tasks_count);
void print_rmschedule(MinimalRMTaskNode* n);