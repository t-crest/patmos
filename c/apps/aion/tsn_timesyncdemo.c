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

#define WCET_SYNC_MASTER 74398 
#define WCET_SYNC_SLAVE 30077
#define NO_RX_PACKETS 5000
#define CPU_PERIOD 12.5
#define TIME_TO_CLKS(DELAY) (DELAY/CPU_PERIOD)

const char* eth_protocol_names[] = {"UFF", "IP", "ICMP", "UDP", "TCP", "PTP", "ARP", "LLDP", "MDNS", "TTE_PCF"};

const char ptp_slaves_iptable[3][4] = {{192, 168, 2, 3}, {192, 168, 2, 4}};

unsigned int ptp_slave_index = 0;

unsigned int rx_addr = 0x000;
unsigned int tx_addr = 0x800;

const unsigned char multicastip[4] = {224, 0, 0, 255};

volatile _SPM int *uart_ptr = (volatile _SPM int *)	 PATMOS_IO_UART;
volatile _SPM int *led_ptr  = (volatile _SPM int *)  PATMOS_IO_LED;
volatile _SPM int *key_ptr = (volatile _SPM int *)	 PATMOS_IO_KEYS;
volatile _IODEV int *dead_ptr = (volatile _IODEV int *) PATMOS_IO_DEADLINE;
volatile _SPM unsigned *disp_ptr = (volatile _SPM unsigned *) PATMOS_IO_SEGDISP;


//Variables
unsigned int initNanoseconds;
unsigned int initSeconds;

PTPPortInfo thisPtpPortInfo;
PTPv2Time softTimestamp;
PTPv2Time hardTimestamp;

signed long long clkOffsetSum = 0.0;
unsigned short rxPacketLog[NO_RX_PACKETS];
int clkOffsetNsLog[NO_RX_PACKETS];
unsigned int deltaTimeLog[NO_RX_PACKETS];
unsigned int ptpSeqLog[NO_RX_PACKETS];
unsigned long long tsnTransClkLog[NO_RX_PACKETS];
int rxPacketCount = 0;
unsigned int ptpSeqId = 0;
unsigned int startTime = 0;

