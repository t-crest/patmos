#include "rosace_dist_common.h"

struct nonencoded_task_params* tasks;
int num_of_tasks;
schedtime_t hyper_period;
MinimalTTTask *schedule;

// Output variables
output_t outs;
uint64_t step_simu;
uint64_t max_step_simu;

// Clock related variables
PTPv2Time hardTimestamp;
PTPv2Time softTimestamp;
long long clkDiff = 0;
long long clkDiffLast = 0;
long long clkDiffSum = 0;
unsigned short integration_cycle = 0;
unsigned long long coldStartIntegrationTime = 0;
unsigned long long initNanoseconds = 0;
unsigned long long initSeconds = 0;

// Counters
unsigned short rxPcfCount = 0;
unsigned int stableCycles = 0;
unsigned int unstableCycles = 0;
unsigned int stableClusters = 0;
unsigned int unstableClusters = 0;
unsigned int totalScheduledReceptions = 0;

// Flags
unsigned char nodeIntegrated = 0;	//is used to indicate when the node has achieved sufficient syncrhonization
unsigned char nodeSyncStable = 0;	//is used to enable task execution when the node is in a stable sync
unsigned char nodeColdStart = 1;		//is used to indicate that a node has just booted and has not received a single PCF
unsigned char nodeFirstSync = 1;
unsigned char nodeRecvEnable = 0;

// TTE PID synchronization
#define TTE_SYNC_Kp 1000LL
#define TTE_SYNC_Ki 0LL

// TTE directives
#define TIME_CORRECTION_EN
#define HW_TIMESTAMPING

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

__attribute__((noinline))
void copy_output_vars(output_t* v, uint64_t step)
{
  v->t_simu           = step * STEP_TIME_SCALE;
	v->sig_outputs.Va 	= aircraft_dynamics495_Va_Va_filter_100449_Va[step%2];
	v->sig_outputs.Vz 	= aircraft_dynamics495_Vz_Vz_filter_100452_Vz[step%2];
	v->sig_outputs.q  	= aircraft_dynamics495_q_q_filter_100455_q[step%2];
	v->sig_outputs.az 	= aircraft_dynamics495_az_az_filter_100458_az[step%2];
	v->sig_outputs.h  	= aircraft_dynamics495_h_h_filter_100446_h[step%2];
	v->sig_delta_th_c   = Va_control_50474_delta_th_c_delta_th_c;
	v->sig_delta_e_c	  = Vz_control_50483_delta_e_c_delta_e_c;
}

__attribute__((noinline))
int logging_fun(void *args)
{
  copy_output_vars(&outs, step_simu);
  printf("%3.2f,%5.3f,%5.3f,%5.4f,%5.3f,%5.3f,%5.4f,%5.4f\n", 
  outs.t_simu/1000.0f, outs.sig_outputs.Va, outs.sig_outputs.az, 
  outs.sig_outputs.q, outs.sig_outputs.Vz, outs.sig_outputs.h,
  outs.sig_delta_th_c,outs.sig_delta_e_c);
  return 1;
}

void init_sync()
{
  nodeFirstSync = 1;
  nodeColdStart = 1;
  nodeIntegrated = 0;
  nodeSyncStable = 0;
  stableCycles = 0;
  unstableCycles = 0;
  stableClusters = 0;
  unstableClusters = 0;
  clkDiffLast = 0;
  clkDiffSum = 0;
  clkDiff = 0;
  rxPcfCount = 0;
}

uint8_t isNodeSyncStable()
{
	return nodeSyncStable;
}

