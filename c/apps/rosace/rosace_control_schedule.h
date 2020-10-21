#pragma once

/*
 * This file was generated using SimpleSMTScheduler (https://github.com/egk696/SimpleSMTScheduler)
 * Generated schedule based on task set defined in examples/rosace_control.csv
 * Scheduled Task Set Utilization = 2.53825 %
 */

#define NUM_OF_TASKS 8
#define HYPER_PERIOD 400000

#define SYNC_ID 0
#define SYNC_PERIOD 20000
#define VL_FILTER_RECV_ID 1
#define VL_FILTER_RECV_PERIOD 40000
#define VZ_CONTROL_ID 2
#define VZ_CONTROL_PERIOD 80000
#define ALTI_HOLD_ID 3
#define ALTI_HOLD_PERIOD 80000
#define VA_CONTROL_ID 4
#define VA_CONTROL_PERIOD 80000
#define VL_CTRL_SEND_ID 5
#define VL_CTRL_SEND_PERIOD 80000
#define H_C0_ID 6
#define H_C0_PERIOD 400000
#define VA_C0_ID 7
#define VA_C0_PERIOD 400000

char* tasks_names[NUM_OF_TASKS] = {"SYNC", "VL_FILTER_RECV", "VZ_CONTROL", "ALTI_HOLD", "VA_CONTROL", "VL_CTRL_SEND", "H_C0", "VA_C0"};

unsigned long long tasks_periods[NUM_OF_TASKS] = {SYNC_PERIOD, VL_FILTER_RECV_PERIOD, VZ_CONTROL_PERIOD, ALTI_HOLD_PERIOD, VA_CONTROL_PERIOD, VL_CTRL_SEND_PERIOD, H_C0_PERIOD, VA_C0_PERIOD};

#define SYNC_INSTS_NUM 20
#define VL_FILTER_RECV_INSTS_NUM 10
#define VZ_CONTROL_INSTS_NUM 5
#define ALTI_HOLD_INSTS_NUM 5
#define VA_CONTROL_INSTS_NUM 5
#define VL_CTRL_SEND_INSTS_NUM 5
#define H_C0_INSTS_NUM 1
#define VA_C0_INSTS_NUM 1

unsigned tasks_insts_counts[NUM_OF_TASKS] = {SYNC_INSTS_NUM, VL_FILTER_RECV_INSTS_NUM, VZ_CONTROL_INSTS_NUM, ALTI_HOLD_INSTS_NUM, VA_CONTROL_INSTS_NUM, VL_CTRL_SEND_INSTS_NUM, H_C0_INSTS_NUM, VA_C0_INSTS_NUM};

unsigned long long SYNC_sched_insts[SYNC_INSTS_NUM] = {0, 20000, 40000, 60000, 80000, 100000, 120000, 140000, 160000, 180000, 200000, 220000, 240000, 260000, 280000, 300000, 320000, 340000, 360000, 380000};
unsigned long long VL_FILTER_RECV_sched_insts[VL_FILTER_RECV_INSTS_NUM] = {2137, 42137, 82137, 122137, 162137, 202137, 242137, 282137, 322137, 362137};
unsigned long long VZ_CONTROL_sched_insts[VZ_CONTROL_INSTS_NUM] = {2594, 82594, 162594, 242594, 322594};
unsigned long long ALTI_HOLD_sched_insts[ALTI_HOLD_INSTS_NUM] = {3212, 83212, 163212, 243212, 323212};
unsigned long long VA_CONTROL_sched_insts[VA_CONTROL_INSTS_NUM] = {3555, 83555, 163555, 243555, 323555};
unsigned long long VL_CTRL_SEND_sched_insts[VL_CTRL_SEND_INSTS_NUM] = {13242, 93242, 173242, 253242, 333242};
unsigned long long H_C0_sched_insts[H_C0_INSTS_NUM] = {274};
unsigned long long VA_C0_sched_insts[VA_C0_INSTS_NUM] = {473};

unsigned long long *tasks_schedules[NUM_OF_TASKS] = {SYNC_sched_insts, VL_FILTER_RECV_sched_insts, VZ_CONTROL_sched_insts, ALTI_HOLD_sched_insts, VA_CONTROL_sched_insts, VL_CTRL_SEND_sched_insts, H_C0_sched_insts, VA_C0_sched_insts};