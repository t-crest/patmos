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

// Demo roles
#define PRODUCER 1
#define CONSUMER 2

#ifdef ROLE_PRODUCER
	#include "producer_schedule.h"
	#define ROLE PRODUCER
#else
	#include "consumer_schedule.h"
	#define ROLE CONSUMER
#endif

#define MAX(a, b) (a > b) ? a : b
#define MIN(a, b) (a < b) ? a : b


//IO
#define UART *((volatile _SPM unsigned int *) (PATMOS_IO_UART+0x4))
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

// GPIO Flags
#define SYNCTASK_GPIO_BIT	0
#define PULSETASK_GPIO_BIT	1
#define SENDTASK_GPIO_BIT	2
#define RECVTASK_GPIO_BIT	3
#define CALCTASK_GPIO_BIT	4

// Motor control
#define PULSE_PERIOD 		20000000	//ns
#define PULSE_WIDTH			1000000		//ns
#define PWM_PERIOD PULSE_PERIOD
#define MIN_CYCLE 0.015
#define MAX_CYCLE 0.1
#define MOTOR_STEP 0.0005

#define HIGH_TIME(DUTY_CYCLE) PWM_PERIOD*DUTY_CYCLE
#define LOW_TIME(DUTY_CYCLE) PWM_PERIOD-HIGH_TIME(DUTY_CYCLE)
#define DEAD_CALC(DUTY_CYCLE) (HIGH_TIME(DUTY_CYCLE)/CPU_PERIOD)

// Demo directives
#define HW_TIMESTAMPING
#define TIME_CORRECTION_EN
#define ALIGN_TO_REALTIME	//TODO: reset time on every new cluster

// Demo parameters
#define NUM_RX_PACKETS 		2000		//packets
#define CPU_PERIOD			12.5		//ns	
#define ASYNC2SYNC_THRES	10			//clusters
#define SYNC_WINDOW_HALF	10000		//ns
#define RECV_WINDOW_HALF	10000		//ns

//Types
typedef struct
{
	unsigned int seq;
	unsigned long long origin_ts;
	float val;
} SimpleTTEMessage;

typedef struct
{
	unsigned long long period;
    unsigned int release_inst;
    unsigned int nr_releases;
    unsigned long long *release_times;
	unsigned long long last_time;
	unsigned long long delta_sum;
	unsigned long exec_count;
} SimpleTTETask;

//Constants
const char* eth_protocol_names[] = {"UFF", "IP", "ICMP", "UDP", "TCP", "PTP", "ARP", "LLDP", "MDNS", "TTE_PCF", "TTE"};
const unsigned int rx_addr = 0x000;
const unsigned int tx_addr = 0x800;
const unsigned char multicastip[4] = {224, 0, 0, 255};
const unsigned char TTE_CT[] = { 0xAB, 0xAD, 0xBA, 0xBE };
const unsigned char TTE_PRODUCER_VL[] = { 0x0F, 0xA1 };
const unsigned char TTE_CONSUMER_VL[] = { 0x0F, 0xA2 };
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
static unsigned int stableCycles = 0;
static unsigned int unstableCycles = 0;
static unsigned int stableClusters = 0;
static unsigned int unstableClusters = 0;
static unsigned int totalScheduledReceptions = 0;
static unsigned int pulseDuration = DEAD_CALC(MAX_CYCLE);
static unsigned long long endToEndSum = 0ULL;

//Flags
static unsigned char nodeIntegrated = 0;	//is used to indicate when the node has achieved sufficient syncrhonization
static unsigned char nodeSyncStable = 0;	//is used to enable task execution when the node is in a stable sync
static unsigned char nodeColdStart = 1;		//is used to indicate that a node has just booted and has not received a single PCF
static unsigned char nodeFirstSync = 1;
static unsigned char nodeSendEnable = 0;
static unsigned char nodeRecvEnable = 0;

//Communication
unsigned int txMsgCount = 0;
unsigned int rxMsgCount = 0;

void print_general_info()
{
	printf("\nGeneral info:\n");
	printf("\tMAC: %llx", get_mac_address());
	printf("\n");
	return;
}

void printSegmentInt(unsigned number) 
{
    *(&SEGDISP+0) = number & 0xF;
    *(&SEGDISP+1) = (number >> 4) & 0xF;
    *(&SEGDISP+2) = (number >> 8) & 0xF;
    *(&SEGDISP+3) = (number >> 12) & 0xF;
    *(&SEGDISP+4) = (number >> 16) & 0xF;
    *(&SEGDISP+5) = (number >> 20) & 0xF;
    *(&SEGDISP+6) = (number >> 24) & 0xF;
    *(&SEGDISP+7) = (number >> 28) & 0xF;
}

