#pragma once

/*
 * This file was generated using SimpleSMTScheduler (https://github.com/egk696/SimpleSMTScheduler)
 * Generated schedule based on task set defined in examples/rosace_aircraft.csv
 * Scheduled Task Set Utilization = 81.6708125 %
 */

#define NUM_OF_TASKS 7
#define HYPER_PERIOD 80000000

#define MAPPED_CORE_COUNT 1

#define ENGINE_ID 0
#define ENGINE_PERIOD 20000000
#define ELEVATOR_ID 1
#define ELEVATOR_PERIOD 20000000
#define AIRCRAFT_DYN_ID 2
#define AIRCRAFT_DYN_PERIOD 20000000
#define VL_DYN_SEND_ID 3
#define VL_DYN_SEND_PERIOD 20000000
#define LOGGING_ID 4
#define LOGGING_PERIOD 20000000
#define SYNC_ID 5
#define SYNC_PERIOD 80000000
#define VL_CTRL_RECV_ID 6
#define VL_CTRL_RECV_PERIOD 80000000

char* tasks_names[NUM_OF_TASKS] = {"ENGINE", "ELEVATOR", "AIRCRAFT_DYN", "VL_DYN_SEND", "LOGGING", "SYNC", "VL_CTRL_RECV"};

unsigned tasks_per_cores[MAPPED_CORE_COUNT] = {7};

unsigned cores_hyperperiods[MAPPED_CORE_COUNT] = {80000000};

unsigned tasks_coreids[NUM_OF_TASKS] = {0, 0, 0, 0, 0, 0, 0};

unsigned long long tasks_periods[NUM_OF_TASKS] = {ENGINE_PERIOD, ELEVATOR_PERIOD, AIRCRAFT_DYN_PERIOD, VL_DYN_SEND_PERIOD, LOGGING_PERIOD, SYNC_PERIOD, VL_CTRL_RECV_PERIOD};

#define ENGINE_INSTS_NUM 4
#define ELEVATOR_INSTS_NUM 4
#define AIRCRAFT_DYN_INSTS_NUM 4
#define VL_DYN_SEND_INSTS_NUM 4
#define LOGGING_INSTS_NUM 4
#define SYNC_INSTS_NUM 1
#define VL_CTRL_RECV_INSTS_NUM 1

unsigned tasks_insts_counts[NUM_OF_TASKS] = {ENGINE_INSTS_NUM, ELEVATOR_INSTS_NUM, AIRCRAFT_DYN_INSTS_NUM, VL_DYN_SEND_INSTS_NUM, LOGGING_INSTS_NUM, SYNC_INSTS_NUM, VL_CTRL_RECV_INSTS_NUM};

unsigned long long ENGINE_sched_insts[4] = {14200000, 34200000, 54200000, 74200000};
unsigned long long ELEVATOR_sched_insts[4] = {16800000, 36800000, 56800000, 76800000};
unsigned long long AIRCRAFT_DYN_sched_insts[4] = {500000, 20500000, 40500000, 60500000};
unsigned long long VL_DYN_SEND_sched_insts[4] = {14000000, 34000000, 54000000, 74000000};
unsigned long long LOGGING_sched_insts[4] = {17400000, 37400000, 57400000, 77400000};
unsigned long long SYNC_sched_insts[1] = {0};
unsigned long long VL_CTRL_RECV_sched_insts[1] = {13600000};

unsigned long long *tasks_schedules[NUM_OF_TASKS] = {ENGINE_sched_insts, ELEVATOR_sched_insts, AIRCRAFT_DYN_sched_insts, VL_DYN_SEND_sched_insts, LOGGING_sched_insts, SYNC_sched_insts, VL_CTRL_RECV_sched_insts};