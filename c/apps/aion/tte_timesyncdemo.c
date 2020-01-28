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
#define MAX(a, b) (a > b) ? a : b
#define MIN(a, b) (a < b) ? a : b

// TTE Configuration
#define TTETIME_TO_NS		65536
#define TTE_MAX_TRANS_DELAY	135600			//ns from net_config (eclipse project)
#define TTE_INT_PERIOD		10000000		//ns 10 ms
#define TTE_CYC_PERIOD		200000000		//ns 200 ms
#define TTE_PRECISION		1000000			//ns from network_description (eclipse project)

// TTE PID synchronization
#define TTE_SYNC_Kp 350LL
#define TTE_SYNC_Ki 850LL
#define TTE_SYNC_Kd 0LL

// Demo configuration
#define SYNC_WINDOW_HALF	10000		//ns
#define SYNC_PERIOD			5000000		//ns
#define PULSE_PERIOD 		20000000	//ns
#define CPU_PERIOD			12.5		//ns
#define NO_RX_PACKETS 		3000
#define HW_TIMESTAMPING

//IO
volatile _SPM int *uart_ptr = (volatile _SPM int *)	 PATMOS_IO_UART;
volatile _SPM int *led_ptr  = (volatile _SPM int *)  PATMOS_IO_LED;
volatile _SPM int *key_ptr = (volatile _SPM int *)	 PATMOS_IO_KEYS;
volatile _SPM int *gpio_ptr = (volatile _SPM int *)	 PATMOS_IO_GPIO;
volatile _SPM int *dead_ptr = (volatile _SPM int *)	 PATMOS_IO_DEADLINE;
volatile _SPM unsigned *disp_ptr = (volatile _SPM unsigned *) PATMOS_IO_SEGDISP;

//Variables
const char* eth_protocol_names[] = {"UFF", "IP", "ICMP", "UDP", "TCP", "PTP", "ARP", "LLDP", "MDNS", "TTE_PCF", "TTE"};
const unsigned int rx_addr = 0x000;
const unsigned int tx_addr = 0x800;
const unsigned char multicastip[4] = {224, 0, 0, 255};
const unsigned char TTE_CT[] = { 0xAB, 0xAD, 0xBA, 0xBE };

PTPPortInfo thisPtpPortInfo;
PTPv2Time softTimestamp;
PTPv2Time hardTimestamp;

static unsigned long long target_time = 0;
static long long clkDiff = 0;
static long long clkDiffLast = 0;
static long long clkDiffSum = 0;
static unsigned int rxPcfCount = 0;
static unsigned int inScheduleReceptions = 0;
static unsigned char nodeIsSynced = 0;
static unsigned char firstPCF = 1;
static unsigned short integration_cycle = 0;
unsigned long long initNanoseconds = 0;
unsigned long long initSeconds = 0;

#ifdef LOGGING
static PTPv2Time rxTimestampLog[NO_RX_PACKETS];
static unsigned short rxPacketLog[NO_RX_PACKETS];
static long long clkOffsetNsLog[NO_RX_PACKETS];
static long long timeFixNsLog[NO_RX_PACKETS];
static unsigned long long schedPITLog[NO_RX_PACKETS];
static unsigned long long permaPITLog[NO_RX_PACKETS];
static unsigned long long deltaTimeLog[NO_RX_PACKETS];
static unsigned long long tteTransClkLog[NO_RX_PACKETS];
static unsigned short tteIntCycleLog[NO_RX_PACKETS];
#endif
static unsigned long long syncLoopDeltaLog = 0;
static unsigned long long pulseDeltaLog = 0;

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
	do
	{
		printSegmentInt(*led_ptr);
	}while((*led_ptr=RTC_ADJUST_OFFSET(thisPtpPortInfo.eth_base)) != 0);
	printSegmentInt(RTC_ADJUST_OFFSET(thisPtpPortInfo.eth_base));
	RTC_TIME_NS(thisPtpPortInfo.eth_base) = initNanoseconds;
	RTC_TIME_SEC(thisPtpPortInfo.eth_base) = initSeconds;
}

unsigned long long elapsed_nanos(long long start, long long stop){
	if(stop - start < 0){
		return (unsigned long long) (stop - start + 1000000000); 
	} else {
		return (unsigned long long) (stop - start);
	}
}

