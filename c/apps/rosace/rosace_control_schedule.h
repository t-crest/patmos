#pragma once

/*
 * This file was generated using SimpleSMTScheduler (https://github.com/egk696/SimpleSMTScheduler)
 * Generated schedule based on task set defined in examples/rosace_control.csv
 * Scheduled Task Set Utilization = 2.6716550000000003 %
 */

#define NUM_OF_TASKS 8
#define HYPER_PERIOD 400000000

#define VL_FILTER_RECV_ID 0
#define VL_FILTER_RECV_PERIOD 40000000
#define SYNC_ID 1
#define SYNC_PERIOD 80000000
#define VZ_CONTROL_ID 2
#define VZ_CONTROL_PERIOD 80000000
#define ALTI_HOLD_ID 3
#define ALTI_HOLD_PERIOD 80000000
#define VA_CONTROL_ID 4
#define VA_CONTROL_PERIOD 80000000
#define VL_CTRL_SEND_ID 5
#define VL_CTRL_SEND_PERIOD 80000000
#define H_C0_ID 6
#define H_C0_PERIOD 400000000
#define VA_C0_ID 7
#define VA_C0_PERIOD 400000000

char* tasks_names[NUM_OF_TASKS] = {"VL_FILTER_RECV", "SYNC", "VZ_CONTROL", "ALTI_HOLD", "VA_CONTROL", "VL_CTRL_SEND", "H_C0", "VA_C0"};

unsigned long long tasks_periods[NUM_OF_TASKS] = {VL_FILTER_RECV_PERIOD, SYNC_PERIOD, VZ_CONTROL_PERIOD, ALTI_HOLD_PERIOD, VA_CONTROL_PERIOD, VL_CTRL_SEND_PERIOD, H_C0_PERIOD, VA_C0_PERIOD};

#define VL_FILTER_RECV_INSTS_NUM 10
#define SYNC_INSTS_NUM 5
#define VZ_CONTROL_INSTS_NUM 5
#define ALTI_HOLD_INSTS_NUM 5
#define VA_CONTROL_INSTS_NUM 5
#define VL_CTRL_SEND_INSTS_NUM 5
#define H_C0_INSTS_NUM 1
#define VA_C0_INSTS_NUM 1

unsigned tasks_insts_counts[NUM_OF_TASKS] = {VL_FILTER_RECV_INSTS_NUM, SYNC_INSTS_NUM, VZ_CONTROL_INSTS_NUM, ALTI_HOLD_INSTS_NUM, VA_CONTROL_INSTS_NUM, VL_CTRL_SEND_INSTS_NUM, H_C0_INSTS_NUM, VA_C0_INSTS_NUM};

unsigned long long VL_FILTER_RECV_sched_insts[VL_FILTER_RECV_INSTS_NUM] = {2200000, 42200000, 82200000, 122200000, 162200000, 202200000, 242200000, 282200000, 322200000, 362200000};
unsigned long long SYNC_sched_insts[SYNC_INSTS_NUM] = {0, 80000000, 160000000, 240000000, 320000000};
unsigned long long VZ_CONTROL_sched_insts[VZ_CONTROL_INSTS_NUM] = {2502412, 82502412, 162502412, 242502412, 322502412};
unsigned long long ALTI_HOLD_sched_insts[ALTI_HOLD_INSTS_NUM] = {3441812, 83441812, 163441812, 243441812, 323441812};
unsigned long long VA_CONTROL_sched_insts[VA_CONTROL_INSTS_NUM] = {2935612, 82935612, 162935612, 242935612, 322935612};
unsigned long long VL_CTRL_SEND_sched_insts[VL_CTRL_SEND_INSTS_NUM] = {13200000, 93200000, 173200000, 253200000, 333200000};
unsigned long long H_C0_sched_insts[H_C0_INSTS_NUM] = {357925};
unsigned long long VA_C0_sched_insts[VA_C0_INSTS_NUM] = {372125};

unsigned long long *tasks_schedules[NUM_OF_TASKS] = {VL_FILTER_RECV_sched_insts, SYNC_sched_insts, VZ_CONTROL_sched_insts, ALTI_HOLD_sched_insts, VA_CONTROL_sched_insts, VL_CTRL_SEND_sched_insts, H_C0_sched_insts, VA_C0_sched_insts};