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
#define CPU_PERIOD 12.5
#define DEAD_CALC(DELAY) (DELAY/CPU_PERIOD)

const char* eth_protocol_names[] = {"UFF", "IP", "ICMP", "UDP", "TCP", "PTP", "ARP", "LLDP", "MDNS", "TTE_PCF"};

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

int demoMasterLoop(unsigned int syncPeriod){
	unsigned char ans = 0;
	if(get_ptp_usecs(thisPtpPortInfo.eth_base) - startTime >= syncPeriod){
		startTime = get_ptp_usecs(thisPtpPortInfo.eth_base);
		ptpv2_issue_msg(thisPtpPortInfo, tx_addr, rx_addr, PTP_MULTICAST_MAC, PTP_MULTICAST_IP, ptpSeqId, PTP_SYNC_MSGTYPE); //30 us measured @ 80MHz
		*dead_ptr = DEAD_CALC(PTP_DELAY_FOLLOWUP-30000);  //wait otherwise is too fast for slave patmos
		unsigned char val = *dead_ptr;     
    	ptpv2_issue_msg(thisPtpPortInfo, tx_addr, rx_addr, PTP_MULTICAST_MAC, PTP_MULTICAST_IP, ptpSeqId, PTP_FOLLOW_MSGTYPE);
		if((rxPacketLog[rxPacketCount] = receiveAndHandleFrame(PTP_REQ_TIMEOUT)) == PTP){
			if((ptpv2_process_received(thisPtpPortInfo, tx_addr, rx_addr)) == PTP_DLYREQ_MSGTYPE){
				hardTimestamp.nanoseconds = ptpTimeRecord.t1Nanoseconds;
				hardTimestamp.seconds = ptpTimeRecord.t1Seconds;
				ptpSeqId = rxPTPMsg.head.sequenceId++;
        		ptpSeqLog[rxPacketCount] = ptpSeqId;
				ans = 1;
			}
		}
	}
	deltaTimeLog[rxPacketCount] = abs(initNanoseconds-hardTimestamp.nanoseconds);
	initNanoseconds = hardTimestamp.nanoseconds;
	return ans;
}

int demoSlaveLoop(){
	unsigned char ans = 0;
	if((rxPacketLog[rxPacketCount] = *led_ptr = receiveAndHandleFrame(PTP_SYNC_TIMEOUT)) == PTP){
		if((*led_ptr = (*led_ptr << 4) + ptpv2_process_received(thisPtpPortInfo, tx_addr, rx_addr)) == PTP_DLYRPLY_MSGTYPE){
			hardTimestamp.nanoseconds = ptpTimeRecord.t4Nanoseconds;
			hardTimestamp.seconds = ptpTimeRecord.t4Seconds;
			ptpSeqLog[rxPacketCount] = rxPTPMsg.head.sequenceId;
			clkOffsetNsLog[rxPacketCount] = ptpTimeRecord.offsetNanoseconds;
			ans = 1;
		}
	}
	deltaTimeLog[rxPacketCount] = abs(initNanoseconds-hardTimestamp.nanoseconds);
	initNanoseconds = hardTimestamp.nanoseconds;
	return ans;
}

int main(int argc, char **argv){
	unsigned int runAsPTPMode = PTP_SLAVE;
    int userPTPSyncInterval = -3;
	int ptpSyncPeriod = 0;
	*led_ptr = 0x1FF;
	puts("\nTSN-Patmos Time Synchronization Demo Started");
	puts("Select PTP mode (PTP_MASTER=0, PTP_SLAVE=1):");
	scanf("%u", &runAsPTPMode);
	puts("Enter a sync interval period in 2^n seconds:");
	scanf("%d", &userPTPSyncInterval);
	ptpSyncPeriod = SYNC_INTERVAL_OPTIONS[abs(userPTPSyncInterval)]*USEC_TO_NS;
	printf("PTP node assigned role of %s\n", runAsPTPMode == 0 ? "PTP_MASTER" : "PTP_SLAVE");
	printf("PTP sync interval (0x%02x) configured at %d ns", (unsigned char) userPTPSyncInterval, ptpSyncPeriod);

	//MAC controller settings
	eth_iowr(0x40, 0xEEF0DA42);
	eth_iowr(0x44, 0x000000FF);
	//MODER: PAD|HUGEN|CRCEN|DLYCRCEN|-|FULLD|EXDFREN|NOBCKOF|LOOPBCK|IFG|PRO|IAM|BRO|NOPRE|TXEN|RXEN
	eth_iowr(0x00, 0x0000A423);
	// eth_mac_initialize(); 
	ipv4_set_my_ip((unsigned char[4]) {192, 168, 2, 69*(runAsPTPMode+1)});
	arp_table_init();
	print_general_info();

    //Configure PTP
	thisPtpPortInfo = ptpv2_intialize_local_port(PATMOS_IO_ETH, runAsPTPMode, my_mac, my_ip, 1, userPTPSyncInterval);

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
	while(rxPacketCount < NO_RX_PACKETS){
		if(thisPtpPortInfo.portRole == PTP_MASTER){
			demoMasterLoop(SYNC_INTERVAL_OPTIONS[abs(thisPtpPortInfo.syncInterval)]);
		} else {
			rxPacketCount += demoSlaveLoop();
		}
		printSegmentInt(get_ptp_secs(thisPtpPortInfo.eth_base));
		if((*led_ptr = *key_ptr) == 0xD) {
			RTC_ADJUST_OFFSET(thisPtpPortInfo.eth_base) = 0;
			RTC_TIME_SEC(thisPtpPortInfo.eth_base) = 0;
			RTC_TIME_NS(thisPtpPortInfo.eth_base) = 0;
		}
		if((*led_ptr = *key_ptr) == 0xE)	break;
		// *led_ptr = rxPacketCount;
	}

    if(thisPtpPortInfo.portRole == PTP_SLAVE){
        //Report
        printf("Received No. Packets = %d\n", rxPacketCount);
        puts("------------------------------------------");
        puts("Clock Offset log:");
        puts("------------------------------------------");
        puts("Eth Type;\tCycle#;\tTransClk(ns);\tOffset(ns);\tDelta Time(ns);");
        for(int i=0;i<NO_RX_PACKETS;i++){
            printf("%s;\t%10d;\t%10llx;\t%10d;\t%10d\n", eth_protocol_names[rxPacketLog[i]], ptpSeqLog[i], tsnTransClkLog[i], clkOffsetNsLog[i], deltaTimeLog[i]);
        }
    }
	puts("\nDemo Exiting!");
	return 0;
}