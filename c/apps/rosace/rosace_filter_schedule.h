#pragma once

/*
 * This file was generated using SimpleSMTScheduler (https://github.com/egk696/SimpleSMTScheduler)
 * Generated schedule based on task set defined in examples/rosace_filters.csv
 * Scheduled Task Set Utilization = 4.4350000000000005 %
 */

#define NUM_OF_TASKS 8
#define HYPER_PERIOD 40000000

#define SYNC_ID 0
#define SYNC_PERIOD 20000000
#define VL_DYN_RECV_ID 1
#define VL_DYN_RECV_PERIOD 20000000
#define Q_FILTER_ID 2
#define Q_FILTER_PERIOD 40000000
#define VZ_FILTER_ID 3
#define VZ_FILTER_PERIOD 40000000
#define AZ_FILTER_ID 4
#define AZ_FILTER_PERIOD 40000000
#define VA_FILTER_ID 5
#define VA_FILTER_PERIOD 40000000
#define H_FILTER_ID 6
#define H_FILTER_PERIOD 40000000
#define VL_FILTER_SEND_ID 7
#define VL_FILTER_SEND_PERIOD 40000000

char* tasks_names[NUM_OF_TASKS] = {"SYNC", "VL_DYN_RECV", "Q_FILTER", "VZ_FILTER", "AZ_FILTER", "VA_FILTER", "H_FILTER", "VL_FILTER_SEND"};

unsigned long long tasks_periods[NUM_OF_TASKS] = {SYNC_PERIOD, VL_DYN_RECV_PERIOD, Q_FILTER_PERIOD, VZ_FILTER_PERIOD, AZ_FILTER_PERIOD, VA_FILTER_PERIOD, H_FILTER_PERIOD, VL_FILTER_SEND_PERIOD};

#define SYNC_INSTS_NUM 2
#define VL_DYN_RECV_INSTS_NUM 2
#define Q_FILTER_INSTS_NUM 1
#define VZ_FILTER_INSTS_NUM 1
#define AZ_FILTER_INSTS_NUM 1
#define VA_FILTER_INSTS_NUM 1
#define H_FILTER_INSTS_NUM 1
#define VL_FILTER_SEND_INSTS_NUM 1

unsigned tasks_insts_counts[NUM_OF_TASKS] = {SYNC_INSTS_NUM, VL_DYN_RECV_INSTS_NUM, Q_FILTER_INSTS_NUM, VZ_FILTER_INSTS_NUM, AZ_FILTER_INSTS_NUM, VA_FILTER_INSTS_NUM, H_FILTER_INSTS_NUM, VL_FILTER_SEND_INSTS_NUM};

unsigned long long SYNC_sched_insts[SYNC_INSTS_NUM] = {0, 20000000};
unsigned long long VL_DYN_RECV_sched_insts[VL_DYN_RECV_INSTS_NUM] = {14200000, 34200000};
unsigned long long Q_FILTER_sched_insts[Q_FILTER_INSTS_NUM] = {90185};
unsigned long long VZ_FILTER_sched_insts[VZ_FILTER_INSTS_NUM] = {34488185};
unsigned long long AZ_FILTER_sched_insts[AZ_FILTER_INSTS_NUM] = {284370};
unsigned long long VA_FILTER_sched_insts[VA_FILTER_INSTS_NUM] = {473555};
unsigned long long H_FILTER_sched_insts[H_FILTER_INSTS_NUM] = {662740};
unsigned long long VL_FILTER_SEND_sched_insts[VL_FILTER_SEND_INSTS_NUM] = {2000000};

unsigned long long *tasks_schedules[NUM_OF_TASKS] = {SYNC_sched_insts, VL_DYN_RECV_sched_insts, Q_FILTER_sched_insts, VZ_FILTER_sched_insts, AZ_FILTER_sched_insts, VA_FILTER_sched_insts, H_FILTER_sched_insts, VL_FILTER_SEND_sched_insts};