#include <machine/patmos.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ethlib/eth_mac_driver.h"
#include "ethlib/icmp.h"
#include "ethlib/arp.h"
#include "ethlib/mac.h"
#include "ethlib/udp.h"
#include "ethlib/ipv4.h"
#include "ethlib/ptp1588.h"

#define NO_RX_PACKETS 1000

#define TTE_PRECISION		10000 //ns from network_description (eclipse project)
#define TTE_MAX_TRANS_DELAY	135600 //ns from net_config (eclipse project)
#define TTE_INT_PERIOD		10000000 // 10 ms
#define TTE_CYC_PERIOD		20000000 // 20 ms
#define TTE_COMP_DELAY 0

#define TTETIME_TO_NS 65536

#define TTE_SYNC_Kp 250
#define TTE_SYNC_Ki 700
#define TTE_SYNC_Kd 0

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

uint32_t initNanoseconds = 0;
uint32_t initSeconds = 0;

PTPPortInfo thisPtpPortInfo;
PTPv2Time softTimestamp;
PTPv2Time hardTimestamp;

static uint64_t rxTimestampLog[NO_RX_PACKETS];
static uint16_t rxPacketLog[NO_RX_PACKETS];
static int64_t clkOffsetNsLog[NO_RX_PACKETS];
static uint32_t deltaTimeLog[NO_RX_PACKETS];
static uint32_t tteIntCycleLog[NO_RX_PACKETS];
static uint32_t tteTransClkLog[NO_RX_PACKETS];
static uint64_t sched_rec_pit = 0;
static int64_t clkDiff = 0;
static int64_t clkDiffLast = 0;
static int64_t clkDiffSum = 0;
static int32_t rxPacketCount = 0;
static int32_t inScheduleReceptions = 0;

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
	while((*led_ptr=RTC_ADJUST_OFFSET(thisPtpPortInfo.eth_base)) != 0)
  {
    printSegmentInt(*led_ptr);
  }
	RTC_TIME_NS(thisPtpPortInfo.eth_base) = initNanoseconds;
	RTC_TIME_SEC(thisPtpPortInfo.eth_base) = initSeconds;
}

__attribute__((noinline))
uint64_t get_tte_time(uint64_t current_time){
	uint64_t ans = current_time + (TTE_SYNC_Kp*clkDiff>>10) + (TTE_SYNC_Ki*clkDiffSum>>10) + ((TTE_SYNC_Kd*clkDiff-clkDiffLast)>>10);
	clkDiffLast = clkDiff;
	return ans;
}

__attribute__((noinline))
int tte_pcf_handle(uint64_t current_time){
	uint64_t trans_clock;
	uint16_t integration_cycle;
	uint64_t permanence_pit;

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

	permanence_pit = (SEC_TO_NS * hardTimestamp.seconds + hardTimestamp.nanoseconds) + (TTE_MAX_TRANS_DELAY-trans_clock); 

	sched_rec_pit = current_time + 2*TTE_MAX_TRANS_DELAY;

	clkOffsetNsLog[rxPacketCount] = clkDiff = (permanence_pit - sched_rec_pit);

	clkDiffSum += clkDiff;

	// if(clkDiff > SEC_TO_NS){
	// 	RTC_TIME_SEC(thisPtpPortInfo.eth_base) += 1;
	// 	RTC_TIME_NS(thisPtpPortInfo.eth_base) = RTC_TIME_NS(thisPtpPortInfo.eth_base) + clkDiff - SEC_TO_NS;
	// } else {
	// 	// RTC_TIME_NS(thisPtpPortInfo.eth_base) = RTC_TIME_NS(thisPtpPortInfo.eth_base) + clkDiff;
	// }

	// RTC_TIME_NS(thisPtpPortInfo.eth_base) = RTC_TIME_NS(thisPtpPortInfo.eth_base) + (300*clkDiff>>10) + (700*clkDiffSum>>10);

	// RTC_ADJUST_OFFSET(thisPtpPortInfo.eth_base) = (300*clkDiff>>10) + (700*clkDiffSum>>10);

	//check scheduled receive window
	if(permanence_pit>(sched_rec_pit-TTE_PRECISION) && permanence_pit<(sched_rec_pit+TTE_PRECISION)){
		*led_ptr |= 1U << 8;
		inScheduleReceptions++;
	}
	else{
	    *led_ptr &= 0U << 8;
	}

	return 1;
}

__attribute__((noinline))
int receiveAndHandleFrame(unsigned int timeout, unsigned long long current_time){
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
				tte_pcf_handle(current_time);
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

__attribute__((noinline))
uint8_t demoLoop(uint32_t timeout, uint32_t current_time){
	uint8_t ans = 0x0;
	int packetType = receiveAndHandleFrame(timeout, current_time);
	if(packetType == TTE_PCF)
  	{
		// sched_rec_pit = current_time + (TTE_INT_PERIOD + 2*TTE_MAX_TRANS_DELAY);
    	ans = 0x1;
	}
	*led_ptr = packetType & 0xFF;
	rxTimestampLog[rxPacketCount] = SEC_TO_NS * hardTimestamp.seconds + hardTimestamp.nanoseconds;
	rxPacketLog[rxPacketCount] = packetType;
	deltaTimeLog[rxPacketCount] = abs(initNanoseconds-hardTimestamp.nanoseconds);
	initNanoseconds = hardTimestamp.nanoseconds;
	return ans;
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
	uint64_t start_time = get_ptp_nanos(thisPtpPortInfo.eth_base);
	// sched_rec_pit = start_time + (TTE_INT_PERIOD + 2*TTE_MAX_TRANS_DELAY);
	while(rxPacketCount < NO_RX_PACKETS){
		uint64_t current_time = get_tte_time(get_ptp_nanos(thisPtpPortInfo.eth_base));
		if(current_time - start_time >= TTE_INT_PERIOD - TTE_PRECISION){
			rxPacketCount+=demoLoop(TTE_PRECISION, current_time);
			start_time = current_time; 
		}
		printSegmentInt(get_ptp_usecs(thisPtpPortInfo.eth_base));
		if(*key_ptr == 0xE)	break;
	}

	//Report
	puts("------------------------------------------");
	puts("Clock Offset log:");
	puts("------------------------------------------");
	puts("Timestamp;\tEth Type;\tCycle#;\tTransClk(ns);\tOffset(ns);\tDelta Time(ns);");
	puts("---------------------------------------------------------------------------");
	for(int i=0;i<NO_RX_PACKETS;i++){
		printf("%llu;\t%s;\t%04lx;\t%10lu;\t%10lld;\t%10ld\n", rxTimestampLog[i], eth_protocol_names[rxPacketLog[i]], tteIntCycleLog[i], tteTransClkLog[i], clkOffsetNsLog[i], deltaTimeLog[i]);
	}
	puts("---------------------------------------------------------------------------");
	printf("--Received No. Packets = %ld\n", rxPacketCount);
	printf("--In-Sched No. Packets = %ld\n", inScheduleReceptions);
	puts("---------------------------------------------------------------------------");
	puts("\nExiting!");
	return 0;
}

