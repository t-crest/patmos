#include "rosace_dist_common.h"

struct nonencoded_task_params* tasks;
int num_of_tasks;
schedtime_t hyper_period;
MinimalTTTask *schedule;

// Output variables
output_t outs;
uint32_t step_simu;
uint32_t max_step_simu;

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

unsigned int tx_buff_addr = 0x800;

unsigned int rx_buff_addr = 0x000;
unsigned int rx_buff2_addr = 0x400;

unsigned int rx_bd_addr = 0x600;
unsigned int rx_bd2_addr = 0x608;

uint64_t REAL_TYPEToBytes(REAL_TYPE x){
	const union {REAL_TYPE f; uint64_t b;} val = {.f = x};
	return val.b;
}

REAL_TYPE bytesToDouble(uint64_t x){
	const union {REAL_TYPE f; uint64_t b;} val = {.b = x};
	return val.f;
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

unsigned getSegmentInt() 
{
	return (*(&SEGDISP+7) << 28) + (*(&SEGDISP+6) << 24) + (*(&SEGDISP+5) << 20) 
			 + (*(&SEGDISP+4) << 16) + (*(&SEGDISP+3) << 12) + (*(&SEGDISP+2) << 8)
			 + (*(&SEGDISP+1) << 4) + (*(&SEGDISP+0) & 0xF);
}

__attribute__((noinline))
void reset_sync()
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
	RTC_TIME_NS(PATMOS_IO_ETH) = 0x0;
	RTC_TIME_SEC(PATMOS_IO_ETH) = 0x0;
}

__attribute__((noinline))
void config_ethmac()
{
	set_mac_address(0x1D000400, 0x00000289);
	eth_iowr(MODER_ADDR, (RECSMALL_BIT | CRCEN_BIT | FULLD_BIT | PRO_BIT | TXEN_BIT | RXEN_BIT));
	eth_iowr(INT_SOURCE_ADDR, INT_SOURCE_RXB_BIT);
	
	eth_iowr(TX_BD_ADDR_BASE, 0x0 | TX_BD_IRQEN_BIT | TX_BD_PAD_EN_BIT | TX_BD_WRAP_BIT);
	eth_iowr(TX_BD_ADDR_BASE+4, tx_buff_addr);

	rx_bd_addr = RX_BD_ADDR_BASE(eth_iord(TX_BD_NUM_ADDR));
  eth_iowr(rx_bd_addr, RX_BD_EMPTY_BIT | RX_BD_IRQEN_BIT | 0x0);
	eth_iowr(rx_bd_addr+4, rx_buff_addr);
	
	rx_bd2_addr = RX_BD_ADDR_BASE(eth_iord(TX_BD_NUM_ADDR)) + 8;
	eth_iowr(rx_bd2_addr, RX_BD_EMPTY_BIT | RX_BD_IRQEN_BIT | RX_BD_WRAP_BIT);
	eth_iowr(rx_bd2_addr+4, rx_buff2_addr);
}

void swap_eth_rx_buffers()
{
	unsigned int temp = rx_buff_addr;
	rx_buff_addr = rx_buff2_addr;
	rx_buff2_addr = temp;
	temp = rx_bd_addr;
	rx_bd_addr = rx_bd2_addr;
	rx_bd2_addr = temp;
}

