/*
	Comparing hardware vs software timestamping capabilities
	Experiments with ping request/reply implemented in Patmos (80MHz) showed:
		Max ping request processing rate: ~900 us
		Demo cycle takes: 207.7625 us
		Checking for packet takes: ~157.4875 us
		Processing ping takes: ~70 us

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

#define NO_RX_PACKETS 100

volatile _SPM int *uart_ptr = (volatile _SPM int *)	 PATMOS_IO_UART;
volatile _SPM int *led_ptr  = (volatile _SPM int *)  PATMOS_IO_LED;
volatile _SPM int *key_ptr = (volatile _SPM int *)	 PATMOS_IO_KEYS;
volatile _SPM unsigned *disp_ptr = (volatile _SPM unsigned *) PATMOS_IO_SEGDISP;

unsigned int rx_addr = 0x000;
unsigned int tx_addr = 0x800;

unsigned char multicastip[4] = {224, 0, 0, 255};

unsigned int initNanoseconds;
unsigned int initSeconds;

PTPPortInfo thisPtpPortInfo;
PTPv2TimeRecord softTime;
PTPv2TimeRecord hardTime;

//Variables
float avgJitterNs = 0.0;
int maxJitterNs = 0.0;
int jitterNsLog[NO_RX_PACKETS];
int noRxPackets;

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

int checkForPacket(const unsigned int timeout){
	if(eth_mac_receive(rx_addr, timeout)){
		softTime.t1Nanoseconds = (unsigned) RTC_TIME_NS(thisPtpPortInfo.eth_base);
		softTime.t1Seconds = (unsigned) RTC_TIME_SEC(thisPtpPortInfo.eth_base);
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

int demoLoop(){
	if((*led_ptr = checkForPacket(0)) != UNSUPPORTED){
		// #pragma loopbound min 1 max 1
		// while(PTP_RXCHAN_STATUS(thisPtpPortInfo.eth_base) != 1){continue;}
		hardTime.t1Nanoseconds = (unsigned) PTP_RXCHAN_TIMESTAMP_NS(thisPtpPortInfo.eth_base);
		hardTime.t1Seconds = (unsigned) PTP_RXCHAN_TIMESTAMP_SEC(thisPtpPortInfo.eth_base);
		int jitterNs = abs(hardTime.t1Nanoseconds-softTime.t1Nanoseconds);
		avgJitterNs = (avgJitterNs + jitterNs);
		if(jitterNs > maxJitterNs) maxJitterNs = jitterNs;
		jitterNsLog[noRxPackets] = jitterNs;
		printSegmentInt(jitterNs);
		return jitterNs;
	} else {
		return 0;
	}
}

int main(int argc, char **argv){
	unsigned int runAsPTPMode = PTP_SLAVE;
	*led_ptr = 0x1FF;
	puts("\nEthernet Timestamping Demo Started");

	//MAC controller settings
	eth_mac_initialize();
	ipv4_set_my_ip((unsigned char[4]){192, 168, 2, 253});
	arp_table_init();
	print_general_info();
	thisPtpPortInfo = ptpv2_intialize_local_port(PATMOS_IO_ETH, my_mac, (unsigned char[4]){192, 168, 2, 253}, 1);

	//Keep initial time
	initNanoseconds = RTC_TIME_NS(PATMOS_IO_ETH);
	initSeconds = RTC_TIME_SEC(PATMOS_IO_ETH);

	//Test offset
	RTC_ADJUST_OFFSET(PATMOS_IO_ETH) = 0x1000;
	while((*led_ptr=RTC_ADJUST_OFFSET(PATMOS_IO_ETH)) != 0){printSegmentInt(*led_ptr);}
	RTC_TIME_NS(PATMOS_IO_ETH) = initNanoseconds;
	RTC_TIME_SEC(PATMOS_IO_ETH) = initSeconds;

	printSegmentInt(*led_ptr);

	//Demo
	for(noRxPackets=0; noRxPackets < NO_RX_PACKETS; noRxPackets++){
		*led_ptr ^= 1U << 8;
		demoLoop();
	}

	printf("Received No. Packets = %d\n", noRxPackets);
	printf("Avg. timestamping jitter = %.3f (ns)\n", avgJitterNs / noRxPackets);
	printf("Max. timestamping jitter = %d.000 (ns)\n", maxJitterNs);
	printf("Jitter log:\n");
	for(int i=0;i<NO_RX_PACKETS;i++){
		printf("%d\n", jitterNsLog[i]);
	}

	puts("\nExiting!");
	return 0;
}
