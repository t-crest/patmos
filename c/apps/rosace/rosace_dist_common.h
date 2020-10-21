#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h> //just for fabs
#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/rtc.h>
#include "assemblage_includes.h"
#include "assemblage.h"
#include "io.h"
#include "tteconfig.h"

#define LOCAL_SPM_BASE 0x0
#define LEDS *((volatile _SPM unsigned int *) (PATMOS_IO_LED))   
#define KEYS *((volatile _SPM unsigned int *) (PATMOS_IO_KEYS))
#define GPIO *((volatile _SPM unsigned int *) (PATMOS_IO_GPIO))
#define SEGDISP *((volatile _SPM unsigned int *) (PATMOS_IO_SEGDISP))
#define DEADLINE *((volatile _SPM unsigned int *) (PATMOS_IO_DEADLINE))
#define CALL(val)   tasks[(val)].ne_t_body(NULL)

#define MAX_STEP_SIM 30000
#define STEP_TIME_SCALE 20  //ms

#define schedtime_t uint64_t

#define SYNCTASK_GPIO_BIT	0
#define SENDTASK_GPIO_BIT	1
#define RECVTASK_GPIO_BIT	2

// Task set
typedef int (*generic_task_fp)(void *);
typedef struct
{
	unsigned short id;
	schedtime_t period;
	schedtime_t *release_times;
	unsigned long release_inst;
	unsigned long nr_releases;
	schedtime_t last_time;
	schedtime_t delta_sum;
	unsigned long exec_count;
	generic_task_fp task_fun;
} MinimalTTTask;
typedef void (*task_sync_fp)(unsigned long long start_time, unsigned long long current_time, MinimalTTTask* tasks);

struct nonencoded_task_params* tasks;
int num_of_tasks;
schedtime_t hyper_period;
MinimalTTTask *schedule;

// Output variables
extern double aircraft_dynamics495_Va_Va_filter_100449_Va[2];
extern double Vz_control_50483_delta_e_c_elevator489_delta_e_c;
extern double Va_filter_100449_Va_f_Va_control_50474_Va_f[2];
extern double Vz_filter_100452_Vz_f_Va_control_50474_Vz_f[2];
extern double q_filter_100455_q_f_Va_control_50474_q_f[2];
extern double Va_c_Va_control_50474_Va_c;
extern double h_filter_100446_h_f_altitude_hold_50464_h_f[2];
extern double h_c_altitude_hold_50464_h_c;
extern double Va_control_50474_delta_th_c_delta_th_c;
extern double aircraft_dynamics495_az_az_filter_100458_az[2];
extern double aircraft_dynamics495_Vz_Vz_filter_100452_Vz[2];
extern double aircraft_dynamics495_q_q_filter_100455_q[2];
extern double elevator489_delta_e_aircraft_dynamics495_delta_e[3];
extern double engine486_T_aircraft_dynamics495_T[3];
extern double aircraft_dynamics495_h_h_filter_100446_h[2];
extern double Va_control_50474_delta_th_c_engine486_delta_th_c;
extern double Vz_filter_100452_Vz_f_Vz_control_50483_Vz_f[2];
extern double altitude_hold_50464_Vz_c_Vz_control_50483_Vz_c;
extern double q_filter_100455_q_f_Vz_control_50483_q_f[2];
extern double az_filter_100458_az_f_Vz_control_50483_az_f[2];
extern double Vz_control_50483_delta_e_c_delta_e_c;

// I/O
output_t outs;
uint64_t step_simu;
uint64_t max_step_simu;
schedtime_t start_time;

__attribute__((noinline))
void printSegmentInt(unsigned number);
__attribute__((noinline))
void copy_output_vars(output_t* v, uint64_t step);
__attribute__((noinline))
int logging_fun(void *args);
__attribute__((noinline))
void sync_fun(unsigned long long start_time, unsigned long long current_time, MinimalTTTask* tasks);
