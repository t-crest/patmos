#include <machine/patmos.h>
#include <stdio.h>
#include <stdlib.h>
#include "ethlib/eth_mac_driver.h"
#include "ethlib/icmp.h"
#include "ethlib/arp.h"
#include "ethlib/mac.h"
#include "ethlib/udp.h"
#include "ethlib/ipv4.h"
#include "ethlib/tte.h"
#include "ethlib/ptp1588.h"

#define NO_RX_PACKETS 100

#define INT_PERIOD 100 //ms
#define CYC_PERIOD 200 //ms
#define TTE_MAX_TRANS_DELAY 135600 //ns from net_config (eclipse project)
#define TTE_COMP_DELAY 0
#define TTE_PRECISION 10000 //ns from network_description (eclipse project)

//IO
volatile _SPM int *uart_ptr = (volatile _SPM int *)	 PATMOS_IO_UART;
volatile _SPM int *led_ptr  = (volatile _SPM int *)  PATMOS_IO_LED;
volatile _SPM int *key_ptr = (volatile _SPM int *)	 PATMOS_IO_KEYS;
volatile _SPM unsigned *disp_ptr = (volatile _SPM unsigned *) PATMOS_IO_SEGDISP;

//Variables
const char* eth_protocol_names[] = {"UFF", "IP", "ICMP", "UDP", "TCP", "PTP", "ARP", "LLDP", "MDNS", "TTE_PCF", "TTE"};

unsigned int rx_addr = 0x000;
unsigned int tx_addr = 0x800;

const unsigned char multicastip[4] = {224, 0, 0, 255};
const unsigned char TTE_CT[] = { 0xAB, 0xAD, 0xBA, 0xBE };

unsigned int initNanoseconds;
unsigned int initSeconds;

unsigned long long sched_rec_pit;

PTPPortInfo thisPtpPortInfo;
PTPv2Time softTimestamp;
PTPv2Time hardTimestamp;

signed long long clkOffsetSum = 0.0;
unsigned short rxPacketLog[NO_RX_PACKETS];
int clkOffsetNsLog[NO_RX_PACKETS];
unsigned int deltaTimeLog[NO_RX_PACKETS];
unsigned int tteIntCycleLog[NO_RX_PACKETS];
unsigned long long tteTransClkLog[NO_RX_PACKETS];
int rxPacketCount;
unsigned short integration_cycle;

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

int tteintframe_process_received(unsigned current_time){
	unsigned long long permanence_pit;
	unsigned long long trans_clock;
	signed long long clkDiff;

	trans_clock = mem_iord_byte(rx_addr + 34);
	trans_clock = (trans_clock << 8) | (mem_iord_byte(rx_addr + 35));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(rx_addr + 36));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(rx_addr + 37));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(rx_addr + 38));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(rx_addr + 38));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(rx_addr + 40));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(rx_addr + 41));
	tteTransClkLog[rxPacketCount] = trans_clock = trans_clock / TTETIME_TO_NS;

	integration_cycle = mem_iord_byte(rx_addr + 14);
	integration_cycle = (integration_cycle << 8) | (mem_iord_byte(rx_addr + 15));
	integration_cycle = (integration_cycle << 8) | (mem_iord_byte(rx_addr + 16));
	integration_cycle = (integration_cycle << 8) | (mem_iord_byte(rx_addr + 17));
	tteIntCycleLog[rxPacketCount] = integration_cycle;

	permanence_pit = (int) softTimestamp.nanoseconds + ((int)TTE_MAX_TRANS_DELAY-(int)trans_clock); 

	// if(rxPacketCount == 0){
	// 	RTC_TIME_NS(thisPtpPortInfo.eth_base) = permanence_pit - (2*TTE_MAX_TRANS_DELAY + TTE_COMP_DELAY);;
	// }

	sched_rec_pit = (int) RTC_TIME_NS(thisPtpPortInfo.eth_base) + ((int)2*TTE_MAX_TRANS_DELAY + TTE_COMP_DELAY);

	clkOffsetNsLog[rxPacketCount] = clkDiff = (permanence_pit - sched_rec_pit);

	clkOffsetSum += clkDiff;

	if(integration_cycle==0){
		RTC_TIME_NS(thisPtpPortInfo.eth_base) = 0;
	} else {
		RTC_TIME_NS(thisPtpPortInfo.eth_base) += 0.3*clkDiff;
	}

	//check scheduled receive window
	if(permanence_pit>(sched_rec_pit-TTE_PRECISION) && permanence_pit<(sched_rec_pit+TTE_PRECISION)){
		
		*led_ptr |= 1U << 8;
	}
	else{
	    *led_ptr &= 0U << 8;
	}

	return 1;
}


