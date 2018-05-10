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
	Main function for ptplib (IEEE 1588v2 Precise-Time-Protocol) demo

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

#define PTP_SYNC_PERIOD 4000

// #define PTP_MASTER
#define PTP_SLAVE

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
	unsigned char ans;
	if(eth_mac_receive(rx_addr, timeout)){
		packet_type = mac_packet_type(rx_addr);
		destination_port = udp_get_destination_port(rx_addr);
		source_port = udp_get_source_port(rx_addr);
		ipv4_get_source_ip(rx_addr, source_ip);
		//TODO: replace with dynamic function call provided by the user
		switch (packet_type) {
		case 1:
			ans = icmp_process_received(rx_addr, tx_addr);
			if (ans == 0){
				printf("\n- Notes:\n");
				printf("  - ICMP packet not our IP or not a ping request, no actions performed.\n");
			}else{
				printf("\n- Notes:\n");
				printf("  - Ping to our IP, replied.\n");
			}
			return 0;
		case 2:
			if((destination_port==PTP_EVENT_PORT && source_port==PTP_EVENT_PORT) || (destination_port==PTP_GENERAL_PORT && source_port==PTP_GENERAL_PORT)){
			// if(destination_port==expectedUDPPort && source_port==expectedUDPPort){
				return ptpv2_handle_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, source_ip);
			} else {
				printf("FAIL: Unexpected UDP port: %d : %d\n", destination_port, source_port);
				return 0;
			}
		case 3:
			ans = arp_process_received(rx_addr, tx_addr);
			printf("\n- Notes:\n");
			if (ans == 0){
				printf("  - ARP request not to our IP, no actions performed.\n");
			}else if(ans == 1){
				printf("  - ARP request to our IP, replied.\n");
			}
			return 0;
		default:
			printf("WARN: Unhandled PacketType (%d)\n", packet_type);
			return 0;
		}
	} else {
		printf("FAIL: EthMacRX Timeout\n");
		return 0;
	}
}

void ptp_master_loop(int msgDelay){
	unsigned short int seqId = 0;
	unsigned int start_time = 0;
	unsigned int elapsed_time = 0;
	start_time = get_cpu_usecs();
	while(1){
		//Count the time passed
		elapsed_time = get_cpu_usecs()-start_time;
		if (elapsed_time >= msgDelay){
			printf("----Seq#%d----\n", seqId);
			start_time = get_cpu_usecs();
			do {
				//Send SYNQ
				*led_ptr = 0x1;
				ptpv2_issue_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, target_ip, seqId, PTP_SYNC_MSGTYPE, PTP_SYNC_CTRL, PTP_EVENT_PORT);

				//Send FOLLOW_UP
				*led_ptr = 0x2;
				ptpv2_issue_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, target_ip, seqId, PTP_FOLLOW_MSGTYPE, PTP_FOLLOW_CTRL, PTP_GENERAL_PORT);

				//WaitFor DELAY_REQ
				// printf("WaitFor DELAY_REQ\n");
			} while(!checkForPacket(2, PTP_EVENT_PORT, PTP_REQ_TIMEOUT));
			*led_ptr = 0x4;
			seqId++;
		} else {
			//TODO: Do something useful
			if(0xE == *key_ptr){
				*led_ptr = 0x8;
				RTC_TIME_NS = initNanoseconds;
				RTC_TIME_SEC = initSeconds;
				seqId = 0x0;
			}
		}
	}
}

void ptp_slave_loop(){
	unsigned short int seqId = 0;
	while(1){
		if(0xE == *key_ptr){
			*led_ptr = 0xE;
			RTC_TIME_NS = 0;
			RTC_TIME_SEC = 0;
		} else if(0xD == *key_ptr){
			*led_ptr = 0xD;
			RTC_CORRECTION_OFFSET = 0;
		} else {
			*led_ptr = checkForPacket(2, PTP_EVENT_PORT, PTP_RPLY_TIMEOUT);
			printf("$offset\t%d\n", RTC_CORRECTION_OFFSET);
		}
	}
}


int main(int argc, char **argv){
	*led_ptr = 0x7;

	puts("PTPlib Demo Started\n");
	puts("\n");

	//MAC controller settings
	eth_iowr(0x40, 0xEEF0DA42);
	eth_iowr(0x44, 0x000000FF);
	eth_iowr(0x00, 0x0000A423);

	//Keep initial time
	initNanoseconds = RTC_TIME_NS;
	initSeconds = RTC_TIME_SEC;

	//Test offset
	RTC_CORRECTION_OFFSET = 4000000;
	if(RTC_CORRECTION_OFFSET == 0 || RTC_CORRECTION_OFFSET == initNanoseconds){
		printf("Error HW clock adjustment does not work");
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