void init_simple_simplettetask(SimpleTTETask *new_task, unsigned long long period, unsigned nr_releases, unsigned long long *release_times){
  new_task->period = period;
  new_task->last_time = 0;
  new_task->delta_sum = 0;
  new_task->exec_count = 0;
  new_task->release_inst = 0;
  new_task->nr_releases = nr_releases;
  new_task->release_times = release_times;
}


__attribute__((noinline))
unsigned long long get_tte_aligned_time(unsigned long long current_time, unsigned long long corr_limit)
{
	long long clock_corr = (((TTE_SYNC_Kp*clkDiff)>>10) + ((TTE_SYNC_Ki*clkDiffSum)>>10));
	#ifdef TIME_CORRECTION_EN
	return (unsigned long long) ((long long) current_time + clock_corr);
	#else
	return (unsigned long long) current_time;
	#endif
}

__attribute__((noinline))
int tte_pcf_handle(unsigned long long sched_rec_pit, unsigned long long schedule_start)
{
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

	if((nodeColdStart && integration_cycle==0) || !nodeColdStart)
	{
		nodeColdStart = 0;
		clkDiff = (long long) (permanence_pit - sched_rec_pit);
		clkDiffSum += clkDiff;
		//check scheduled receive window
		if(permanence_pit>(sched_rec_pit-TTE_PRECISION) && permanence_pit<(sched_rec_pit+TTE_PRECISION))
		{
			stableCycles++;
			if(integration_cycle == 0)
			{
				stableClusters++;
			}
		} else {
			unstableCycles++;
			if(integration_cycle == 0)
			{
				unstableClusters++;
			}
		}
	}
	return 1;
}

__attribute__((noinline))
void task_sync(unsigned long long start_time, unsigned long long schedule_time, SimpleTTETask* tasks)
{
	GPIO |= (1U << SYNCTASK_GPIO_BIT);
	int ethFrameType = TIMEOUT;
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
	//work
	if(ethFrameType == TTE_PCF)
	{
		tte_pcf_handle(schedule_time, start_time);
		nodeIntegrated = !nodeColdStart && ethFrameType > 0 && (stableCycles - unstableCycles) > 0 && abs(clkDiff) < TTE_PRECISION;
		if((nodeFirstSync && integration_cycle == 0 && nodeIntegrated) || !nodeFirstSync){
			nodeSyncStable = nodeIntegrated && (stableClusters - unstableClusters) > ASYNC2SYNC_THRES;	
			nodeFirstSync = 0;
		}
		rxPcfCount += 1;
	} 
	else if(ethFrameType == TIMEOUT)
	{
		nodeFirstSync = 1;
		nodeColdStart = 1;
		nodeIntegrated = 0;
		nodeSyncStable = 0;
		clkDiffLast = 0;
		clkDiffSum = 0;
		clkDiff = 0;
	}
	// if(nodeIntegrated){
		#pragma loopbound min 1 max 5
		for(int i=0; i<NUM_OF_TASKS; i++)
		{
			#pragma loopbound min 1 max 4
			for(int n=0; n<tasks[i].nr_releases; n++)
				tasks[i].release_times[n] = get_tte_aligned_time(tasks[i].release_times[n], tasks[i].period);
		}
	// }
	printSegmentInt(abs(clkDiff));
	LEDS = (nodeSyncStable << 7) + (nodeIntegrated << 6) + (ethFrameType & 0xF);
	GPIO &= (0U << SYNCTASK_GPIO_BIT);
}

unsigned int last_rx_seq = 0; 

__attribute__((noinline))
void task_calc(unsigned long long current_time, SimpleTTEMessage *outgoing_message, const SimpleTTEMessage *incoming_message)
{
	GPIO |= (1U << CALCTASK_GPIO_BIT);
	switch (ROLE)
	{
	case PRODUCER:
		pulseDuration = (int) DEAD_CALC(outgoing_message->val);
		outgoing_message->seq += 1;
		outgoing_message->val = (outgoing_message->val >= MAX_CYCLE) ? MIN_CYCLE : outgoing_message->val + MOTOR_STEP;  //increase dutyCycle for sweep;
		outgoing_message->origin_ts = get_tte_aligned_time((unsigned long long) get_ptp_nanos(thisPtpPortInfo.eth_base), HYPER_PERIOD);
		nodeSendEnable = nodeSyncStable && KEYS != 0xD;
		nodeRecvEnable = 0;
		break;
	case CONSUMER:
		if(incoming_message->seq > 0)
		{
			pulseDuration = (int) DEAD_CALC(incoming_message->val);
			endToEndSum += get_tte_aligned_time((unsigned long long) get_ptp_nanos(thisPtpPortInfo.eth_base), HYPER_PERIOD) - incoming_message->origin_ts;
			last_rx_seq = incoming_message->seq;
		}
		nodeSendEnable = 0;
		nodeRecvEnable = nodeSyncStable && KEYS != 0xD;
	}
	GPIO &= (0U << CALCTASK_GPIO_BIT);
}

