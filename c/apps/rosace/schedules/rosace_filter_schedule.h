#pragma once

/*
 * This file was generated using SimpleSMTScheduler (https://github.com/egk696/SimpleSMTScheduler)
 * Generated schedule based on task set defined in examples/rosace_filters.csv
 * Scheduled Task Set Utilization = 4.55596875 %
 */

#define NUM_OF_TASKS 8
#define HYPER_PERIOD 80000000

#define MAPPED_CORE_COUNT 1

#define VL_DYN_RECV_ID 0
#define VL_DYN_RECV_PERIOD 20000000
#define Q_FILTER_ID 1
#define Q_FILTER_PERIOD 40000000
#define VZ_FILTER_ID 2
#define VZ_FILTER_PERIOD 40000000
#define AZ_FILTER_ID 3
#define AZ_FILTER_PERIOD 40000000
#define VA_FILTER_ID 4
#define VA_FILTER_PERIOD 40000000
#define H_FILTER_ID 5
#define H_FILTER_PERIOD 40000000
#define VL_FILTER_SEND_ID 6
#define VL_FILTER_SEND_PERIOD 40000000
#define SYNC_ID 7
#define SYNC_PERIOD 80000000

char* tasks_names[NUM_OF_TASKS] = {"VL_DYN_RECV", "Q_FILTER", "VZ_FILTER", "AZ_FILTER", "VA_FILTER", "H_FILTER", "VL_FILTER_SEND", "SYNC"};

unsigned tasks_per_cores[MAPPED_CORE_COUNT] = {8};

unsigned cores_hyperperiods[MAPPED_CORE_COUNT] = {80000000};

unsigned tasks_coreids[NUM_OF_TASKS] = {0, 0, 0, 0, 0, 0, 0, 0};

unsigned long long tasks_periods[NUM_OF_TASKS] = {VL_DYN_RECV_PERIOD, Q_FILTER_PERIOD, VZ_FILTER_PERIOD, AZ_FILTER_PERIOD, VA_FILTER_PERIOD, H_FILTER_PERIOD, VL_FILTER_SEND_PERIOD, SYNC_PERIOD};

#define VL_DYN_RECV_INSTS_NUM 4
#define Q_FILTER_INSTS_NUM 2
#define VZ_FILTER_INSTS_NUM 2
#define AZ_FILTER_INSTS_NUM 2
#define VA_FILTER_INSTS_NUM 2
#define H_FILTER_INSTS_NUM 2
#define VL_FILTER_SEND_INSTS_NUM 2
#define SYNC_INSTS_NUM 1

unsigned tasks_insts_counts[NUM_OF_TASKS] = {VL_DYN_RECV_INSTS_NUM, Q_FILTER_INSTS_NUM, VZ_FILTER_INSTS_NUM, AZ_FILTER_INSTS_NUM, VA_FILTER_INSTS_NUM, H_FILTER_INSTS_NUM, VL_FILTER_SEND_INSTS_NUM, SYNC_INSTS_NUM};

unsigned long long VL_DYN_RECV_sched_insts[4] = {14600000, 34600000, 54600000, 74600000};
unsigned long long Q_FILTER_sched_insts[2] = {15100000, 55100000};
unsigned long long VZ_FILTER_sched_insts[2] = {15500000, 55500000};
unsigned long long AZ_FILTER_sched_insts[2] = {600000, 40600000};
unsigned long long VA_FILTER_sched_insts[2] = {1200000, 41200000};
unsigned long long H_FILTER_sched_insts[2] = {1600000, 41600000};
unsigned long long VL_FILTER_SEND_sched_insts[2] = {2000000, 42000000};
unsigned long long SYNC_sched_insts[1] = {0};

unsigned long long *tasks_schedules[NUM_OF_TASKS] = {VL_DYN_RECV_sched_insts, Q_FILTER_sched_insts, VZ_FILTER_sched_insts, AZ_FILTER_sched_insts, VA_FILTER_sched_insts, H_FILTER_sched_insts, VL_FILTER_SEND_sched_insts, SYNC_sched_insts};