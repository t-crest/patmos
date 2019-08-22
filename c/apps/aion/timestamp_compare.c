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

const char* eth_protocol_names[] = {"UFF", "IP", "ICMP", "UDP", "TCP", "PTP", "ARP", "LLDP", "MDNS", "TTE_PCF"};

unsigned int rx_addr = 0x000;
unsigned int tx_addr = 0x800;

const unsigned char multicastip[4] = {224, 0, 0, 255};

volatile _SPM int *uart_ptr = (volatile _SPM int *)	 PATMOS_IO_UART;
volatile _SPM int *led_ptr  = (volatile _SPM int *)  PATMOS_IO_LED;
volatile _SPM int *key_ptr = (volatile _SPM int *)	 PATMOS_IO_KEYS;
volatile _SPM unsigned *disp_ptr = (volatile _SPM unsigned *) PATMOS_IO_SEGDISP;


//Variables
unsigned int initNanoseconds;
unsigned int initSeconds;

PTPPortInfo thisPtpPortInfo;
PTPv2Time softTimestamp;
PTPv2Time hardTimestamp;

float avgJitterNs = 0.0;
int maxJitterNs = 0.0;
int jitterNsLog[NO_RX_PACKETS];
unsigned short rxPacketLog[NO_RX_PACKETS];
unsigned short rxSizeLog[NO_RX_PACKETS];
unsigned int ethPeriodLog[NO_RX_PACKETS];
int rxPacketCount;

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

void testRTCAdjustment(){
	initNanoseconds = RTC_TIME_NS(thisPtpPortInfo.eth_base);
	initSeconds = RTC_TIME_SEC(thisPtpPortInfo.eth_base);
	RTC_ADJUST_OFFSET(thisPtpPortInfo.eth_base) = 0x1000;
	while((*led_ptr=RTC_ADJUST_OFFSET(thisPtpPortInfo.eth_base)) != 0){printSegmentInt(*led_ptr);}
	RTC_TIME_NS(thisPtpPortInfo.eth_base) = initNanoseconds;
	RTC_TIME_SEC(thisPtpPortInfo.eth_base) = initSeconds;
}

int receiveAndHandleFrame(const unsigned int timeout){
	unsigned short dest_port;
	if(eth_mac_receive(rx_addr, timeout)){
		softTimestamp.nanoseconds = (unsigned) RTC_TIME_NS(thisPtpPortInfo.eth_base);
		softTimestamp.seconds = (unsigned) RTC_TIME_SEC(thisPtpPortInfo.eth_base);
		switch (mac_packet_type(rx_addr)) {
		case ICMP:
			return (icmp_process_received(rx_addr, tx_addr)==0) ? -ICMP : ICMP;
		case UDP:
			dest_port = udp_get_destination_port(rx_addr);
			if((dest_port==PTP_EVENT_PORT) || (dest_port==PTP_GENERAL_PORT)){
				return (ptpv2_process_received(thisPtpPortInfo, tx_addr, rx_addr)!=PTP_DLYRPLY_MSGTYPE) ? -PTP : PTP;
			} else if (dest_port==5353){
				return MDNS;
			} else {
				return UDP;
			}
		case TCP:
			return TCP;
		case ARP:
			return (arp_process_received(rx_addr, tx_addr)==0) ? -ARP : ARP;
		case TTE_PCF:
			return TTE_PCF;
		default:
			return UNSUPPORTED;
		}
	} else {
		return -1;
	}
}

int demoLoop(){
	int packetType = receiveAndHandleFrame(1000000);
	*led_ptr = packetType;
	if(packetType != -1){
		if(abs(packetType) == PTP){	//then use the already consumed hardware timestamp
			hardTimestamp.nanoseconds = ptpTimeRecord.t4Nanoseconds;
			hardTimestamp.seconds = ptpTimeRecord.t4Seconds;
		} else {
			hardTimestamp.nanoseconds = (unsigned) PTP_RXCHAN_TIMESTAMP_NS(thisPtpPortInfo.eth_base);
			hardTimestamp.seconds = (unsigned) PTP_RXCHAN_TIMESTAMP_SEC(thisPtpPortInfo.eth_base);
			PTP_RXCHAN_STATUS(thisPtpPortInfo.eth_base) = 0x1; //Clear PTP timestampAvail flag
		}
		int jitterNs = (hardTimestamp.nanoseconds-softTimestamp.nanoseconds);
		avgJitterNs = abs(avgJitterNs + jitterNs);
		if(jitterNs > maxJitterNs) maxJitterNs = jitterNs;
		jitterNsLog[rxPacketCount] = jitterNs;
		rxPacketLog[rxPacketCount] = packetType;
		rxSizeLog[rxPacketCount] = ipv4_get_length(rx_addr);
		ethPeriodLog[rxPacketCount] = abs(initNanoseconds-hardTimestamp.nanoseconds);
		initNanoseconds = hardTimestamp.nanoseconds;
		return jitterNs;
	} else {
		return 0;
	}
}

int main(int argc, char **argv){
	*led_ptr = 0x1FF;
	puts("\nEthernet Timestamping Demo Started");
	//MAC controller settings
	set_mac_address(0x1D000400, 0x00000289);
	//MODER: PAD|HUGEN|CRCEN|DLYCRCEN|-|FULLD|EXDFREN|NOBCKOF|LOOPBCK|IFG|PRO|IAM|BRO|NOPRE|TXEN|RXEN
	eth_iowr(0x00, 0x0000A423); 
	ipv4_set_my_ip((unsigned char[4]) {192, 168, 2, 69});
	arp_table_init();
	print_general_info();
	thisPtpPortInfo = ptpv2_intialize_local_port(PATMOS_IO_ETH, PTP_SLAVE, my_mac, my_ip, 1, 0);

	//Test offset
	testRTCAdjustment();
	initNanoseconds = 0;
	initSeconds = 0;

	*led_ptr = 0x100;

	//Demo
	for(rxPacketCount=0; rxPacketCount < NO_RX_PACKETS; rxPacketCount++){
		demoLoop();
		printSegmentInt(get_ptp_secs(thisPtpPortInfo.eth_base));
		if(*key_ptr == 0xE)	break;
	}

	//Report
	printf("Received No. Packets = %d\n", rxPacketCount);
	printf("Avg. timestamping jitter = %.3f (ns)\n", avgJitterNs / rxPacketCount);
	printf("Max. timestamping jitter = %d.000 (ns)\n", maxJitterNs);
	puts("Jitter log:");
	puts("------------------------------------------");
	puts("Eth Type;\tLength;\tJitter(ns);\tPeriod(ns);");
	for(int i=0;i<NO_RX_PACKETS;i++){
		printf("%s;\t%d;\t%d;\t%d\n", eth_protocol_names[rxPacketLog[i]], rxSizeLog[i], jitterNsLog[i], ethPeriodLog[i]);
	}

	puts("\nExiting!");
	return 0;
}
