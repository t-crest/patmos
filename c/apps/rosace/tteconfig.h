#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <machine/patmos.h>
#include "ethlib/eth_mac_driver.h"
#include "ethlib/icmp.h"
#include "ethlib/arp.h"
#include "ethlib/mac.h"
#include "ethlib/udp.h"
#include "ethlib/ipv4.h"
#include "ethlib/ptp1588.h"


// TTE Configuration
#define TTETIME_TO_NS         65536
#define TTE_MAX_TRANS_DELAY	  135600			//ns from net_config (eclipse project)
#define TTE_INT_PERIOD        10000000		//ns
#define TTE_CYC_PERIOD        1000000000		//ns
#define TTE_PRECISION         10000			//ns from network_description (eclipse project)

#define TTE_SYNC_WINDOW_HALF	10000		//ns
#define TTE_ASYNC2SYNC_THRES	10			//clusters
#define TTE_RECV_WINDOW_HALF	10000		//ns

// TTE PID synchronization
#define TTE_SYNC_Kp 1000LL
#define TTE_SYNC_Ki 0LL

// TTE directives
#define TIME_CORRECTION_EN
#define ALIGN_TO_REALTIME	//TODO: reset time on every new cluster
#define HW_TIMESTAMPING

// Constants
extern const char* eth_protocol_names[];
extern const unsigned int rx_buff_addr;
extern const unsigned int tx_buff_addr;
extern const unsigned char multicastip[4];
extern const unsigned char TTE_MAC[];
extern const unsigned char TTE_CT[];
extern const unsigned char TTE_DYN_VL[];
extern const unsigned char TTE_FILTER_VL[];
extern const unsigned char TTE_CTRL_VL[];

// Clock related variables
extern PTPPortInfo thisPtpPortInfo;
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

// Functions
int tte_pcf_handle(unsigned long long sched_rec_pit, unsigned long long schedule_start);
unsigned long long get_tte_aligned_time(unsigned long long current_time);