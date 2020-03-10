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

//IO
#define LEDS *((volatile _SPM unsigned int *) (PATMOS_IO_LED))
#define KEYS *((volatile _SPM unsigned int *) (PATMOS_IO_KEYS))
#define GPIO *((volatile _SPM unsigned int *) (PATMOS_IO_GPIO))
#define DEAD *((volatile _SPM unsigned int *) (PATMOS_IO_DEADLINE))
#define SEGDISP *((volatile _SPM unsigned int *) (PATMOS_IO_SEGDISP))

// TTE Configuration
#define TTETIME_TO_NS		65536
#define TTE_MAX_TRANS_DELAY	135600			//ns from net_config (eclipse project)
#define TTE_INT_PERIOD		10000000		//ns
#define TTE_CYC_PERIOD		1000000000		//ns
#define TTE_PRECISION		10000			//ns from network_description (eclipse project)

// TTE PID synchronization
#define TTE_SYNC_Kp 1000LL
#define TTE_SYNC_Ki 0LL

// Demo directives
#define HW_TIMESTAMPING
#define TIME_CORRECTION_EN
#define ALIGN_TO_REALTIME	//TODO: reset time on every new cluster

// Demo parameters
#define NO_RX_PACKETS 		2000		//packets
#define CPU_PERIOD			12.5		//ns	
#define SYNC_WINDOW_HALF	10000		//ns
#define SYNC_PERIOD			10000000	//ns
#define ASYNC2SYNC_THRES	10			//clusters
#define PULSE_PERIOD 		50000000	//ns
#define PULSE_WIDTH			1000000		//ns

#define SYNCTASK_GPIO_BIT	0
#define PULSETASK_GPIO_BIT	1

#define SYNC_START_TIME		321200
#define PULSE_START_TIME	17924225

//Constants
const char* eth_protocol_names[] = {"UFF", "IP", "ICMP", "UDP", "TCP", "PTP", "ARP", "LLDP", "MDNS", "TTE_PCF", "TTE"};
const unsigned int rx_addr = 0x000;
const unsigned int tx_addr = 0x800;
const unsigned char multicastip[4] = {224, 0, 0, 255};
const unsigned char TTE_CT[] = { 0xAB, 0xAD, 0xBA, 0xBE };

//Hardware timestamping provided through ptp1588assist unit
PTPPortInfo thisPtpPortInfo;
PTPv2Time softTimestamp;
PTPv2Time hardTimestamp;

//Clock related variables
static long long clkDiff = 0;
static long long clkDiffLast = 0;
static long long clkDiffSum = 0;
static unsigned short integration_cycle = 0;
static unsigned long long coldStartIntegrationTime = 0;
static unsigned long long initNanoseconds = 0;
static unsigned long long initSeconds = 0;

//Counters
static unsigned short rxPcfCount = 0;
static unsigned short pulseCount = 0;
static unsigned int stableCycles = 0;
static unsigned int unstableCycles = 0;
static unsigned int stableClusters = 0;
static unsigned int unstableClusters = 0;
static unsigned int totalScheduledReceptions = 0;

//Flags
static unsigned char nodeIntegrated = 0;	//is used to indicate when the node has achieved sufficient syncrhonization
static unsigned char nodeSyncStable = 0;	//is used to enable task execution when the node is in a stable sync
static unsigned char nodeColdStart = 1;		//is used to indicate that a node has just booted and has not received a single PCF
static unsigned char nodeFirstSync = 1;

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
static unsigned long long syncLoopDeltaSum;
static unsigned long long pulseLoopDeltaSum;

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
    *(&SEGDISP+0) = number & 0xF;
    *(&SEGDISP+1) = (number >> 4) & 0xF;
    *(&SEGDISP+2) = (number >> 8) & 0xF;
    *(&SEGDISP+3) = (number >> 12) & 0xF;
    *(&SEGDISP+4) = (number >> 16) & 0xF;
    *(&SEGDISP+5) = (number >> 20) & 0xF;
    *(&SEGDISP+6) = (number >> 24) & 0xF;
    *(&SEGDISP+7) = (number >> 28) & 0xF;
}

