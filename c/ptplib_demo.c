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

#define PTP_MASTER

volatile _SPM int *uart_ptr = (volatile _SPM int *)	 0xF0080004;
volatile _SPM int *led_ptr  = (volatile _SPM int *)  0xF0090000;
volatile _SPM int *key_ptr = (volatile _SPM int *)	 0xF00A0000;

unsigned int rx_addr = 0x000;
unsigned int tx_addr = 0x800;

unsigned char multicastip[4] = {224, 0, 0, 255};

#ifdef PTP_MASTER
unsigned char target_ip[4] = {192, 168, 2, 1};
#endif
#ifdef PTP_SLAVE
unsigned char target_ip[4] = {192, 168, 2, 50};
#endif

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

int checkForPacket(unsigned int expectedPacketType, unsigned int expectedUDPPort, const unsigned int timeout){
	unsigned char packet_type;
	unsigned short destination_port;
	unsigned short source_port;
	unsigned char source_ip[4];	
	signed char ans;
	if(eth_mac_receive(rx_addr, timeout)){
		packet_type = mac_packet_type(rx_addr);
		destination_port = udp_get_destination_port(rx_addr);
		source_port = udp_get_source_port(rx_addr);
		ipv4_get_source_ip(rx_addr, source_ip);
		//TODO: replace with dynamic function call provided by the user
		switch (packet_type) {
		case 1:
			ans = icmp_process_received(rx_addr, tx_addr);
			return (ans==0) ? -1 : 1;
		case 2:
			if((destination_port==PTP_EVENT_PORT && source_port==PTP_EVENT_PORT) || (destination_port==PTP_GENERAL_PORT && source_port==PTP_GENERAL_PORT)){
				return ptpv2_handle_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, source_ip);
			} else {
				return -2;	
			}
		case 3:
			ans = arp_process_received(rx_addr, tx_addr);
			return (ans==0) ? -3 : 3;
		default:
			return -4;
		}
	} else {
		return -5;
	}
}

void ptp_master_loop(int msgDelay){
	unsigned short int seqId = 0;
	unsigned int start_time = 0;
	unsigned int elapsed_time = 0;
	signed char ans = 0;
	//start_time = get_cpu_usecs();
	start_time = get_rtc_usecs();
	while(1){
		//Count the time passed
		//elapsed_time = get_cpu_usecs()-start_time;
		elapsed_time = get_rtc_usecs()-start_time;
		if(0xE == *key_ptr){
			*led_ptr = 0x8;
			RTC_TIME_NS = initNanoseconds;
			RTC_TIME_SEC = initSeconds;
			seqId = 0x0;
			//start_time = get_cpu_usecs();
			start_time = get_rtc_usecs();
		} else if (elapsed_time >= msgDelay){
			puts("----\n");
			do {
				//Send SYNQ
				*led_ptr = 0x0;
				//printf("%.3fus\n", elapsed_time);
				puts("i_MSG=0");
				ptpv2_issue_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, target_ip, seqId, PTP_SYNC_MSGTYPE, PTP_SYNC_CTRL, PTP_EVENT_PORT);

				//Send FOLLOW_UP
				*led_ptr = 0x8;
				puts("i_MSG=8");
				ptpv2_issue_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, target_ip, seqId, PTP_FOLLOW_MSGTYPE, PTP_FOLLOW_CTRL, PTP_GENERAL_PORT);

				//WaitFor DELAY_REQ
				ans = checkForPacket(2, PTP_EVENT_PORT, PTP_REQ_TIMEOUT);
				*led_ptr = ans;
			} while(ans <= 0);
			if(ans == PTP_DLYREQ_MSGTYPE){
				ptpv2_issue_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, lastSlaveInfo.ip, ptpMsg.head.sequenceId, PTP_DLYRPLY_MSGTYPE, PTP_DLYRPLY_CTRL, PTP_GENERAL_PORT);
				// puts("i_MSG=9");
				*led_ptr = 0x9;
			}
			seqId++;
			//start_time = get_cpu_usecs();
			start_time = get_rtc_usecs();
		}
	}
}

void ptp_slave_loop(){
	unsigned short int seqId = 0;
	short int ans = 0;
	while(1){
		if(0xE == *key_ptr){
			*led_ptr = 0xE;
			RTC_TIME_NS = 0;
			RTC_TIME_SEC = 0;
			RTC_CORRECTION_OFFSET = 0;
		} else {
			ans = checkForPacket(2, PTP_EVENT_PORT, 0);
			*led_ptr = ans;
			switch(ans){
				case PTP_SYNC_MSGTYPE:
					if((ptpMsg.head.flagField & FLAG_PTP_TWO_STEP_MASK) != FLAG_PTP_TWO_STEP_MASK){
						ptpv2_issue_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, lastMasterInfo.ip, ptpMsg.head.sequenceId, PTP_DLYREQ_MSGTYPE, PTP_DLYREQ_CTRL, PTP_EVENT_PORT);
						*led_ptr = 0x1;
						//puts("i_MSG=1");
					}
					break;
				case PTP_FOLLOW_MSGTYPE:
					ptpv2_issue_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, lastMasterInfo.ip, ptpMsg.head.sequenceId, PTP_DLYREQ_MSGTYPE, PTP_DLYREQ_CTRL, PTP_EVENT_PORT);
					//puts("i_MSG=1");
					*led_ptr = 0x1;
					break;
				case PTP_DLYRPLY_MSGTYPE:
					printf("#%u\t%d\t%d\n", ptpMsg.head.sequenceId, ptpTimeRecord.offsetSeconds, ptpTimeRecord.offsetNanoseconds);
					break;
				default:
					puts("FAIL: EthMacRX Timeout or Unhandled");
					break;
			}
		}
		*led_ptr = 0x0;
	}
}


int main(int argc, char **argv){
	*led_ptr = 0x7;

	puts("PTPlib Demo Started");

	//MAC controller settings
	eth_iowr(0x40, 0xEEF0DA42);
	eth_iowr(0x44, 0x000000FF);
	eth_iowr(0x00, 0x0000A423);

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
	#ifdef PTP_MASTER
		ipv4_set_my_ip((unsigned char[4]){192, 168, 2, 50});
		arp_table_init();
		puts("Mode Master Running\n");
		arp_resolve_ip(rx_addr, tx_addr, target_ip, 200000);
		print_general_info();
		*led_ptr = 0x0;
		ptp_master_loop(PTP_SYNC_PERIOD);
		puts("\n");
		puts("Exiting!");
	#endif
	#ifdef PTP_SLAVE
		ipv4_set_my_ip((unsigned char[4]){192, 168, 2, 1});
		arp_table_init();
		puts("Mode Slave Running\n");
		arp_resolve_ip(rx_addr, tx_addr, target_ip, 200000);
		print_general_info();
		*led_ptr = 0x0;
		ptp_slave_loop();
		puts("\n");
		puts("Exiting!");
	#endif

	return 0;
}
