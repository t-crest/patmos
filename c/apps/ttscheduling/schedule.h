#pragma once

/*
 * This file was generated using SimpleSMTScheduler (https://github.com/egk696/SimpleSMTScheduler)
 * Generated schedule based on task set defined in examples/demo_tasks.csv
 * Scheduled Task Set Utilization = 74.1 %
 */

#define NUM_OF_TASKS 8
#define HYPER_PERIOD 50000000

#define T1_PERIOD 5000000
#define T2_PERIOD 10000000
#define T3_PERIOD 2500000
#define T4_PERIOD 50000000
#define T5_PERIOD 5000000
#define T6_PERIOD 10000000
#define T7_PERIOD 2500000
#define T8_PERIOD 50000000

schedtime_t tasks_periods[NUM_OF_TASKS] = {T1_PERIOD, T2_PERIOD, T3_PERIOD, T4_PERIOD, T5_PERIOD, T6_PERIOD, T7_PERIOD, T8_PERIOD};

#define T1_INSTS_NUM 10
#define T2_INSTS_NUM 5
#define T3_INSTS_NUM 20
#define T4_INSTS_NUM 1
#define T5_INSTS_NUM 10
#define T6_INSTS_NUM 5
#define T7_INSTS_NUM 20
#define T8_INSTS_NUM 1

unsigned tasks_insts_counts[NUM_OF_TASKS] = {T1_INSTS_NUM, T2_INSTS_NUM, T3_INSTS_NUM, T4_INSTS_NUM, T5_INSTS_NUM, T6_INSTS_NUM, T7_INSTS_NUM, T8_INSTS_NUM};

schedtime_t T1_sched_insts[T1_INSTS_NUM] = {680000, 5680000, 10680000, 15680000, 20680000, 25680000, 30680000, 35680000, 40680000, 45680000};
schedtime_t T2_sched_insts[T2_INSTS_NUM] = {3180000, 13180000, 23180000, 33180000, 43180000};
schedtime_t T3_sched_insts[T3_INSTS_NUM] = {2260000, 4760000, 7260000, 9760000, 12260000, 14760000, 17260000, 19760000, 22260000, 24760000, 27260000, 29760000, 32260000, 34760000, 37260000, 39760000, 42260000, 44760000, 47260000, 49760000};
schedtime_t T4_sched_insts[T4_INSTS_NUM] = {9220000};
schedtime_t T5_sched_insts[T5_INSTS_NUM] = {970000, 5970000, 10970000, 15970000, 20970000, 25970000, 30970000, 35970000, 40970000, 45970000};
schedtime_t T6_sched_insts[T6_INSTS_NUM] = {3720000, 13720000, 23720000, 33720000, 43720000};
schedtime_t T7_sched_insts[T7_INSTS_NUM] = {0, 2500000, 5000000, 7500000, 10000000, 12500000, 15000000, 17500000, 20000000, 22500000, 25000000, 27500000, 30000000, 32500000, 35000000, 37500000, 40000000, 42500000, 45000000, 47500000};
schedtime_t T8_sched_insts[T8_INSTS_NUM] = {18220000};

schedtime_t *tasks_schedules[NUM_OF_TASKS] = {T1_sched_insts, T2_sched_insts, T3_sched_insts, T4_sched_insts, T5_sched_insts, T6_sched_insts, T7_sched_insts, T8_sched_insts};
