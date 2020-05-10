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
#define TTETIME_TO_NS		65536
#define TTE_MAX_TRANS_DELAY	135600			//ns from net_config (eclipse project)
#define TTE_INT_PERIOD		10000000		//ns
#define TTE_CYC_PERIOD		1000000000		//ns
#define TTE_PRECISION		10000			//ns from network_description (eclipse project)

// TTE PID synchronization
#define TTE_SYNC_Kp 1000LL
#define TTE_SYNC_Ki 5LL

// TTE directives
#define TIME_CORRECTION_EN

//Constants
extern const char* eth_protocol_names[];
extern const unsigned int rx_buff_addr;
extern const unsigned int tx_buff_addr;
extern const unsigned char multicastip[4];
extern const unsigned char TTE_MAC[];
extern const unsigned char TTE_CT[];
extern const unsigned char TTE_SENSE_VL[];
extern const unsigned char TTE_CONTROL_VL[];
