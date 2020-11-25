#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h> //just for fabs
#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/rtc.h>
#include "io.h"
#include "ethlib/eth_mac_driver.h"
#include "ethlib/icmp.h"
#include "ethlib/arp.h"
#include "ethlib/mac.h"
#include "ethlib/udp.h"
#include "ethlib/ipv4.h"
#include "ethlib/ptp1588.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

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

// TTE Configuration
#define TTETIME_TO_NS         65536
#define TTE_MAX_TRANS_DELAY	  135600			//ns from net_config (eclipse project)
#define TTE_INT_PERIOD        20000000		//ns
#define TTE_CYC_PERIOD        4000000000		//ns
#define TTE_PRECISION         100000			//ns from network_description (eclipse project)

#define TTE_SYNC_WINDOW_HALF	10000		//ns
#define TTE_ASYNC2SYNC_THRES	10			//clusters
#define TTE_RECV_WINDOW_HALF	10000		//ns

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

extern struct nonencoded_task_params* tasks;
extern int num_of_tasks;
extern schedtime_t hyper_period;
extern MinimalTTTask *schedule;

// Output variables
extern output_t outs;
extern uint64_t step_simu;
extern uint64_t max_step_simu;

extern char* eth_protocol_names[];
extern unsigned int rx_buff_addr;
extern unsigned int tx_buff_addr;
extern unsigned char multicastip[4];
extern unsigned char TTE_MAC[];
extern unsigned char TTE_CT[];
extern unsigned char TTE_DYN_VL[];
extern unsigned char TTE_FILTER_VL[];
extern unsigned char TTE_CTRL_VL[];
// Clock related variables
extern PTPv2Time hardTimestamp;
extern PTPv2Time softTimestamp;
extern long long clkDiff;
extern long long clkDiffLast;
extern long long clkDiffSum;
extern unsigned short integration_cycle;
extern unsigned long long coldStartIntegrationTime;
extern unsigned long long initNanoseconds;
extern unsigned long long initSeconds; 
// Counters
extern unsigned short rxPcfCount;
extern unsigned int stableCycles;
extern unsigned int unstableCycles;
extern unsigned int stableClusters;
extern unsigned int unstableClusters;
extern unsigned int totalScheduledReceptions;
// Flags
extern unsigned char nodeIntegrated;	//is used to indicate when the node has achieved sufficient syncrhonization
extern unsigned char nodeSyncStable;	//is used to enable task execution when the node is in a stable sync
extern unsigned char nodeColdStart;		//is used to indicate that a node has just booted and has not received a single PCF
extern unsigned char nodeFirstSync;
extern unsigned char nodeRecvEnable;

double aircraft_dynamics495_Va_Va_filter_100449_Va[2];
double Vz_control_50483_delta_e_c_elevator489_delta_e_c;
double Va_filter_100449_Va_f_Va_control_50474_Va_f[2];
double Vz_filter_100452_Vz_f_Va_control_50474_Vz_f[2];
double q_filter_100455_q_f_Va_control_50474_q_f[2];
double Va_c_Va_control_50474_Va_c;
double h_filter_100446_h_f_altitude_hold_50464_h_f[2];
double h_c_altitude_hold_50464_h_c;
double Va_control_50474_delta_th_c_delta_th_c;
double aircraft_dynamics495_az_az_filter_100458_az[2];
double aircraft_dynamics495_Vz_Vz_filter_100452_Vz[2];
double aircraft_dynamics495_q_q_filter_100455_q[2];
double elevator489_delta_e_aircraft_dynamics495_delta_e[3];
double engine486_T_aircraft_dynamics495_T[3];
double aircraft_dynamics495_h_h_filter_100446_h[2];
double Va_control_50474_delta_th_c_engine486_delta_th_c;
double Vz_filter_100452_Vz_f_Vz_control_50483_Vz_f[2];
double altitude_hold_50464_Vz_c_Vz_control_50483_Vz_c;
double q_filter_100455_q_f_Vz_control_50483_q_f[2];
double az_filter_100458_az_f_Vz_control_50483_az_f[2];
double Vz_control_50483_delta_e_c_delta_e_c;

void printSegmentInt(unsigned number);
void copy_output_vars(output_t* v, uint64_t step);
int logging_fun(void *args);
void init_sync();
uint8_t isNodeSyncStable();
unsigned long long get_tte_aligned_time(unsigned long long current_time, unsigned long long corr_limit);
void sync_fun(unsigned long long start_time, unsigned long long current_time, MinimalTTTask* tasks);
