#include "rosace_dist_common.h"

// Clock related variables
PTPPortInfo thisPtpPortInfo;
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
  // printf("%3.2f,%5.3f,%5.3f,%5.4f,%5.3f,%5.3f,%5.4f,%5.4f\n", 
  // outs.t_simu/1000.0f, outs.sig_outputs.Va, outs.sig_outputs.az, 
  // outs.sig_outputs.q, outs.sig_outputs.Vz, outs.sig_outputs.h,
  // outs.sig_delta_th_c,outs.sig_delta_e_c);
  printSegmentInt(
     (0xFF000000 & (unsigned) outs.sig_outputs.Va << 24) + 
   + (0x00FF0000 & (unsigned) outs.sig_outputs.Vz << 16)
   + (0xFFFF & (unsigned)outs.sig_outputs.h)
  );
  step_simu += 1;
  return 1;
}

__attribute__((noinline))
void sync_fun(unsigned long long start_time, unsigned long long current_time, MinimalTTTask* tasks)
{
	GPIO |= (1U << SYNCTASK_GPIO_BIT);
	int ethFrameType;
	PTP_RXCHAN_STATUS(thisPtpPortInfo.eth_base) = 0x1; //Clear timestampAvail flag
	unsigned long long listen_start = get_cpu_usecs(); //keep track when we started listening
	#pragma loopbound min 1 max 1
	do
	{
		if(eth_mac_receive_nb(rx_buff_addr))
		{
			if((unsigned short) ((mem_iord_byte(rx_buff_addr + 12) << 8) + (mem_iord_byte(rx_buff_addr + 13))) == 0x891D)
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
	} while(ethFrameType != TTE_PCF && get_cpu_usecs() - listen_start < 2*TTE_SYNC_WINDOW_HALF*NS_TO_USEC);
	//work
	if(ethFrameType == TTE_PCF)
	{
		tte_pcf_handle(current_time, start_time);
		nodeIntegrated = !nodeColdStart && ethFrameType > 0 && (stableCycles - unstableCycles) > 0 && abs(clkDiff) < TTE_PRECISION;
		if((!nodeFirstSync) || (nodeFirstSync && integration_cycle == 0 && nodeIntegrated))
        {
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
		clkDiffLast = 0;
		clkDiffSum = 0;
		clkDiff = 0;
	}
	if(nodeIntegrated){
		#pragma loopbound min 5 max 8
		for(int i=0; i<num_of_tasks; i++)
		{
			tasks[i].release_times[tasks->release_inst] = get_tte_aligned_time(tasks[i].release_times[tasks->release_inst]);
			tasks[i].release_times[tasks->release_inst+1] = get_tte_aligned_time(tasks[i].release_times[tasks->release_inst+1]);
		}
	}
	printSegmentInt(abs(clkDiff));
	LEDS = (nodeSyncStable << 7) + (nodeIntegrated << 6) + (ethFrameType & 0xF);
	GPIO &= (0U << SYNCTASK_GPIO_BIT);
}