__attribute__((noinline))
int tte_pcf_handle(unsigned long long sched_rec_pit, unsigned long long schedule_start){
	unsigned long long trans_clock;
	unsigned long long permanence_pit;

	trans_clock = mem_iord_byte(rx_addr + 34);
	trans_clock = (trans_clock << 8) | (mem_iord_byte(rx_addr + 35));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(rx_addr + 36));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(rx_addr + 37));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(rx_addr + 38));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(rx_addr + 38));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(rx_addr + 40));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(rx_addr + 41));
	trans_clock = trans_clock / TTETIME_TO_NS;

	integration_cycle = mem_iord_byte(rx_addr + 14);
	integration_cycle = (integration_cycle << 8) | (mem_iord_byte(rx_addr + 15));
	integration_cycle = (integration_cycle << 8) | (mem_iord_byte(rx_addr + 16));
	integration_cycle = (integration_cycle << 8) | (mem_iord_byte(rx_addr + 17));

	#ifdef HW_TIMESTAMPING
	permanence_pit = PTP_TIME_TO_NS(hardTimestamp.seconds, hardTimestamp.nanoseconds) - schedule_start + ((unsigned long long) TTE_MAX_TRANS_DELAY - trans_clock);
	#else
	permanence_pit = PTP_TIME_TO_NS(softTimestamp.seconds, softTimestamp.nanoseconds) - schedule_start + ((unsigned long long) TTE_MAX_TRANS_DELAY - trans_clock);
	#endif

	clkDiffLast = clkDiff;
	clkDiff = (long long) (permanence_pit - sched_rec_pit);
	clkDiffSum += clkDiff;

	//check scheduled receive window
	
	if(permanence_pit>(sched_rec_pit-TTE_PRECISION) && permanence_pit<(sched_rec_pit+TTE_PRECISION)){
		inScheduleReceptions++;
		*led_ptr |= 1U << 8;
	} else {
		*led_ptr &= 0U << 8;
	}

	#ifdef LOGGING
		tteTransClkLog[rxPcfCount] = trans_clock;
		tteIntCycleLog[rxPcfCount] = integration_cycle;
		permaPITLog[rxPcfCount] = permanence_pit;
		schedPITLog[rxPcfCount] = sched_rec_pit;
		clkOffsetNsLog[rxPcfCount] = clkDiff;
	#endif

	return 1;
}

