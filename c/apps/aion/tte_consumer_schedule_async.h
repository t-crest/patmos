#pragma once

/*
 * This file was generated using SimpleSMTScheduler (https://github.com/egk696/SimpleSMTScheduler)
 * Generated schedule based on task set defined in examples/tte_consumer.csv
 * Scheduled Task Set Utilization = 21.171445 %
 */

#define NUM_OF_TASKS 5
#define HYPER_PERIOD 20000000

#define task_syn_ID 0
#define CONSUMER_SYN_PERIOD 10000000
#define task_calc_ID 1
#define CONSUMER_CALC_PERIOD 5000000
#define task_send_ID 2
#define CONSUMER_SEND_PERIOD 5000000
#define task_recv_ID 3
#define CONSUMER_RECV_PERIOD 5000000
#define task_pulse_ID 4
#define CONSUMER_PULSE_PERIOD 20000000

unsigned long long tasks_periods[NUM_OF_TASKS] = {CONSUMER_SYN_PERIOD, CONSUMER_CALC_PERIOD, CONSUMER_SEND_PERIOD, CONSUMER_RECV_PERIOD, CONSUMER_PULSE_PERIOD};

#define CONSUMER_SYN_INSTS_NUM 2
#define CONSUMER_CALC_INSTS_NUM 4
#define CONSUMER_SEND_INSTS_NUM 4
#define CONSUMER_RECV_INSTS_NUM 4
#define CONSUMER_PULSE_INSTS_NUM 1

unsigned tasks_insts_counts[NUM_OF_TASKS] = {CONSUMER_SYN_INSTS_NUM, CONSUMER_CALC_INSTS_NUM, CONSUMER_SEND_INSTS_NUM, CONSUMER_RECV_INSTS_NUM, CONSUMER_PULSE_INSTS_NUM};

unsigned long long CONSUMER_SYN_sched_insts[CONSUMER_SYN_INSTS_NUM] = {0, 10000000};
unsigned long long CONSUMER_CALC_sched_insts[CONSUMER_CALC_INSTS_NUM] = {498075, 5498075, 10498075, 15498075};
unsigned long long CONSUMER_SEND_sched_insts[CONSUMER_SEND_INSTS_NUM] = {800000, 5800000, 10800000, 15800000};
unsigned long long CONSUMER_RECV_sched_insts[CONSUMER_RECV_INSTS_NUM] = {4000000, 9000000, 14000000, 19000000};
unsigned long long CONSUMER_PULSE_sched_insts[CONSUMER_PULSE_INSTS_NUM] = {938100};

unsigned long long *tasks_schedules[NUM_OF_TASKS] = {CONSUMER_SYN_sched_insts, CONSUMER_CALC_sched_insts, CONSUMER_SEND_sched_insts, CONSUMER_RECV_sched_insts, CONSUMER_PULSE_sched_insts};