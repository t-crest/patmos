#pragma once

#include <stdlib.h>
#include <inttypes.h>
#include <math.h> //just for fabs
#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/rtc.h>
#include "helpers/printf.h"
#include "onera/io.h"
#include "ethlib/eth_mac_driver.h"
#include "ethlib/icmp.h"
#include "ethlib/arp.h"
#include "ethlib/mac.h"
#include "ethlib/udp.h"
#include "ethlib/ipv4.h"
#include "ethlib/ptp1588.h"
#include "types.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define LEDS *((volatile _SPM unsigned int *) (PATMOS_IO_LED))   
#define KEYS *((volatile _SPM unsigned int *) (PATMOS_IO_KEYS))
#define GPIO *((volatile _SPM unsigned int *) (PATMOS_IO_GPIO))
#define SEGDISP *((volatile _SPM unsigned int *) (PATMOS_IO_SEGDISP))
#define DEADLINE *((volatile _SPM unsigned int *) (PATMOS_IO_DEADLINE))

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
#define TTE_RECV_WINDOW_HALF	50000		//ns

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
	schedtime_t exec_time;
	unsigned long exec_count;
	generic_task_fp task_fun;
} MinimalTTTask;
typedef void (*task_sync_fp)(unsigned long long start_time, unsigned long long current_time, MinimalTTTask* tasks);

extern struct nonencoded_task_params* tasks;
extern int num_of_tasks;
extern schedtime_t hyper_period;
extern MinimalTTTask *schedule;

// Messages Format
typedef struct {
	uint32_t step;
	uint8_t enable_filter;
	REAL_TYPE engine_dynamics_T;
	REAL_TYPE elevator_dynamics_delta_e;
	struct aircraft_dynamics_outs_t dynamics;
} aircraft_state_message;

typedef struct{
	uint32_t step;
	uint8_t enable_control;
  REAL_TYPE h_meas;
  REAL_TYPE q_meas;
  REAL_TYPE az_meas;
  REAL_TYPE vz_meas;
  REAL_TYPE va_meas;
} filter_state_message;
typedef struct{
	uint32_t step;
	uint32_t controlling;
	REAL_TYPE h_c;
	REAL_TYPE Va_c;
	REAL_TYPE Vz_c;
  REAL_TYPE delta_e_c;
  REAL_TYPE delta_th_c;
} control_state_message;

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

// Functions 
uint64_t REAL_TYPEToBytes(REAL_TYPE x);
REAL_TYPE bytesToDouble(uint64_t x);
void printSegmentInt(unsigned number);
unsigned getSegmentInt() ;
void config_ethmac();
void reset_sync();
void swap_eth_rx_buffers();
int eth_mac_poll_for_frames();
void copy_output_vars(output_t* v, uint64_t step);
unsigned long long get_tte_aligned_time(unsigned long long current_time, unsigned long long corr_limit);
void sync_fun(unsigned long long start_time, unsigned long long current_time, MinimalTTTask* tasks);
int udp_send_tte(unsigned int tx_addr, unsigned char tte_ct[], unsigned char tte_vl[], unsigned char tte_mac[], unsigned char destination_ip[], unsigned char source_ip[], unsigned short source_port, unsigned short destination_port, unsigned char data[], unsigned short data_length, uint16_t ipv4_id);