void testRTCAdjustment(){
	initNanoseconds = RTC_TIME_NS(thisPtpPortInfo.eth_base);
	initSeconds = RTC_TIME_SEC(thisPtpPortInfo.eth_base);
	RTC_ADJUST_OFFSET(thisPtpPortInfo.eth_base) = 0x1000;
	do
	{
		printSegmentInt(LEDS);
	}while((LEDS=RTC_ADJUST_OFFSET(thisPtpPortInfo.eth_base)) != 0);
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

unsigned long long get_tte_aligned_time(unsigned long long current_time){
	long long clock_corr = (((TTE_SYNC_Kp*clkDiff)>>10) 
                        + ((TTE_SYNC_Ki*clkDiffSum)>>10));
	#ifdef LOGGING
		timeFixNsLog[rxPcfCount-1] = clock_corr;
	#endif
	#ifdef TIME_CORRECTION_EN
	return (unsigned long long) ((long long) current_time + clock_corr);
	#else
	return (unsigned long long) current_time;
	#endif
}

__attribute__((noinline))
int tte_pcf_handle(unsigned long long sched_rec_pit, unsigned long long* schedule_start){
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
	permanence_pit = PTP_TIME_TO_NS(hardTimestamp.seconds, hardTimestamp.nanoseconds) - *schedule_start + ((unsigned long long) TTE_MAX_TRANS_DELAY - trans_clock);
	#else
	permanence_pit = PTP_TIME_TO_NS(softTimestamp.seconds, softTimestamp.nanoseconds) - *schedule_start + ((unsigned long long) TTE_MAX_TRANS_DELAY - trans_clock);
	#endif


#ifndef ALIGN_TO_REALTIME
	if(integration_cycle == 0 && nodeIntegrated){
		*schedule_start = sched_rec_pit;
	}
#else
	if((nodeColdStart && integration_cycle==0) || !nodeColdStart){
#endif

	nodeColdStart = 0;

	clkDiff = (long long) (permanence_pit - sched_rec_pit);
	clkDiffSum += clkDiff;
	
	//check scheduled receive window
	if(permanence_pit>(sched_rec_pit-TTE_PRECISION) && permanence_pit<(sched_rec_pit+TTE_PRECISION)){
		stableCycles++;
		if(integration_cycle == 0){
			stableClusters++;
		}
	} else {
		unstableCycles++;
		if(integration_cycle == 0){
			unstableClusters++;
		}
	}
#ifdef ALIGN_TO_REALTIME
	}
#endif

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
void task_sync(unsigned long long* start_time, unsigned long long schedule_time, unsigned long long* activation, unsigned long long *nxt_task_activation, unsigned long long* last_time){
	GPIO |= (1U << SYNCTASK_GPIO_BIT);
	int ethFrameType;
	PTP_RXCHAN_STATUS(thisPtpPortInfo.eth_base) = 0x1; //Clear PTP timestampAvail flag
	unsigned long long listen_start = get_cpu_usecs(); //keep track when we started listening
	#pragma loopbound min 1 max 1
	do
	{
		if(eth_mac_receive_nb(rx_addr))
		{
			if((unsigned short) ((mem_iord_byte(rx_addr + 12) << 8) + (mem_iord_byte(rx_addr + 13))) == 0x891D)
			{
				#ifdef HW_TIMESTAMPING
				hardTimestamp.nanoseconds = PTP_RXCHAN_TIMESTAMP_NS(thisPtpPortInfo.eth_base);
				hardTimestamp.seconds = PTP_RXCHAN_TIMESTAMP_SEC(thisPtpPortInfo.eth_base);
				#else
				softTimestamp.nanoseconds = (unsigned) RTC_TIME_NS(thisPtpPortInfo.eth_base);
				softTimestamp.seconds = (unsigned) RTC_TIME_SEC(thisPtpPortInfo.eth_base);
				#endif
				ethFrameType = TTE_PCF;
			}
			PTP_RXCHAN_STATUS(thisPtpPortInfo.eth_base) = 0x1; //Clear PTP timestampAvail flag
		}
	} while(ethFrameType != TTE_PCF && get_cpu_usecs() - listen_start < 2*SYNC_WINDOW_HALF*NS_TO_USEC);
	if(ethFrameType == TTE_PCF){
		tte_pcf_handle(schedule_time, start_time);
		nodeIntegrated = !nodeColdStart && ethFrameType > 0 && (stableCycles - unstableCycles) > 0 && abs(clkDiff) < TTE_PRECISION;
		if((nodeFirstSync && integration_cycle == 0 && nodeIntegrated) || !nodeFirstSync){
			nodeSyncStable = nodeIntegrated && (stableClusters - unstableClusters) > ASYNC2SYNC_THRES;	
			nodeFirstSync = 0;
		}
	} else if(ethFrameType == TIMEOUT){
		stableCycles = 0;
		unstableCycles = 0;
		nodeFirstSync = 1;
		nodeColdStart = 1;
		nodeIntegrated = 0;
		nodeSyncStable = 0;
		clkDiffLast = 0;
		clkDiffSum = 0;
		clkDiff = 0;
	}
	#ifdef LOGGING
	rxPacketLog[rxPcfCount] = ethFrameType;
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
	*activation = *activation + get_tte_aligned_time(SYNC_PERIOD);
	*nxt_task_activation = get_tte_aligned_time(*nxt_task_activation);
	syncLoopDeltaSum += schedule_time - get_tte_aligned_time(*last_time);
	rxPcfCount += ethFrameType == TTE_PCF ? 1 : 0;
	*last_time = schedule_time;
	printSegmentInt(abs(clkDiff));
	LEDS = (nodeSyncStable << 7) + (nodeIntegrated << 6) + (ethFrameType & 0xF);
	GPIO &= (0U << SYNCTASK_GPIO_BIT);
}

__attribute__((noinline))
void task_pulse(unsigned long long schedule_time, unsigned long long* activation, unsigned long long* last_time){
	GPIO |= (1U << PULSETASK_GPIO_BIT);
	LEDS |= (1U << 8);
	DEAD = (int) (PULSE_WIDTH/12.5);	//wait 1ms
	int val = DEAD;
	LEDS &= (0U << 8);
	*activation = *activation + PULSE_PERIOD;
	pulseLoopDeltaSum += schedule_time - get_tte_aligned_time(*last_time);
	*last_time = schedule_time;
	pulseCount++;
	GPIO &= (0U << PULSETASK_GPIO_BIT);
}

void cyclicExecutiveLoop(){
	unsigned long long sync_activation = SYNC_START_TIME;			//initial values generated by scheduler
	unsigned long long pulse_activation = PULSE_START_TIME;			//initial values generated by scheduler
	unsigned long long sync_last_time = 0;							//keep track of the last executed time
	unsigned long long pulse_last_time = 0;							//keep track of the last executed time
	unsigned long long start_time = get_ptp_nanos(thisPtpPortInfo.eth_base);
	#ifdef LOGGING
	_Pragma("loopbound min 1 max 1")
	while(rxPcfCount < NO_RX_PACKETS && KEYS != 0xE){
	#else
	_Pragma("loopbound min 1 max 1")
	while(KEYS != 0xE){
	#endif
		register unsigned long long schedule_time = get_tte_aligned_time(get_ptp_nanos(thisPtpPortInfo.eth_base) - start_time);
		if(schedule_time >= sync_activation){
			task_sync(&start_time, schedule_time, &sync_activation, &pulse_activation, &sync_last_time);
		} 
		else if(KEYS != 0xD && schedule_time >= pulse_activation) {
			task_pulse(schedule_time, &pulse_activation, &pulse_last_time);
		}
	}
}

int main(int argc, char **argv){
	// Start
	LEDS = 0x1FF;
	puts("\nTTEthernet Clock Sync Demo Started");
	
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
	LEDS = 0x000;
	
	//Init
	GPIO = 0x0;
	PTP_RXCHAN_STATUS(thisPtpPortInfo.eth_base) = 0x1; //Clear PTP timestampAvail flag

	// Executive Loop
	cyclicExecutiveLoop();

	//Report
	puts("------------------------------------------");
	puts("Clock Sync Quality Log:");
	#ifdef LOGGING
	puts("------------------------------------------");
	puts("Timestamp(ns);\tDelta Time(ns);\tEth Type;\tCycle#;\tTransClk(ns);\tPermaPIT(ns);\tSchedPIT(ns);\tOffset(ns);\tPIFix(ns);");
	for(int i=0;i<rxPcfCount;i++){
		printf("%10llu;\t%llu;\t%s;\t%04hx;\t%10llu;\t%10lld;\t%10lld;\t%10lld;\t%10lld;\n", 
																PTP_TIME_TO_NS(rxTimestampLog[i].seconds, rxTimestampLog[i].nanoseconds),
		 														deltaTimeLog[i], eth_protocol_names[rxPacketLog[i]], tteIntCycleLog[i], tteTransClkLog[i],
																permaPITLog[i], schedPITLog[i], clkOffsetNsLog[i], timeFixNsLog[i]);
	}
	#endif
	printf("--Avg. clock offset = %lld ns\n", clkDiffSum / rxPcfCount);
	puts("---------------------------------------------------------------------------");
	printf("--Received No. of Packets = %u\n", rxPcfCount);
	printf("--In-Sched No. of Packets = %u (loss = %.3f %%) \n", stableCycles, 100-((float)stableCycles/(float)rxPcfCount)*100);
	printf("--No-Sched No. of Packets = %u (loss = %.3f %%) \n", unstableCycles, 100-((float)unstableCycles/(float)rxPcfCount)*100);
	puts("---------------------------------------------------------------------------");
	puts("Task log:");
	unsigned long long avgSyncDelta = syncLoopDeltaSum/rxPcfCount;
	printf("--task_syn()   avg. dt = %llu\t(avg. jitter = %d ns) from a total of %d executions\n", avgSyncDelta, abs(SYNC_PERIOD - avgSyncDelta), rxPcfCount);
	unsigned long long avgPulseDelta = pulseLoopDeltaSum/pulseCount;
	printf("--task_pulse() avg. dt = %llu\t(avg. jitter = %d ns) from a total of %d executions\n", avgPulseDelta, abs(PULSE_PERIOD - avgPulseDelta), pulseCount);
	puts("------------------------------------------");
	puts("\nExiting!");
	return 0;
}