__attribute__((noinline))
int receiveAndHandleFrame(unsigned long long timeout, unsigned long long current_time, unsigned long long schedule_start){
	unsigned short dest_port;
	//receive
	if(eth_mac_receive(rx_addr, (unsigned long long) timeout*NS_TO_USEC)){
		#ifdef HW_TIMESTAMPING
		hardTimestamp.nanoseconds = PTP_RXCHAN_TIMESTAMP_NS(thisPtpPortInfo.eth_base);
		hardTimestamp.seconds = PTP_RXCHAN_TIMESTAMP_SEC(thisPtpPortInfo.eth_base);
		PTP_RXCHAN_STATUS(thisPtpPortInfo.eth_base) = 0x1; //Clear PTP timestampAvail flag
		#else
		softTimestamp.nanoseconds = (unsigned) RTC_TIME_NS(thisPtpPortInfo.eth_base);
		softTimestamp.seconds = (unsigned) RTC_TIME_SEC(thisPtpPortInfo.eth_base);
		#endif
		//handle
		if (mem_iord_byte(rx_addr + 12) == 0x89 && mem_iord_byte(rx_addr + 13) == 0x1d){
			if((mem_iord_byte(rx_addr + 28)) == 0x2){
				tte_pcf_handle(current_time, schedule_start);
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

void syncPulse(){
	*gpio_ptr = 0x1;	//signal high time
	*dead_ptr = 80000;	//wait 1ms
	int val = *dead_ptr;                                            
	*gpio_ptr = 0x0;
}

void syncMoninor(){
	printSegmentInt(abs(clkDiff));
	*led_ptr |= (integration_cycle & 0x0F) << 4;
	if(nodeIsSynced){
		*led_ptr |= 1U << 7;
	} else {
		*led_ptr &= 0U << 7;
	}
}

__attribute__((noinline))
unsigned char syncWindow(unsigned long long timeout, unsigned long long current_time, unsigned long long schedule_start){
	int packetType = receiveAndHandleFrame(timeout, current_time, schedule_start);
	*led_ptr |= packetType & 0x0F;
	nodeIsSynced = packetType > 0 && inScheduleReceptions > 0 && abs(clkDiff) < TTE_PRECISION;
	#ifdef LOGGING
	rxPacketLog[rxPcfCount] = packetType;
	#ifdef HW_TIMESTAMPING
	rxTimestampLog[rxPcfCount].nanoseconds = hardTimestamp.nanoseconds;
	rxTimestampLog[rxPcfCount].seconds = hardTimestamp.seconds;
	deltaTimeLog[rxPcfCount] = initNanoseconds >0 ? abs(initNanoseconds-hardTimestamp.nanoseconds) : 0;
	initNanoseconds = hardTimestamp.nanoseconds;
	#else
	rxTimestampLog[rxPcfCount].nanoseconds = softTimestamp.nanoseconds;
	rxTimestampLog[rxPcfCount].seconds = softTimestamp.seconds;
	deltaTimeLog[rxPcfCount] = initNanoseconds >0 ? abs(initNanoseconds-softTimestamp.nanoseconds) : 0;
	initNanoseconds = softTimestamp.nanoseconds;
	#endif
	#endif
	return packetType == TTE_PCF ? 1 : 0;
}

unsigned long long get_tte_aligned_time(unsigned long long current_time){
	long long clock_corr = (((TTE_SYNC_Kp*clkDiff)>>10) 
                        + ((TTE_SYNC_Ki*clkDiffSum)>>10) 
                        + ((TTE_SYNC_Kd*(clkDiff-clkDiffLast))>>10));
	#ifdef LOGGING
		timeFixNsLog[rxPcfCount-1] = clock_corr;
	#endif
	return (unsigned long long) ((long long) current_time + clock_corr);
}

void demoExecutiveLoop(){
	*gpio_ptr = 0x0;
	unsigned long long sync_activation = 321200;			//initial values generated by scheduler
	unsigned long long pulse_activation = 17577991;			//initial values generated by scheduler
	unsigned long long sync_last_time = 0;				//keep track of the last executed time
	unsigned long long pulse_last_time = 0;				//keep track of the last executed time
	unsigned long long start_time = get_ptp_nanos(thisPtpPortInfo.eth_base);
	#ifdef LOGGING
	_Pragma("loopbound min 1 max 1")
	while(rxPcfCount < NO_RX_PACKETS && *key_ptr != 0xE){
	#else
	_Pragma("loopbound min 1 max 1")
	while(*key_ptr != 0xE){
	#endif
		register unsigned long long schedule_time = get_tte_aligned_time(get_ptp_nanos(thisPtpPortInfo.eth_base) - start_time);
		if(schedule_time + SYNC_WINDOW_HALF >= sync_activation){
			// *gpio_ptr = 0x1;
			rxPcfCount += syncWindow(nodeIsSynced ? 2*SYNC_WINDOW_HALF : 0, schedule_time, start_time);
			syncMoninor();
			sync_activation += get_tte_aligned_time(SYNC_PERIOD);
			syncLoopDeltaLog = schedule_time - sync_last_time;
			sync_last_time = schedule_time;
			// *gpio_ptr = 0x0;
		} 
		else if(nodeIsSynced && schedule_time >= pulse_activation) {
			syncPulse();
			pulse_activation += get_tte_aligned_time(PULSE_PERIOD);
			pulseDeltaLog = pulse_last_time;
			pulse_last_time = schedule_time;
		}
	}
}


int main(int argc, char **argv){
	// Start
	*led_ptr = 0x1FF;
	puts("\nEthernet Timestamping Demo Started");
	
	//MAC controller settings
	set_mac_address(0x1D000400, 0x00000289);
	//MODER: PAD|HUGEN|CRCEN|DLYCRCEN|-|FULLD|EXDFREN|NOBCKOF|LOOPBCK|IFG|PRO|IAM|BRO|NOPRE|TXEN|RXEN
	eth_iowr(MODER, 0x0000A423);
	eth_iowr(INT_SOURCE, INT_SOURCE_RXB_BIT);
    eth_iowr(RX_BD_ADDR_BASE(eth_iord(TX_BD_NUM)), RX_BD_EMPTY_BIT | RX_BD_IRQEN_BIT | RX_BD_WRAP_BIT);
	ipv4_set_my_ip((unsigned char[4]) {192, 168, 2, 69});
	arp_table_init();
	print_general_info();
	thisPtpPortInfo = ptpv2_intialize_local_port(PATMOS_IO_ETH, PTP_SLAVE, my_mac, my_ip, 1, 0);
	
	// Test offset
	testRTCAdjustment();
	initNanoseconds = 0;
	initSeconds = 0;
	*led_ptr = 0x000;
	
	// Executive Loop 
	demoExecutiveLoop();

	//Report
	puts("------------------------------------------");
	puts("Clock Sync Quality Log:");
	#ifdef LOGGING
	puts("------------------------------------------");
	puts("Timestamp(ns);\tDelta Time(ns);\tEth Type;\tCycle#;\tTransClk(ns);\tPermaPIT(ns);\tSchedPIT(ns);\tOffset(ns);\tPIFix(ns);");
	for(int i=0;i<NO_RX_PACKETS;i++){
		printf("%10llu;\t%llu;\t%s;\t%04hx;\t%10llu;\t%10lld;\t%10lld;\t%10lld;\t%10lld;\n", 
																PTP_TIME_TO_NS(rxTimestampLog[i].seconds, rxTimestampLog[i].nanoseconds),
		 														deltaTimeLog[i], eth_protocol_names[rxPacketLog[i]], tteIntCycleLog[i], tteTransClkLog[i],
																permaPITLog[i], schedPITLog[i], clkOffsetNsLog[i], timeFixNsLog[i]);
	}
	#endif
	puts("---------------------------------------------------------------------------");
	printf("--Received No. Packets = %u / %u \n", rxPcfCount, NO_RX_PACKETS);
	printf("--In-Sched No. Packets = %u (loss = %.3f %%) \n", inScheduleReceptions, 100-((float)inScheduleReceptions/(float)rxPcfCount)*100);
	puts("---------------------------------------------------------------------------");
	puts("Task log:");
	printf("--Sync Activation dt= %llu (jitter = %d ns)\n", syncLoopDeltaLog, abs(SYNC_PERIOD - syncLoopDeltaLog));
	printf("--Pulse Activation dt = %llu (jitter = %d ns)\n", pulseDeltaLog, abs(PULSE_PERIOD - pulseDeltaLog));
	puts("------------------------------------------");
	puts("\nExiting!");
	return 0;
}