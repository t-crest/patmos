/*
   Copyright 2014 Technical University of Denmark, DTU Compute. 
   All rights reserved.
   
   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

/*
	Master demo file for ptplib (IEEE 1588v2 Precise-Time-Protocol) demo

	Author: Eleftherios Kyriakakis 
	Copyright: DTU, BSD License
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <machine/patmos.h>
#include <machine/exceptions.h>
#include <machine/spm.h>
#include "ethlib/icmp.h"
#include "ethlib/icmp.h"
#include "ethlib/arp.h"
#include "ethlib/mac.h"
#include "ethlib/udp.h"
#include "ethlib/ipv4.h"
#include "ethlib/eth_mac_driver.h"
#include "ethlib/ptp1588.h"

#define DISP_SYM_MASK 0x80
#define PTP_MASTER
// #define PTP_SLAVE

volatile _SPM int *uart_ptr = (volatile _SPM int *)	 0xF0080004;
volatile _SPM int *led_ptr  = (volatile _SPM int *)  0xF0090000;
volatile _SPM int *key_ptr = (volatile _SPM int *)	 0xF00A0000;

unsigned int rx_addr = 0x000;
unsigned int tx_addr = 0x800;

unsigned char multicastip[4] = {224, 0, 0, 255};

unsigned int initNanoseconds;
unsigned int initSeconds;

void print_general_info(){
	printf("\nGeneral info:\n");
	printf("\tMAC: %llx", get_mac_address());
	printf("\n\tIP: ");
	ipv4_print_my_ip();
	printf("\n");
	arp_table_print();
	printf("\n");
	return;
}


void printSegmentInt(unsigned base_addr, int number, int displayCount) {
	volatile _IODEV unsigned *disp_ptr = (volatile _IODEV unsigned *) base_addr;
	unsigned pos = 0;
	unsigned byte_mask = 0x0000000F;
	unsigned range = (number > 0) ? displayCount : displayCount-1;	//reserve one digit for '-' symbol
	unsigned value = abs(number);
	for(pos=0; pos < range; pos++) {
		*disp_ptr = (unsigned)((value & byte_mask) >> (pos*4));
		byte_mask = byte_mask << 4;
		disp_ptr += 1;
	}
	if (number < 0) {
		*disp_ptr = DISP_SYM_MASK | 0x3F;
	}
}

int checkForPacket(unsigned int expectedPacketType, const unsigned int timeout){
	unsigned short dst_port;
	unsigned short src_port;
	if(eth_mac_receive(rx_addr, timeout)){
		// printf("%d", mac_packet_type(rx_addr));
		switch (mac_packet_type(rx_addr)) {
		case ICMP:
			return (icmp_process_received(rx_addr, tx_addr)==0) ? -ICMP : ICMP;
		case UDP:
			dst_port = udp_get_destination_port(rx_addr);
			if((dst_port==PTP_EVENT_PORT) || (dst_port==PTP_GENERAL_PORT)){
				return PTP;
			} else {
				return UDP;
			}
		case ARP:
			return (arp_process_received(rx_addr, tx_addr)==0) ? -ARP : ARP;
		default:
			return UNSUPPORTED;
		}
	} else {
		return UNSUPPORTED;
	}
}

void ptp_master_loop(unsigned period){
	unsigned short int seqId = 0;
	unsigned long long start_time = 0;
	unsigned char src_ip[4];
	signed char ans = 0;
	int syncInterval = (int) log2((int)period*USEC_TO_SEC);
	printf("T_sync=%x\n", syncInterval);
	start_time = get_rtc_usecs();
	while(1){
		if(0xE == *key_ptr){
			*led_ptr = 0xE;
			RTC_TIME_NS = initNanoseconds;
			RTC_TIME_SEC = initSeconds;
			seqId = 0x0;
		} else if (0xD == *key_ptr){
			*led_ptr = 0xD;
			RTC_TIME_NS = 0;
			RTC_TIME_SEC = 0;
			return;
		} else if (get_rtc_usecs()-start_time >= period){
			printf("Seq# %u\n", seqId);
			//Send SYNQ
			*led_ptr = PTP_SYNC_MSGTYPE;
			//printf("%.3fus\n", elapsed_time);
			puts("tx_SYNC");
			ptpv2_issue_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, lastSlaveInfo.ip, seqId, PTP_SYNC_MSGTYPE, syncInterval);
			//Send FOLLOW_UP
			*led_ptr = PTP_FOLLOW_MSGTYPE;
			puts("tx_FOLLOW");
			ptpv2_issue_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, lastSlaveInfo.ip, seqId, PTP_FOLLOW_MSGTYPE, syncInterval);
			if(checkForPacket(2, PTP_REQ_TIMEOUT) == PTP){
				ipv4_get_source_ip(rx_addr, src_ip);
				if((ans = ptpv2_handle_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, src_ip)) == PTP_DLYREQ_MSGTYPE){
					*led_ptr = PTP_DLYREQ_MSGTYPE;
					ptpv2_issue_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, lastSlaveInfo.ip, rxPTPMsg.head.sequenceId, PTP_DLYRPLY_MSGTYPE, syncInterval);
					*led_ptr = PTP_DLYRPLY_MSGTYPE;
					seqId++;
				}
			}
			start_time = get_rtc_usecs();
		}
		printSegmentInt(0xF00B0000, get_rtc_secs(), 8);
	}
}

void ptp_slave_loop(unsigned period){
	unsigned short int seqId = 0;
	unsigned char src_ip[4];
	while(1){
		if(0xE == *key_ptr){
			*led_ptr = 0xE;
			RTC_TIME_NS = 0;
			RTC_TIME_SEC = 0;
			RTC_CORRECTION_OFFSET = 0;
		} else if (0xD == *key_ptr){
			*led_ptr = 0xD;
			RTC_TIME_NS = 0;
			RTC_TIME_SEC = 0;
			RTC_CORRECTION_OFFSET = 0;
			return;
		} else if(checkForPacket(2, PTP_SYNC_TIMEOUT) == PTP){
			ipv4_get_source_ip(rx_addr, src_ip);
			switch(ptpv2_handle_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, src_ip)){
			case PTP_SYNC_MSGTYPE:
				if((rxPTPMsg.head.flagField & FLAG_PTP_TWO_STEP_MASK) != FLAG_PTP_TWO_STEP_MASK){
					*led_ptr = PTP_SYNC_MSGTYPE;
					ptpv2_issue_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, lastMasterInfo.ip, rxPTPMsg.head.sequenceId, PTP_DLYREQ_MSGTYPE, ptpTimeRecord.syncInterval);
					*led_ptr = PTP_DLYREQ_MSGTYPE;
				}
				break;
			case PTP_FOLLOW_MSGTYPE:
				*led_ptr = PTP_FOLLOW_MSGTYPE;
				ptpv2_issue_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, lastMasterInfo.ip, rxPTPMsg.head.sequenceId, PTP_DLYREQ_MSGTYPE, ptpTimeRecord.syncInterval);
				*led_ptr = PTP_DLYREQ_MSGTYPE;
				break;
			case PTP_DLYRPLY_MSGTYPE:
				*led_ptr = PTP_DLYRPLY_MSGTYPE;
				printf("#%u\t%d\t%d\n", rxPTPMsg.head.sequenceId, ptpTimeRecord.offsetSeconds, ptpTimeRecord.offsetNanoseconds);
				break;
			case PTP_ANNOUNCE_MSGTYPE:
				*led_ptr = PTP_ANNOUNCE_MSGTYPE;
				break;
			default:
				*led_ptr = 0x100 | *led_ptr;
				break;
			}
		}
		printSegmentInt(0xF00B0000, get_rtc_secs(), 8);
	}
}

int main(int argc, char **argv){
	*led_ptr = 0x1FF;
	puts("\nHello, PTPlib Demo Started");

	//MAC controller settings
	eth_mac_initialize();

	//Keep initial time
	initNanoseconds = RTC_TIME_NS;
	initSeconds = RTC_TIME_SEC;

	//Test offset
	RTC_CORRECTION_OFFSET = 500000;
	if(RTC_CORRECTION_OFFSET == 0 || RTC_CORRECTION_OFFSET == initNanoseconds){
		puts("Error HW clock adjustment does not work");
		return -1;
	}
	while(RTC_CORRECTION_OFFSET != 0){continue;}
	RTC_TIME_NS = initNanoseconds;
	RTC_TIME_SEC = initSeconds;

	//Demo
	*led_ptr = 0x0;
	#ifdef PTP_MASTER
		ipv4_set_my_ip((unsigned char[4]){192, 168, 2, 254});
		arp_table_init();
		puts("Mode Master Running\n");
		print_general_info();
		thisPortInfo = ptpv2_intialize_local_port(my_mac, (unsigned char[4]){192, 168, 2, 50}, 1);
		int loop = 1;
		int sel = 500;
		while(loop){
			puts("Enter a sync interval period in us (should be a power of two). Negative numbers terminate the program.");
			scanf("%d", &sel);
			if(sel > 0){
				ptp_master_loop(sel);
			} else {
				loop = 0;
			}
		}
		puts("\n");
		puts("Exiting!");
	#endif
	#ifdef PTP_SLAVE
		srand((unsigned) get_cpu_usecs());
		unsigned char rand_addr = rand()%253;
		ipv4_set_my_ip((unsigned char[4]){192, 168, 2, rand_addr});
		arp_table_init();
		puts("Mode Slave Running\n");
		print_general_info();
		thisPortInfo = ptpv2_intialize_local_port(my_mac, (unsigned char[4]){192, 168, 2, rand_addr}, rand_addr);
		ptp_slave_loop(0);
		puts("\n");
		puts("Exiting!");
	#endif

	return 0;
}