__attribute__((noinline))
void task_pulse(unsigned long long current_time)
{
	GPIO |= (1U << PULSETASK_GPIO_BIT);
	LEDS |= (1U << 8);
	GPIO |= (1U << 5);
	DEAD = pulseDuration;	//wait pwm duration
	int val = DEAD;
	GPIO &= (0U << 5);
	LEDS &= (0U << 8);
	GPIO &= (0U << PULSETASK_GPIO_BIT);
}

__attribute__((noinline))
void task_send(unsigned long long current_time, SimpleTTEMessage* message, int length, const unsigned char VL[2]){
	GPIO |= (1U << SENDTASK_GPIO_BIT);
	if(nodeSendEnable)
	{
		unsigned char ethType[2];
		ethType[0]=((4+length)>>8) & 0xFF;
		ethType[1]=(4+length) & 0xFF;
		//Header
		#pragma loopbound min 4 max 4
		for(int i=0; i<4; i++)
		{
			mem_iowr_byte(tx_addr + i, TTE_CT[i]);
		}
		mem_iowr_byte(tx_addr + 4, VL[0]);
		mem_iowr_byte(tx_addr + 5, VL[1]);

		#pragma loopbound min 6 max 6
		for(int i=6; i<12; i++)
		{
			mem_iowr_byte(tx_addr + i, TTE_MAC[i-6]);
		}
		mem_iowr_byte(tx_addr + 12, ethType[0]);
		mem_iowr_byte(tx_addr + 13, ethType[1]);
		//Data
		#pragma loopbound min 16 max 16
		for(int i=0; i<length; i++)
		{
			mem_iowr_byte(tx_addr + 18 + i, *((unsigned char*) message + i));
		}
		eth_mac_send_nb(tx_addr, 18+4+length);
		txMsgCount++;
	}
	GPIO &= (0U << SENDTASK_GPIO_BIT);
}


__attribute__((noinline))
void task_recv(unsigned long long current_time, SimpleTTEMessage* message, int length){
	GPIO |= (1U << RECVTASK_GPIO_BIT);
	if(nodeRecvEnable)
	{
		unsigned short ethFrameType = UNSUPPORTED;
		unsigned long long listen_start = get_cpu_usecs();
		#pragma loopbound min 1 max 1
		do{
			if(eth_mac_receive_nb(rx_addr))
			{
				ethFrameType = (unsigned short) ((mem_iord_byte(rx_addr + 12) << 8) + (mem_iord_byte(rx_addr + 13)));
				LEDS &= 0xF0;
				LEDS |= (ethFrameType & 0xF);
				if(ethFrameType != 0x891D)
				{
					break;
				}
			}
		}while(get_cpu_usecs() - listen_start < 2*SYNC_WINDOW_HALF*NS_TO_USEC);
		if(ethFrameType != UNSUPPORTED)
		{
			//Data
			#pragma loopbound min 16 max 16
			for(int i=0; i<length; i++)
			{
				*((unsigned char*)message + i) = mem_iord_byte(rx_addr + 18 + i);
			}
			rxMsgCount++;
		}
		LEDS = (LEDS & 0x1F0) + (ethFrameType & 0xF);
	}
	GPIO &= (0U << RECVTASK_GPIO_BIT);
}

