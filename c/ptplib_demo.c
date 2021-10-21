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
#include "ethlib/arp.h"
#include "ethlib/mac.h"
#include "ethlib/udp.h"
#include "ethlib/ipv4.h"
#include "ethlib/eth_mac_driver.h"
#include "ethlib/ptp1588.h"

volatile _SPM int *uart_ptr = (volatile _SPM int *)	 PATMOS_IO_UART;
volatile _SPM int *led_ptr  = (volatile _SPM int *)  PATMOS_IO_LED;
volatile _SPM int *key_ptr = (volatile _SPM int *)	 PATMOS_IO_KEYS;
volatile _SPM unsigned *disp_ptr = (volatile _SPM unsigned *) PATMOS_IO_SEGDISP;

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

void printSegmentInt(unsigned number) {
    *(disp_ptr+0) = number & 0xF;
    *(disp_ptr+1) = (number >> 4) & 0xF;
    *(disp_ptr+2) = (number >> 8) & 0xF;
    *(disp_ptr+3) = (number >> 12) & 0xF;
    *(disp_ptr+4) = (number >> 16) & 0xF;
    *(disp_ptr+5) = (number >> 20) & 0xF;
    *(disp_ptr+6) = (number >> 24) & 0xF;
    *(disp_ptr+7) = (number >> 28) & 0xF;
}

