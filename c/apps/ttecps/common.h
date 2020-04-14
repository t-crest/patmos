#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <machine/patmos.h>
#include <machine/spm.h>
#define MAX(a, b) (a > b) ? a : b
#define MIN(a, b) (a < b) ? a : b

//IO
#define UART *((volatile _SPM unsigned int *) (PATMOS_IO_UART+0x4))
#define LEDS *((volatile _SPM unsigned int *) (PATMOS_IO_LED))
#define KEYS *((volatile _SPM unsigned int *) (PATMOS_IO_KEYS))
#define GPIO *((volatile _SPM unsigned int *) (PATMOS_IO_GPIO))
#define DEAD *((volatile _SPM unsigned int *) (PATMOS_IO_DEADLINE))
#define SEGM *((volatile _SPM unsinged int *) (PATMOS_IO_SEGDISP))

//System
#define CPU_PERIOD			12.5		//ns	

#define DEAD_FOR(TIME) {    \
  DEAD = TIME / CPU_PERIOD; \
  int val = DEAD;    \
}

// Types
typedef struct
{
    unsigned short x;
    unsigned short y;
    unsigned short z;
} SensorVector;

typedef struct
{
	unsigned int seq;
    SensorVector accel;
    SensorVector gyro;
    float theta;
    float duty_cycle;
} SimpleTTMessage;

typedef void (*generic_task_fp)(void);
typedef struct
{
	unsigned long long period;
    unsigned long long activation_time;
	unsigned long long last_time;
	unsigned long long delta_sum;
	unsigned long exec_count;
    generic_task_fp task_fp;
} SimpleTTETask;

typedef void (*task_sync_fp)(unsigned long long start_time, unsigned long long schedule_time, SimpleTTETask* tasks);
typedef void (*task_sense_fp)(SimpleTTMessage *outgoing_message);
typedef void (*task_recv_fp)(SimpleTTMessage* message, int length);
typedef void (*task_calc_fp)(const SimpleTTMessage *incoming_message, SimpleTTMessage *outgoing_message);
typedef void (*task_send_fp)(const SimpleTTMessage* outgoing_message, int length, const unsigned char VL[2]);
typedef void (*task_pulse_fp)(unsigned int duty_cycle);

//Tasks
void task_sync(unsigned long long start_time, unsigned long long schedule_time, SimpleTTETask* tasks);
void task_sense(SimpleTTMessage *outgoing_message);
void task_recv(SimpleTTMessage* message, int length);
void task_calc(const SimpleTTMessage *incoming_message, SimpleTTMessage *outgoing_message);
void task_send(const SimpleTTMessage* outgoing_message, int length, const unsigned char VL[2]);
void task_pulse(unsigned int duty_cycle);

//Common functions
float atan2_approximation1(float y, float x);
float atan2_approximation2(float y, float x );
int atan2_approximation3(int y, int x);
void cyclic_executive_loop(SimpleTTETask* task_schedule);
void sort_asc_ttetasks(SimpleTTETask *tasks, const unsigned short num_tasks);