__attribute__((noinline))
void cyclic_executive_loop(SimpleTTETask* sched){
	// printf("Cyclic executive role: %d\n", exec_role);
	SimpleTTEMessage outgoing_message = {.seq = 0, .origin_ts = 0, .val = 0};
	SimpleTTEMessage incoming_message = {.seq = 0, .origin_ts = 0, .val = 0};
	unsigned long long start_time = get_ptp_nanos(thisPtpPortInfo.eth_base);
	#pragma loopbound min 1 max 1
	while(KEYS != 0xE){
    	register unsigned long long schedule_time = get_ptp_nanos(thisPtpPortInfo.eth_base);
		schedule_time = get_tte_aligned_time(schedule_time - start_time, HYPER_PERIOD);
		#pragma loopbound min 1 max 1
		for(int i=0; i<NUM_OF_TASKS; i++){
			if(schedule_time >= sched[i].release_times[sched[i].release_inst]){
				switch (i)
				{
				case 0:
          			task_sync(start_time, schedule_time, sched);
					break;
				case 1:
          			task_calc(schedule_time, &outgoing_message, &incoming_message);
					break;
				case 2:
          			task_send(schedule_time, &outgoing_message, sizeof(SimpleTTEMessage), ROLE==PRODUCER ? TTE_PRODUCER_VL : TTE_CONSUMER_VL);
					break;
				case 3:
					task_recv(schedule_time, &incoming_message, sizeof(SimpleTTETask));
					break;
				case 4:
          			task_pulse(schedule_time);
					break;
				}
				sched[i].release_times[sched[i].release_inst] += HYPER_PERIOD;
				sched[i].release_inst = (sched[i].release_inst + 1) % sched[i].nr_releases;
				sched[i].delta_sum += sched[i].last_time == 0 ? 0 : (schedule_time - get_tte_aligned_time(sched[i].last_time, sched[i].period));
				sched[i].last_time = schedule_time;
				sched[i].exec_count += 1;
			}
		}
	}
}

int main(int argc, char **argv){
	LEDS = GPIO = 0x1FF;
	puts("\nTTEthernet Ping Demo Started");

	//MAC controller settings
	set_mac_address(0x1D000400, 0x00000289);
	eth_iowr(MODER, (RECSMALL_BIT | CRCEN_BIT | FULLD_BIT | PRO_BIT | TXEN_BIT | RXEN_BIT));
	eth_iowr(INT_SOURCE, INT_SOURCE_RXB_BIT);
    eth_iowr(RX_BD_ADDR_BASE(eth_iord(TX_BD_NUM)), RX_BD_EMPTY_BIT | RX_BD_IRQEN_BIT | RX_BD_WRAP_BIT);
	eth_iowr(INT_SOURCE, INT_SOURCE_TXB_BIT);
	eth_iowr(TX_BD_ADDR_BASE, TX_BD_READY_BIT | TX_BD_IRQEN_BIT | TX_BD_PAD_EN_BIT);
	thisPtpPortInfo = ptpv2_intialize_local_port(PATMOS_IO_ETH, PTP_SLAVE, my_mac, my_ip, 1, 0);
	LEDS = GPIO = 0x0;

	SimpleTTETask task_schedule[NUM_OF_TASKS];
	for(unsigned i=0; i < NUM_OF_TASKS; i++){
		init_simple_simplettetask(&task_schedule[i], (unsigned long long) tasks_periods[i], tasks_insts_counts[i], tasks_schedules[i]);
	}


	// sort_asc_ttetasks(task_schedule, NO_TASKS);

	// Executive Loop
	do{
		nodeFirstSync = 1;
		nodeColdStart = 1;
		nodeIntegrated = 0;
		nodeSyncStable = 0;
		clkDiffLast = 0;
		clkDiffSum = 0;
		clkDiff = 0;
		cyclic_executive_loop(task_schedule);
	}while(KEYS !=  0xE && rxMsgCount < NUM_RX_PACKETS);

	//Report
	puts("\nTTEthernet Ping Demo Exiting and Reporting...");
	puts("------------------------------------------");
	puts("Clock Sync Quality Log:");
	printf("--Avg. clock offset = %lld ns\n", clkDiffSum / rxPcfCount);
	puts("---------------------------------------------------------------------------");
	puts("Communication Log:");
	printf("--Avg. end-to-end latency = %.4f ms\n", abs(endToEndSum/rxMsgCount) * NS_TO_USEC * USEC_TO_MS);
	printf("--Received No. of Frames = %u\n", rxPcfCount+rxMsgCount);
	printf("--In-Sched No. of PCF = %u (loss = %.3f %%) \n", stableCycles, 100-((float)stableCycles/(float)rxPcfCount)*100);
	printf("--No-Sched No. of PCF = %u (success = %.3f %%) \n", unstableCycles, 100-((float)unstableCycles/(float)rxPcfCount)*100);
	printf("--Received No. of Message Frames = %u\n", rxMsgCount);
	printf("--Transmit No. of Message Frames = %u\n", txMsgCount);
	puts("---------------------------------------------------------------------------");
	puts("Task log:");
	for(int i=0; i<NUM_OF_TASKS; i++){
		unsigned long long avgDelta = task_schedule[i].delta_sum/task_schedule[i].exec_count;
		printf("--task[%d]   avg. dt = %.4f us\t(avg. jitter = %.4f us) from a total of %lu executions\n", i, avgDelta * NS_TO_USEC, abs(task_schedule[i].period - avgDelta) * NS_TO_USEC, task_schedule[i].exec_count);
	}
	puts("---------------------------------------------------------------------------");
	
	return 0;
}