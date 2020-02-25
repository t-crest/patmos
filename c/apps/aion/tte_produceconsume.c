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
#define NO_TASKS			4
#define NO_RX_PACKETS 		2000		//packets
#define CPU_PERIOD			12.5		//ns	
#define SYNC_WINDOW_HALF	10000		//ns
#define SYNC_PERIOD			10000000	//ns
#define ASYNC2SYNC_THRES	10			//clusters
#define PULSE_PERIOD 		50000000	//ns
#define PULSE_WIDTH			1000000		//ns
#define SEND_PERIOD			4000000
#define RECV_PERIOD			4000000

#define SYNC_START_TIME		0
#define PULSE_START_TIME	295001
#define SEND_START_TIME     3324415
#define RECV_START_TIME     3367016

#define SYNCTASK_GPIO_BIT	0
#define PULSETASK_GPIO_BIT	1
#define SENDTASK_GPIO_BIT	2
#define RECVTASK_GPIO_BIT	3

#define PRODUCER 1
#define CONSUMER 2

//Types
typedef struct{
	unsigned long long val;
} SimpleTTMessage;

//Constants
const char* eth_protocol_names[] = {"UFF", "IP", "ICMP", "UDP", "TCP", "PTP", "ARP", "LLDP", "MDNS", "TTE_PCF", "TTE"};
const unsigned int rx_addr = 0x000;
const unsigned int tx_addr = 0x800;
const unsigned char multicastip[4] = {224, 0, 0, 255};
const unsigned char TTE_CT[] = { 0xAB, 0xAD, 0xBA, 0xBE };
const unsigned char TTE_VL[] = { 0x0F, 0xA1 };
const unsigned char TTE_MAC[] = {0x02, 0x89, 0x1D, 0x00, 0x04, 0x00};

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

//Logging
static unsigned long long syncLoopDeltaSum;
static unsigned long long pulseLoopDeltaSum;

//Communication
unsigned int txMsgCount = 0;
unsigned int rxMsgCount = 0;
SimpleTTMessage theMessage = {.val=0};


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
int waitAndReceiveFrame(unsigned long long timeout){
	unsigned short dest_port;
	unsigned char frame_dest_mac[6];
	//receive
	PTP_RXCHAN_STATUS(thisPtpPortInfo.eth_base) = 0x1; //Clear PTP timestampAvail flag
	if(eth_mac_receive(rx_addr, (unsigned long long) timeout * NS_TO_USEC)){
		mac_addr_dest(rx_addr, frame_dest_mac);
		#ifdef HW_TIMESTAMPING
		hardTimestamp.nanoseconds = PTP_RXCHAN_TIMESTAMP_NS(thisPtpPortInfo.eth_base);
		hardTimestamp.seconds = PTP_RXCHAN_TIMESTAMP_SEC(thisPtpPortInfo.eth_base);
		#else
		softTimestamp.nanoseconds = (unsigned) RTC_TIME_NS(thisPtpPortInfo.eth_base);
		softTimestamp.seconds = (unsigned) RTC_TIME_SEC(thisPtpPortInfo.eth_base);
		#endif
		PTP_RXCHAN_STATUS(thisPtpPortInfo.eth_base) = 0x1; //Clear PTP timestampAvail flag
		//handle
		if (mem_iord_byte(rx_addr + 12) == 0x89 && mem_iord_byte(rx_addr + 13) == 0x1d){
			if((mem_iord_byte(rx_addr + 28)) == 0x2){
				return TTE_PCF;		
			} else {
				return TTE_MSG;
			}
		} else if(frame_dest_mac[0] == TTE_CT[0] && frame_dest_mac[1] == TTE_CT[1] && frame_dest_mac[2] == TTE_CT[2] 
				&& frame_dest_mac[3] == TTE_CT[3] && frame_dest_mac[4] == TTE_VL[0] && frame_dest_mac[5] == TTE_VL[1]) {
			return TTE_MSG;
		} else {
			return UNSUPPORTED;
		}
	} else {
		return TIMEOUT;
	}
}

