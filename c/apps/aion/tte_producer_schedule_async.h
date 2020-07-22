#pragma once

/*
 * This file was generated using SimpleSMTScheduler (https://github.com/egk696/SimpleSMTScheduler)
 * Generated schedule based on task set defined in examples/tte_producer.csv
 * Scheduled Task Set Utilization = 17.991685 %
 */

#define NUM_OF_TASKS 5
#define HYPER_PERIOD 20000000

#define task_syn_ID 0
#define PRODUCER_SYN_PERIOD 10000000
#define task_calc_ID 1
#define PRODUCER_CALC_PERIOD 5000000
#define task_send_ID 2
#define PRODUCER_SEND_PERIOD 5000000
#define task_recv_ID 3
#define PRODUCER_RECV_PERIOD 5000000
#define task_pulse_ID 4
#define PRODUCER_PULSE_PERIOD 20000000

unsigned long long tasks_periods[NUM_OF_TASKS] = {PRODUCER_SYN_PERIOD, PRODUCER_CALC_PERIOD, PRODUCER_SEND_PERIOD, PRODUCER_RECV_PERIOD, PRODUCER_PULSE_PERIOD};

#define PRODUCER_SYN_INSTS_NUM 2
#define PRODUCER_CALC_INSTS_NUM 4
#define PRODUCER_SEND_INSTS_NUM 4
#define PRODUCER_RECV_INSTS_NUM 4
#define PRODUCER_PULSE_INSTS_NUM 1

unsigned tasks_insts_counts[NUM_OF_TASKS] = {PRODUCER_SYN_INSTS_NUM, PRODUCER_CALC_INSTS_NUM, PRODUCER_SEND_INSTS_NUM, PRODUCER_RECV_INSTS_NUM, PRODUCER_PULSE_INSTS_NUM};

unsigned long long PRODUCER_SYN_sched_insts[PRODUCER_SYN_INSTS_NUM] = {0, 10000000};
unsigned long long PRODUCER_CALC_sched_insts[PRODUCER_CALC_INSTS_NUM] = {1338100, 6338100, 11338100, 16338100};
unsigned long long PRODUCER_SEND_sched_insts[PRODUCER_SEND_INSTS_NUM] = {3600000, 8600000, 13600000, 18600000};
unsigned long long PRODUCER_RECV_sched_insts[PRODUCER_RECV_INSTS_NUM] = {1200000, 6200000, 11200000, 16200000};
unsigned long long PRODUCER_PULSE_sched_insts[PRODUCER_PULSE_INSTS_NUM] = {4061413};

unsigned long long *tasks_schedules[NUM_OF_TASKS] = {PRODUCER_SYN_sched_insts, PRODUCER_CALC_sched_insts, PRODUCER_SEND_sched_insts, PRODUCER_RECV_sched_insts, PRODUCER_PULSE_sched_insts};