void print_general_info(){
	printf("\nGeneral info:\n");
	printf("\tMAC: %llx", get_mac_address());
	printf("\n\tIP: ");
	ipv4_print_my_ip();
	printf("\n");
	arp_table_print();
	printf("\n");
	printf("PTP node assigned role of %s\n", thisPtpPortInfo.portRole == 0 ? "PTP_MASTER" : "PTP_SLAVE");
	printf("PTP sync interval (0x%02x) configured at %d ns", (unsigned char) thisPtpPortInfo.syncInterval, SYNC_INTERVAL_OPTIONS[abs(thisPtpPortInfo.syncInterval)]*USEC_TO_NS);
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

void waitFor(unsigned int clocks){
	*dead_ptr = clocks;  
	unsigned char val = *dead_ptr;    
}

int receiveAndHandleFrame(const unsigned int timeout){
	unsigned short dest_port;
	if(eth_mac_receive(rx_addr, timeout)){
        switch (mac_packet_type(rx_addr)) {
            case ICMP:
                return (icmp_process_received(rx_addr, tx_addr)==0) ? -ICMP : ICMP;
            case UDP:
                dest_port = udp_get_destination_port(rx_addr);
                if((dest_port==PTP_EVENT_PORT) || (dest_port==PTP_GENERAL_PORT)){
                    return PTP;
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
                return 0x0F;
        }
    } else {
		return -1;
	}
}

int masterSyncLoop(unsigned int syncPeriod){
	unsigned char ans = 0;
	if(get_ptp_usecs(thisPtpPortInfo.eth_base) - startTime >= syncPeriod){
		startTime = get_ptp_usecs(thisPtpPortInfo.eth_base);
		ptpv2_issue_msg(thisPtpPortInfo, tx_addr, rx_addr, PTP_MULTICAST_MAC, PTP_MULTICAST_IP, ptpSeqId, PTP_SYNC_MSGTYPE); //30 us measured @ 80MHz
		waitFor(WCET_SYNC_SLAVE); //wait otherwise is too fast for slave patmos
    	ptpv2_issue_msg(thisPtpPortInfo, tx_addr, rx_addr, PTP_MULTICAST_MAC, PTP_MULTICAST_IP, ptpSeqId, PTP_FOLLOW_MSGTYPE);
		if((rxPacketLog[rxPacketCount] = receiveAndHandleFrame(2*WCET_SYNC_SLAVE*CPU_PERIOD*NS_TO_USEC)) == PTP){
			if((ptpv2_process_received(thisPtpPortInfo, tx_addr, rx_addr)) == PTP_DLYREQ_MSGTYPE){
				hardTimestamp.nanoseconds = ptpTimeRecord.t1Nanoseconds;
				hardTimestamp.seconds = ptpTimeRecord.t1Seconds;
				ptpSeqLog[rxPacketCount] = ptpSeqId = rxPTPMsg.head.sequenceId++;
				ans = 1;
			}
			deltaTimeLog[rxPacketCount] = abs(initNanoseconds-hardTimestamp.nanoseconds);
			initNanoseconds = hardTimestamp.nanoseconds;
		}
	}
	return ans;
}

int slaveSyncLoop(){
	unsigned char ans = 0;
	if((rxPacketLog[rxPacketCount] = receiveAndHandleFrame(2*WCET_SYNC_MASTER*CPU_PERIOD*NS_TO_USEC)) == PTP){
		if(ptpv2_process_received(thisPtpPortInfo, tx_addr, rx_addr) == PTP_DLYRPLY_MSGTYPE){
			hardTimestamp.nanoseconds = ptpTimeRecord.t4Nanoseconds;
			hardTimestamp.seconds = ptpTimeRecord.t4Seconds;
			ptpSeqLog[rxPacketCount] = rxPTPMsg.head.sequenceId;
			clkOffsetNsLog[rxPacketCount] = ptpTimeRecord.offsetNanoseconds;
			ans = 1;
		}
		deltaTimeLog[rxPacketCount] = abs(initNanoseconds-hardTimestamp.nanoseconds);
		initNanoseconds = hardTimestamp.nanoseconds;
	}
	return ans;
}

int main(int argc, char **argv){
	unsigned int runAsPTPMode = PTP_SLAVE;
    int userPTPSyncInterval = -3;
	unsigned char userID;
	int ptpSyncPeriod = 0;
	*led_ptr = 0x1FF;
	puts("\nTSN-Patmos Time Synchronization Demo Started");
	puts("Enter your node ID ( > 0):");
	scanf("%hhu", &userID);
	puts("Select PTP mode (PTP_MASTER=0, PTP_SLAVE=1):");
	scanf("%u", &runAsPTPMode);
	puts("Enter a sync interval period in 2^n seconds:");
	scanf("%d", &userPTPSyncInterval);

    //Configure PTP
	thisPtpPortInfo = ptpv2_intialize_local_port(PATMOS_IO_ETH, runAsPTPMode, my_mac, my_ip, 1, userPTPSyncInterval);

	//MAC controller settings
	eth_mac_initialize(); 
	ipv4_set_my_ip((unsigned char[4]) {192, 168, 2, userID+1});
	arp_table_init();
	print_general_info();

	//Test offset
	testRTCAdjustment();
	initNanoseconds = 0;
	initSeconds = 0;

	*led_ptr = 0x100;
	puts("\nDemo starting...");

	//Demo
	RTC_TIME_NS(thisPtpPortInfo.eth_base) = 0;
	RTC_TIME_SEC(thisPtpPortInfo.eth_base) = 0;
	startTime = RTC_TIME_NS(thisPtpPortInfo.eth_base);
	while(*key_ptr != 0xE){
		while(*key_ptr != 0xD){
			if(thisPtpPortInfo.portRole == PTP_MASTER){
				masterSyncLoop(SYNC_INTERVAL_OPTIONS[abs(thisPtpPortInfo.syncInterval)]);
			} else {
				rxPacketCount = (rxPacketCount + slaveSyncLoop()) % NO_RX_PACKETS;
			}
			printSegmentInt(get_ptp_secs(thisPtpPortInfo.eth_base));
			if(*key_ptr == 0xD) {
				RTC_ADJUST_OFFSET(thisPtpPortInfo.eth_base) = 0;
				RTC_TIME_SEC(thisPtpPortInfo.eth_base) = 0;
				RTC_TIME_NS(thisPtpPortInfo.eth_base) = 0;
			}
			if(*key_ptr == 0xE)	break;
			*led_ptr = rxPacketCount;
		}

		if(thisPtpPortInfo.portRole == PTP_SLAVE){
			//Report
			printf("PTP Slave Received No. Packets = %d\n", rxPacketCount);
			puts("------------------------------------------");
			puts("Clock Offset log:");
			puts("------------------------------------------");
			puts("Eth Type;\tCycle#;\tTransClk(ns);\tOffset(ns);\tDelta Time(ns);");
			for(int i=0;i<NO_RX_PACKETS;i++){
				printf("%s;\t%10d;\t%10llx;\t%10d;\t%10d\n", eth_protocol_names[rxPacketLog[i]], ptpSeqLog[i], tsnTransClkLog[i], clkOffsetNsLog[i], deltaTimeLog[i]);
			}
		}
		rxPacketCount = 0;
	}

	puts("\nDemo Exiting!");
	return 0;
}