__attribute__((noinline))
unsigned long long get_tte_aligned_time(unsigned long long current_time, unsigned long long corr_limit)
{
	long long clock_corr = (((TTE_SYNC_Kp*clkDiff)>>10) + ((TTE_SYNC_Ki*clkDiffSum)>>10));
		// if(clock_corr < 0){
	// 	clock_corr = MAX(-corr_limit, clock_corr);
	// } else {
	// 	clock_corr = MIN(corr_limit, clock_corr);
	// }
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

	trans_clock = mem_iord_byte(rx_buff_addr + 34);
	trans_clock = (trans_clock << 8) | (mem_iord_byte(rx_buff_addr + 35));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(rx_buff_addr + 36));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(rx_buff_addr + 37));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(rx_buff_addr + 38));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(rx_buff_addr + 38));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(rx_buff_addr + 40));
	trans_clock = (trans_clock << 8) | (mem_iord_byte(rx_buff_addr + 41));
	trans_clock = trans_clock / TTETIME_TO_NS;

	integration_cycle = mem_iord_byte(rx_buff_addr + 14);
	integration_cycle = (integration_cycle << 8) | (mem_iord_byte(rx_buff_addr + 15));
	integration_cycle = (integration_cycle << 8) | (mem_iord_byte(rx_buff_addr + 16));
	integration_cycle = (integration_cycle << 8) | (mem_iord_byte(rx_buff_addr + 17));

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
void sync_fun(unsigned long long start_time, unsigned long long current_time, MinimalTTTask* tasks)
{
	GPIO |= (1U << SYNCTASK_GPIO_BIT);
	int ethFrameType = TIMEOUT;
	PTP_RXCHAN_STATUS(PATMOS_IO_ETH) = 0x1; //Clear timestampAvail flag
	unsigned long long listen_start = get_cpu_usecs(); //keep track when we started listening
	#pragma loopbound min 1 max 1
	do
	{
		if(eth_mac_receive_nb(rx_buff_addr))
		{
			if((unsigned short) ((mem_iord_byte(rx_buff_addr + 12) << 8) + (mem_iord_byte(rx_buff_addr + 13))) == 0x891D)
			{
				#ifdef HW_TIMESTAMPING
				hardTimestamp.nanoseconds = PTP_RXCHAN_TIMESTAMP_NS(PATMOS_IO_ETH);
				hardTimestamp.seconds = PTP_RXCHAN_TIMESTAMP_SEC(PATMOS_IO_ETH);
				#else
				softTimestamp.nanoseconds = (unsigned) RTC_TIME_NS(PATMOS_IO_ETH);
				softTimestamp.seconds = (unsigned) RTC_TIME_SEC(PATMOS_IO_ETH);
				#endif
				ethFrameType = TTE_PCF;
			}
			PTP_RXCHAN_STATUS(PATMOS_IO_ETH) = 0x1; //Clear PTP timestampAvail flag
		}
	} while(ethFrameType != TTE_PCF && get_cpu_usecs() - listen_start < 2*TTE_SYNC_WINDOW_HALF*NS_TO_USEC);
	//work
	if(ethFrameType == TTE_PCF)
	{
		tte_pcf_handle(current_time, start_time);
		nodeIntegrated = !nodeColdStart && ethFrameType > 0 && (stableCycles - unstableCycles) > 0 && abs(clkDiff) < TTE_PRECISION;
		if((nodeFirstSync && integration_cycle == 0 && nodeIntegrated) || !nodeFirstSync){
			nodeSyncStable = nodeIntegrated && (stableClusters - unstableClusters) > TTE_ASYNC2SYNC_THRES;	
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
		stableCycles = 0;
		unstableCycles = 0;
		stableClusters = 0;
		unstableClusters = 0;
		clkDiffLast = 0;
		clkDiffSum = 0;
		clkDiff = 0;
	}
	// if(nodeIntegrated){
		#pragma loopbound min 1 max 5
		for(int i=0; i<num_of_tasks; i++)
		{
			#pragma loopbound min 1 max 4
			for(int n=0; n<tasks[i].nr_releases; n++)
				tasks[i].release_times[n] = get_tte_aligned_time(tasks[i].release_times[n], tasks[i].period);
		}
	// }
	printSegmentInt(abs(clkDiff));
	LEDS = (nodeSyncStable << 7) + (nodeIntegrated << 6) + (ethFrameType & 0xF);
	GPIO &= (0U << SYNCTASK_GPIO_BIT);
  step_simu += 1;
}