int checkForPacket(unsigned int expectedPacketType, const unsigned int timeout){
	if(eth_mac_receive(rx_addr, timeout)){
		// printf("%d", mac_packet_type(rx_addr));
		switch (mac_packet_type(rx_addr)) {
		case ICMP:
			return (icmp_process_received(rx_addr, tx_addr)==0) ? -ICMP : ICMP;
		case UDP:
			if((udp_get_destination_port(rx_addr)==PTP_EVENT_PORT) || (udp_get_destination_port(rx_addr)==PTP_GENERAL_PORT)){
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

void ptp_master_loop(){
	unsigned long long startTime, elapsedTime;
	unsigned short int seqId = 0;
	printf("T_sync=%d\n", thisPtpPortInfo.syncInterval);
	startTime = get_ptp_usecs(thisPtpPortInfo.eth_base);
	while(1){
		// elapsedTime = get_ptp_usecs(thisPtpPortInfo.eth_base) - startTime;
		if (0xD == *key_ptr){
			*led_ptr = 0xD;
			RTC_ADJUST_OFFSET(thisPtpPortInfo.eth_base) = 0;
			RTC_TIME_SEC(thisPtpPortInfo.eth_base) = 0;
			RTC_TIME_NS(thisPtpPortInfo.eth_base) = 0;
		} else if (get_ptp_usecs(thisPtpPortInfo.eth_base) - startTime >= thisPtpPortInfo.syncInterval){
			printf("Seq# %u\n", seqId);
			ptpv2_issue_msg(thisPtpPortInfo, tx_addr, rx_addr, PTP_BROADCAST_MAC, PTP_MULTICAST_IP, seqId, PTP_SYNC_MSGTYPE);
			*led_ptr = 0x1;
			ptpv2_issue_msg(thisPtpPortInfo, tx_addr, rx_addr, PTP_BROADCAST_MAC, PTP_MULTICAST_IP, seqId, PTP_FOLLOW_MSGTYPE);
			*led_ptr = 0x2;
			if(checkForPacket(2, PTP_REQ_TIMEOUT) == PTP){
				*led_ptr = 0x2 | 0x4;
				if(ptpv2_handle_msg(thisPtpPortInfo, tx_addr, rx_addr, PTP_BROADCAST_MAC) == PTP_DLYREQ_MSGTYPE){
					*led_ptr = 0x4;
					ptpv2_issue_msg(thisPtpPortInfo, tx_addr, rx_addr, PTP_BROADCAST_MAC, lastSlaveInfo.ip, rxPTPMsg.head.sequenceId, PTP_DLYRPLY_MSGTYPE);
					*led_ptr = 0x8;
					seqId++;
				}
			}
			startTime = get_ptp_usecs(thisPtpPortInfo.eth_base);
		} else {
			*led_ptr ^= 1 << 8; 
		}
		printSegmentInt(get_ptp_secs(thisPtpPortInfo.eth_base));
	}
}

void ptp_slave_loop(){
	unsigned short int seqId = 0;
	while(1){
		if (0xD == *key_ptr){
			*led_ptr = 0xD;
			RTC_ADJUST_OFFSET(thisPtpPortInfo.eth_base) = 0;
			RTC_TIME_SEC(thisPtpPortInfo.eth_base) = 0;
			RTC_TIME_NS(thisPtpPortInfo.eth_base) = 0;
		} else if(checkForPacket(2, PTP_SYNC_TIMEOUT) == PTP){
			switch(ptpv2_handle_msg(thisPtpPortInfo, tx_addr, rx_addr, PTP_BROADCAST_MAC)){
			case PTP_SYNC_MSGTYPE:
				if((rxPTPMsg.head.flagField & FLAG_PTP_TWO_STEP_MASK) != FLAG_PTP_TWO_STEP_MASK){
					*led_ptr = 0x1;
					ptpv2_issue_msg(thisPtpPortInfo, tx_addr, rx_addr, PTP_BROADCAST_MAC, lastMasterInfo.ip, rxPTPMsg.head.sequenceId, PTP_DLYREQ_MSGTYPE);
					*led_ptr = 0x4;
				}
				break;
			case PTP_FOLLOW_MSGTYPE:
				*led_ptr = 0x2;
				ptpv2_issue_msg(thisPtpPortInfo, tx_addr, rx_addr, PTP_BROADCAST_MAC, lastMasterInfo.ip, rxPTPMsg.head.sequenceId, PTP_DLYREQ_MSGTYPE);
				*led_ptr = 0x4;
				break;
			case PTP_DLYRPLY_MSGTYPE:
				*led_ptr = 0x8;
				printf("#%u\t%d\t%d\n", rxPTPMsg.head.sequenceId, ptpTimeRecord.offsetSeconds, ptpTimeRecord.offsetNanoseconds);
				break;
			case PTP_ANNOUNCE_MSGTYPE:
				*led_ptr = 0xF;
				break;
			default:
				*led_ptr = 0x100 | *led_ptr;
				break;
			}
		}
		printSegmentInt(get_ptp_secs(thisPtpPortInfo.eth_base));
		*led_ptr = 0x0;
	}
}

int main(int argc, char **argv){
	unsigned int runAsPTPMode = PTP_SLAVE;
	unsigned long long userPeriod = PTP_SYNC_PERIOD;

	*led_ptr = 0x1FF;
	puts("\nHello, PTPlib Demo Started");
	puts("Select PTP mode (PTP_MASTER=0, PTP_SLAVE=1):");
	scanf("%u", &runAsPTPMode);

	//MAC controller settings
	eth_mac_initialize();
	// set_tx_db_num(0x2);

	//Keep initial time
	initNanoseconds = RTC_TIME_NS(PATMOS_IO_ETH);
	initSeconds = RTC_TIME_SEC(PATMOS_IO_ETH);

	//Test offset
	RTC_ADJUST_OFFSET(PATMOS_IO_ETH) = 0x1000;
	while((*led_ptr=RTC_ADJUST_OFFSET(PATMOS_IO_ETH)) != 0){printSegmentInt(*led_ptr);}
	RTC_TIME_NS(PATMOS_IO_ETH) = initNanoseconds;
	RTC_TIME_SEC(PATMOS_IO_ETH) = initSeconds;

	// read sync period from stdin
	puts("Enter a sync interval period in us (should be a power of two). Negative numbers terminate the program.");
	scanf("%llu", &userPeriod);
	int syncInterval = (int) log2((int)userPeriod*USEC_TO_SEC);
	
	//Demo
	*led_ptr = 0x0;
	if(runAsPTPMode == PTP_MASTER){
		puts("Mode Master Running");
		ipv4_set_my_ip((unsigned char[4]){192, 168, 2, 253});
		arp_table_init();
		print_general_info();
		thisPtpPortInfo = ptpv2_intialize_local_port(PATMOS_IO_ETH, PTP_MASTER, my_mac, (unsigned char[4]){192, 168, 2, 50}, 1, syncInterval);
		int loop = 1;
		if(userPeriod > 0){
			ptp_master_loop();
		} else {
			loop = 0;
		}
	} else if(runAsPTPMode == PTP_SLAVE){
		puts("Mode Slave Running\n");
		srand((unsigned) get_cpu_usecs());
		unsigned char rand_addr = rand()%253;
		ipv4_set_my_ip((unsigned char[4]){192, 168, 2, rand_addr});
		arp_table_init();
		print_general_info();
		thisPtpPortInfo = ptpv2_intialize_local_port(PATMOS_IO_ETH, PTP_SLAVE, my_mac, (unsigned char[4]){192, 168, 2, rand_addr}, rand_addr, syncInterval);
		ptp_slave_loop();
	}

	puts("\n");
	puts("Exiting!");
	return 0;
}
