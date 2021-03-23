#pragma once

/*
 * This file was generated using SimpleSMTScheduler (https://github.com/egk696/SimpleSMTScheduler)
 * Generated schedule based on task set defined in examples/rosace_aircraft.csv
 * Scheduled Task Set Utilization = 81.6758125 %
 */

#define NUM_OF_TASKS 9
#define HYPER_PERIOD 80000000

#define ENGINE_ID 0
#define ENGINE_PERIOD 20000000
#define AIRCRAFT_DYN_ID 1
#define AIRCRAFT_DYN_PERIOD 20000000
#define ELEVATOR_ID 2
#define ELEVATOR_PERIOD 20000000
#define LOGGING_ID 3
#define LOGGING_PERIOD 20000000
#define VL_DYN_SEND_ID 4
#define VL_DYN_SEND_PERIOD 20000000
#define SYNC_ID 5
#define SYNC_PERIOD 80000000
#define DELTA_E_C0_ID 6
#define DELTA_E_C0_PERIOD 80000000
#define DELTA_TH_C0_ID 7
#define DELTA_TH_C0_PERIOD 80000000
#define VL_CTRL_RECV_ID 8
#define VL_CTRL_RECV_PERIOD 80000000

char* tasks_names[NUM_OF_TASKS] = {"ENGINE", "AIRCRAFT_DYN", "ELEVATOR", "LOGGING", "VL_DYN_SEND", "SYNC", "DELTA_E_C0", "DELTA_TH_C0", "VL_CTRL_RECV"};

unsigned long long tasks_periods[NUM_OF_TASKS] = {ENGINE_PERIOD, AIRCRAFT_DYN_PERIOD, ELEVATOR_PERIOD, LOGGING_PERIOD, VL_DYN_SEND_PERIOD, SYNC_PERIOD, DELTA_E_C0_PERIOD, DELTA_TH_C0_PERIOD, VL_CTRL_RECV_PERIOD};

#define ENGINE_INSTS_NUM 4
#define AIRCRAFT_DYN_INSTS_NUM 4
#define ELEVATOR_INSTS_NUM 4
#define LOGGING_INSTS_NUM 4
#define VL_DYN_SEND_INSTS_NUM 4
#define SYNC_INSTS_NUM 1
#define DELTA_E_C0_INSTS_NUM 1
#define DELTA_TH_C0_INSTS_NUM 1
#define VL_CTRL_RECV_INSTS_NUM 1

unsigned tasks_insts_counts[NUM_OF_TASKS] = {ENGINE_INSTS_NUM, AIRCRAFT_DYN_INSTS_NUM, ELEVATOR_INSTS_NUM, LOGGING_INSTS_NUM, VL_DYN_SEND_INSTS_NUM, SYNC_INSTS_NUM, DELTA_E_C0_INSTS_NUM, DELTA_TH_C0_INSTS_NUM, VL_CTRL_RECV_INSTS_NUM};

unsigned long long ENGINE_sched_insts[ENGINE_INSTS_NUM] = {14079475, 34079475, 54079475, 74079475};
unsigned long long AIRCRAFT_DYN_sched_insts[AIRCRAFT_DYN_INSTS_NUM] = {357925, 20357925, 40357925, 60357925};
unsigned long long ELEVATOR_sched_insts[ELEVATOR_INSTS_NUM] = {14242675, 34242675, 54242675, 74242675};
unsigned long long LOGGING_sched_insts[LOGGING_INSTS_NUM] = {14675275, 34675275, 54675275, 74675275};
unsigned long long VL_DYN_SEND_sched_insts[VL_DYN_SEND_INSTS_NUM] = {14000000, 34000000, 54000000, 74000000};
unsigned long long SYNC_sched_insts[SYNC_INSTS_NUM] = {0};
unsigned long long DELTA_E_C0_sched_insts[DELTA_E_C0_INSTS_NUM] = {14670875};
unsigned long long DELTA_TH_C0_sched_insts[DELTA_TH_C0_INSTS_NUM] = {14673075};
unsigned long long VL_CTRL_RECV_sched_insts[VL_CTRL_RECV_INSTS_NUM] = {13600000};

unsigned long long *tasks_schedules[NUM_OF_TASKS] = {ENGINE_sched_insts, AIRCRAFT_DYN_sched_insts, ELEVATOR_sched_insts, LOGGING_sched_insts, VL_DYN_SEND_sched_insts, SYNC_sched_insts, DELTA_E_C0_sched_insts, DELTA_TH_C0_sched_insts, VL_CTRL_RECV_sched_insts};