int eth_mac_poll_for_frames()
{
  int ethType = TIMEOUT;
  schedtime_t listen_start = get_cpu_usecs();
	 do{
    if(eth_mac_has_frame(rx_buff_addr))
    {
      eth_mac_clear_rx_buffer(rx_buff2_addr, rx_bd2_addr);
      ethType = (unsigned short) ((mem_iord_byte(rx_buff_addr + 12) << 8) + (mem_iord_byte(rx_buff_addr + 13)));
      break;
    }
  }while(get_cpu_usecs() - listen_start <= (2*TTE_RECV_WINDOW_HALF*NS_TO_USEC));
	return ethType;
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
	unsigned long long listen_start = get_cpu_usecs(); //keep track when we started listening
	#pragma loopbound min 1 max 1
	do
	{
		if(eth_mac_has_frame(rx_buff_addr))
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
				PTP_RXCHAN_STATUS(PATMOS_IO_ETH) = 0x1; //Clear PTP timestampAvail flag
			} else {
				ethFrameType = UNSUPPORTED;
			}
			eth_mac_clear_rx_buffer(rx_buff2_addr, rx_bd2_addr);
			break;
		}
	} while(get_cpu_usecs() - listen_start <= (2*TTE_SYNC_WINDOW_HALF*NS_TO_USEC));
	//work
	if(ethFrameType != TIMEOUT)
	{
		if(ethFrameType == TTE_PCF)
		{
			tte_pcf_handle(current_time, start_time);
			#pragma loopbound min 1 max 5
			for(int i=0; i<num_of_tasks; i++)
			{
				#pragma loopbound min 1 max 4
				for(int n=0; n<tasks[i].nr_releases; n++)
					tasks[i].release_times[n] = get_tte_aligned_time(tasks[i].release_times[n], tasks[i].period);
			}
			rxPcfCount += 1;
		} else {
			unstableCycles++;
		}
		//Either way swap buffers and update sync state
		swap_eth_rx_buffers();
		int syncedCyclesWeight = (int) (stableCycles - unstableCycles);
		nodeIntegrated = !nodeColdStart && syncedCyclesWeight > TTE_ASYNC2SYNC_THRES_CYCLES  && abs(clkDiff) <= TTE_PRECISION;
	} else {
			unstableCycles++;
			nodeIntegrated = 0;
	}
	int stableClustersWeight = (int) (stableClusters - unstableClusters);
	if((nodeFirstSync && integration_cycle == 0 && nodeIntegrated) || !nodeFirstSync)
	{
		nodeSyncStable = nodeIntegrated &&  stableClustersWeight > TTE_ASYNC2SYNC_THRES_CLUSTERS;	
		nodeFirstSync = 0;
	} 
	else if (!nodeIntegrated || !nodeSyncStable)
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
	printSegmentInt(stableClustersWeight << 16 | (abs(clkDiff) & 0xFFFF));
	LEDS = (nodeSyncStable << 7) + (nodeIntegrated << 6) + (ethFrameType & 0xF);
	GPIO &= (0U << SYNCTASK_GPIO_BIT);
}

//This function sends an UDP packet to the dstination IP and destination MAC.
__attribute__((noinline))
int udp_send_tte(unsigned int tx_addr, unsigned char tte_ct[], unsigned char tte_vl[], unsigned char tte_mac[], unsigned char destination_ip[], unsigned char source_ip[], unsigned short source_port, unsigned short destination_port, unsigned char data[], unsigned short data_length, uint16_t ipv4_id)
{
	//Resolve the ip address
	unsigned short udp_length = data_length + 8;
	unsigned short ip_length = udp_length + 20;
	unsigned short frame_length = ip_length + 14;
	//MAC addrs
	mem_iowr(tx_addr, (tte_ct[0] << 24) | (tte_ct[1] << 16) | (tte_ct[2] << 8) | tte_ct[3]);
	mem_iowr(tx_addr + 4, (tte_vl[0] << 24) | (tte_vl[1] << 16) | (tte_mac[0] << 8) | tte_mac[1]);
	mem_iowr(tx_addr + 8, (tte_mac[2] << 24) | (tte_mac[3] << 16) | (tte_mac[4] << 8) | tte_mac[5]);
	//MAC type + IP version + IP type
	mem_iowr(tx_addr + 12, 0x08004500);
	//Length + Identification
	mem_iowr(tx_addr + 16, (ip_length << 16) | (ipv4_id));
	//Flags + TTL + Protocol
	mem_iowr(tx_addr + 20, 0x40004011);
	//IP addrs + Ports + UDP Length
	mem_iowr(tx_addr + 24, (source_ip[0] << 8) | source_ip[1]);
	mem_iowr(tx_addr + 28, (source_ip[2] << 24) | (source_ip[3] << 16) | (destination_ip[0] << 8) | destination_ip[1]);
	mem_iowr(tx_addr + 32, (destination_ip[2] << 24) | (destination_ip[3] << 16) | source_port);
	mem_iowr(tx_addr + 36, (destination_port << 16) | udp_length);
	mem_iowr(tx_addr + 40, 0x0000);
	//UDP Data
	_Pragma("loopbound min 0 max 64")
	for (int i=0; i<data_length; i++){
		mem_iowr_byte(tx_addr + 42 + i, data[i]);//Sender myip
	}
	//IPv4 checksum
	unsigned short int checksum = ipv4_compute_checksum(tx_addr);
	mem_iowr_byte(tx_addr + 24, (checksum >> 8));
	mem_iowr_byte(tx_addr + 25, (checksum & 0xFF));
	// UDP checksum
	checksum = udp_compute_checksum(tx_addr);
	mem_iowr_byte(tx_addr + 40, (checksum >> 8));
	mem_iowr_byte(tx_addr + 41, (checksum & 0xFF));
	//Actually send
	unsigned temp_config = eth_iord(INT_SOURCE_ADDR);
	temp_config = eth_iord(TX_BD_ADDR_BASE);
	eth_iowr(TX_BD_ADDR_BASE, temp_config | TX_BD_READY_BIT | ((frame_length<<16)|(0xF000)));
	eth_iowr(INT_SOURCE_ADDR, temp_config | INT_SOURCE_TXB_BIT | TXEN_BIT);
	return 1;
}