int receiveAndHandleFrame(const unsigned int timeout, unsigned int current_time){
	unsigned short dest_port;
	//receive
	if(eth_mac_receive(rx_addr, timeout)){
		softTimestamp.nanoseconds = (unsigned) RTC_TIME_NS(thisPtpPortInfo.eth_base);
		softTimestamp.seconds = (unsigned) RTC_TIME_SEC(thisPtpPortInfo.eth_base);
		hardTimestamp.nanoseconds = PTP_RXCHAN_TIMESTAMP_NS(thisPtpPortInfo.eth_base);
		hardTimestamp.seconds = PTP_RXCHAN_TIMESTAMP_SEC(thisPtpPortInfo.eth_base);
		PTP_RXCHAN_STATUS(thisPtpPortInfo.eth_base) = 0x1; //Clear PTP timestampAvail flag
		//handle
		if (mem_iord_byte(rx_addr + 12) == 0x89 && mem_iord_byte(rx_addr + 13) == 0x1d){
			if((mem_iord_byte(rx_addr + 28)) == 0x2){
				tteintframe_process_received(current_time);
			}
			return TTE_PCF;
		} else if(mac_compare_mac(mac_addr_dest(rx_addr), (unsigned char*) &TTE_CT)) {
			return TTE_MSG;
		} else {
			return UNSUPPORTED;
		}
	} else {
		return -1;
	}
}


int demoLoop(unsigned int timeout, unsigned int current_time){
	int packetType = receiveAndHandleFrame(timeout, current_time);
	*led_ptr = packetType & 0xFF;
	rxPacketLog[rxPacketCount] = packetType;
	deltaTimeLog[rxPacketCount] = abs(initNanoseconds-hardTimestamp.nanoseconds);
	initNanoseconds = hardTimestamp.nanoseconds;
	if(packetType == TTE_PCF){
        return 1;
	} else if(packetType == PTP) {
		clkOffsetNsLog[rxPacketCount] = ptpTimeRecord.offsetNanoseconds;
		return 1;
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
	RTC_TIME_NS(thisPtpPortInfo.eth_base) = 0;
	RTC_TIME_SEC(thisPtpPortInfo.eth_base) = 0;
	// sched_rec_pit = (int) RTC_TIME_NS(thisPtpPortInfo.eth_base) + ((int)2*TTE_MAX_TRANS_DELAY + TTE_COMP_DELAY);
	unsigned int startTime = RTC_TIME_NS(thisPtpPortInfo.eth_base);
	// printf("first sched_rec_pit = %d", sched_rec_pit);
	while(rxPacketCount < NO_RX_PACKETS){
		if(RTC_TIME_NS(thisPtpPortInfo.eth_base) - startTime >= INT_PERIOD*MS_TO_NS-TTE_PRECISION){
			rxPacketCount+=demoLoop(0, RTC_TIME_NS(thisPtpPortInfo.eth_base));
			// sched_rec_pit = (int) RTC_TIME_NS(thisPtpPortInfo.eth_base) + ((int)2*TTE_MAX_TRANS_DELAY + TTE_COMP_DELAY);
			printSegmentInt(get_ptp_usecs(thisPtpPortInfo.eth_base));
		}
		if(*key_ptr == 0xE)	break;
	}

	//Report
	printf("Received No. Packets = %d\n", rxPacketCount);
	puts("------------------------------------------");
	puts("Clock Offset log:");
	puts("------------------------------------------");
	puts("Eth Type;\tCycle#;\tTransClk(ns);\tOffset(ns);\tDelta Time(ns);");
	for(int i=0;i<NO_RX_PACKETS;i++){
		printf("%s;\t%04x;\t%10llx;\t%10d;\t%10d\n", eth_protocol_names[rxPacketLog[i]], tteIntCycleLog[i], tteTransClkLog[i], clkOffsetNsLog[i], deltaTimeLog[i]);
	}

	puts("\nExiting!");
	return 0;
}