__attribute__((noinline))
void task_sync(unsigned long long* start_time, unsigned long long schedule_time, unsigned long long* activation, unsigned long long *nxt_activations[NO_TASKS], unsigned long long* last_time){
	GPIO |= (1U << SYNCTASK_GPIO_BIT);
	int ethFrameType = waitAndReceiveFrame(2*SYNC_WINDOW_HALF);
	if(ethFrameType == TTE_PCF){
		tte_pcf_handle(schedule_time, start_time);
		nodeIntegrated = !nodeColdStart && ethFrameType > 0 && (stableCycles - unstableCycles) > 0 && abs(clkDiff) < TTE_PRECISION;
		if((nodeFirstSync && integration_cycle == 0 && nodeIntegrated) || !nodeFirstSync){
			nodeSyncStable = nodeIntegrated && (stableClusters - unstableClusters) > ASYNC2SYNC_THRES;	
			nodeFirstSync = 0;
		}
	} else if(ethFrameType == TIMEOUT){
		nodeFirstSync = 1;
		nodeColdStart = 1;
		nodeIntegrated = 0;
		nodeSyncStable = 0;
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
	for(int i=1; i<NO_TASKS; i++){
		*nxt_activations[i] = get_tte_aligned_time(*nxt_activations[i]);
	}
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

__attribute__((noinline))
void task_send(SimpleTTMessage* message, int length, unsigned long long* activation){
	GPIO |= (1U << SENDTASK_GPIO_BIT);
	if(nodeSyncStable){
		unsigned char ethType[2];
		ethType[0]=((4+length)>>8) & 0xFF;
		ethType[1]=(4+length) & 0xFF;
		
		//Header
		#pragma loopbound min 4 max 4
		for(int i=0; i<4; i++){
			mem_iowr_byte(tx_addr + i, TTE_CT[i]);
		}
		mem_iowr_byte(tx_addr + 4, TTE_VL[0]);
		mem_iowr_byte(tx_addr + 5, TTE_VL[1]);

		#pragma loopbound min 6 max 6
		for(int i=6; i<12; i++){
			mem_iowr_byte(tx_addr + i, TTE_MAC[i-6]);
		}

		mem_iowr_byte(tx_addr + 12, ethType[0]);
		mem_iowr_byte(tx_addr + 13, ethType[1]);

		//Data
		#pragma loopbound min 4 max 8
		for(int i=0; i<length; i++){
			mem_iowr_byte(tx_addr + 18 + i, *((unsigned char*) message + i));
		}
		message->val += 1;
		txMsgCount++;
		eth_mac_send_nb(tx_addr, 18+4+length);
	}
	*activation += SEND_PERIOD;
	GPIO &= (0U << SENDTASK_GPIO_BIT);
}


__attribute__((noinline))
void task_recv(SimpleTTMessage* message, int length, unsigned long long* activation){
	GPIO |= (1U << RECVTASK_GPIO_BIT);
	if(nodeSyncStable){
		int ethFrameType = waitAndReceiveFrame(2*SYNC_WINDOW_HALF);
		LEDS &= 0xF0;
		LEDS |= (ethFrameType & 0xF);
		if(ethFrameType == TTE_MSG){
			//Data
			#pragma loopbound min 0 max 8
			for(int i=0; i<length; i++){
				*((unsigned char*)message + i) = mem_iord_byte(rx_addr + 18 + i);
			}
			printf("%llu,", message->val);
		} else if(ethFrameType == TIMEOUT){
			puts("x");
		}
	}
	*activation += RECV_PERIOD;
	GPIO &= (0U << RECVTASK_GPIO_BIT);
}


void cyclicExecutiveLoop(unsigned char exec_role){
	unsigned long long sync_activation = SYNC_START_TIME;			//initial values generated by scheduler
	unsigned long long pulse_activation = PULSE_START_TIME;			//initial values generated by scheduler
	unsigned long long send_activation = SEND_START_TIME;			//initial values generated by scheduler
	unsigned long long recv_activation = RECV_START_TIME;			//initial values generated by scheduler
	unsigned long long *activations[NO_TASKS] = {&sync_activation, &pulse_activation, &send_activation, &recv_activation};
	unsigned long long sync_last_time = 0;							//keep track of the last executed time
	unsigned long long pulse_last_time = 0;							//keep track of the last executed time
	SimpleTTMessage the_message = {.val = 0};
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
			task_sync(&start_time, schedule_time, &sync_activation, activations, &sync_last_time);
		} 
		else 
		if(schedule_time >= pulse_activation) {
			task_pulse(schedule_time, &pulse_activation, &pulse_last_time);
		}
		else
		if(exec_role==PRODUCER && schedule_time >= send_activation) {
            task_send(&the_message, 8, &send_activation);
        }
        else 
		if(exec_role==CONSUMER && schedule_time >= recv_activation) {
            task_recv(&the_message, 8, &recv_activation);
        }
	}
}

int main(int argc, char **argv){
	// Start
	unsigned char role;
	LEDS = 0x1FF;
	puts("\nTTEthernet Ping Demo Started");
	
	//MAC controller settings
	set_mac_address(0x1D000400, 0x00000289);
	eth_iowr(MODER, (RECSMALL_BIT | CRCEN_BIT | FULLD_BIT | PRO_BIT | TXEN_BIT | RXEN_BIT));
	eth_iowr(INT_SOURCE, INT_SOURCE_RXB_BIT);
    eth_iowr(RX_BD_ADDR_BASE(eth_iord(TX_BD_NUM)), RX_BD_EMPTY_BIT | RX_BD_IRQEN_BIT | RX_BD_WRAP_BIT);
	eth_iowr(INT_SOURCE, INT_SOURCE_TXB_BIT);
	eth_iowr(TX_BD_ADDR_BASE, TX_BD_READY_BIT | TX_BD_IRQEN_BIT | TX_BD_PAD_EN_BIT);
	thisPtpPortInfo = ptpv2_intialize_local_port(PATMOS_IO_ETH, PTP_SLAVE, my_mac, my_ip, 1, 0);
	
	// Test offset
	testRTCAdjustment();
	initNanoseconds = 0;
	initSeconds = 0;
	LEDS = 0x000;
	
	//Init
	GPIO = 0x0;
	printf("Select execution role (PRODUCER=%d, CONSUMER=%d): ", PRODUCER, CONSUMER);
	scanf("%02x", (unsigned int*) &role);
	PTP_RXCHAN_STATUS(thisPtpPortInfo.eth_base) = 0x1; //Clear PTP timestampAvail flag

	// Executive Loop
	cyclicExecutiveLoop(role);

	//Report
	puts("------------------------------------------");
	puts("Clock Sync Quality Log:");
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