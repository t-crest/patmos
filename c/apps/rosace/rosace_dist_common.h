#pragma once

#include <stdlib.h>
#include <inttypes.h>
#include <math.h> //just for fabs
#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/rtc.h>
#include "printf.h"
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

#define STEP_TIME_SCALE 20  //ms
#define MAX_STEP_SIM (600000 / STEP_TIME_SCALE)
#define ALT_COMMAND_STEPSIM (50000 / STEP_TIME_SCALE)

#define schedtime_t uint64_t

#define SYNCTASK_GPIO_BIT	0
#define SENDTASK_GPIO_BIT	1
#define RECVTASK_GPIO_BIT	2
#define SCOPE_GPIO_BIT	3

// TTE Configuration
#define TTETIME_TO_NS         65536
#define TTE_MAX_TRANS_DELAY	  135600			//ns from net_config (eclipse project)
#define TTE_PRECISION         100000			//ns from network_description (eclipse project)

#define TTE_SYNC_WINDOW_HALF	40000000		//ns
#define TTE_RECV_WINDOW_HALF	25000		//ns

#define TTE_ASYNC2SYNC_THRES_CLUSTERS	5			//clusters
#define TTE_ASYNC2SYNC_THRES_CYCLES		0		//cycles

// TTE PID synchronization
#define TTE_SYNC_Kp 700LL
#define TTE_SYNC_Ki 300LL

// TTE directives
#define TIME_CORRECTION_EN
#define HW_TIMESTAMPING

// Macros
#define getLoWord(x)	(x & 0xffffffffU)
#define getHiWord(x)	(x >> 32)
#define getLoByte(x)	(x & 0xFF)
#define getHiByte(x)	((x >> 8) & 0xFF)

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
extern uint32_t step_simu;
extern uint32_t max_step_simu;

extern char* eth_protocol_names[];
extern unsigned int tx_buff_addr;
extern unsigned int rx_buff_addr;
extern unsigned int rx_bd_addr;
extern unsigned int rx_buff2_addr;
extern unsigned int rx_bd2_addr;
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
extern uint8_t enable_communication;
extern uint8_t enable_control;

// Rosace variables
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


// Messages Format
typedef struct {
	uint32_t step;
	double dynamics_va_filter_va;
	double dynamics_az_filter_az;
	double dynamics_vz_filter_vz;
	double dynamics_q_filter_q;
	double dynamics_h_filter_h;
} aircraft_state_message;

typedef struct{
	uint32_t step;
  double h_filter_alt_hold_h_f;
  double q_filter_va_control_q_f;
	double q_filter_vz_control_q_f;
  double az_filter_vz_control_az_f;
  double vz_filter_va_control_vz_f;
  double va_filter_va_control_va_f;
  double vz_filter_vz_control_vz_f;
} filter_state_message;
typedef struct{
	uint32_t step;
  double vz_control_elevator_delta_e_c;
  double vz_control_delta_e_c;
  double va_control_engine_delta_th_c;
  double va_control_delta_th_c;
} control_state_message;

// Functions 
uint64_t doubleToBytes(double x);
double bytesToDouble(uint64_t x);
void printSegmentInt(unsigned number);
unsigned getSegmentInt() ;
void config_ethmac();
void reset_sync();
void swap_eth_rx_buffers();
int eth_mac_poll_for_frames();
void copy_output_vars(output_t* v, uint64_t step);
int logging_fun(void *args);
unsigned long long get_tte_aligned_time(unsigned long long current_time, unsigned long long corr_limit);
void sync_fun(unsigned long long start_time, unsigned long long current_time, MinimalTTTask* tasks);
int udp_send_tte(unsigned int tx_addr, unsigned char tte_ct[], unsigned char tte_vl[], unsigned char tte_mac[], unsigned char destination_ip[], unsigned char source_ip[], unsigned short source_port, unsigned short destination_port, unsigned char data[], unsigned short data_length, uint16_t ipv4_id);
