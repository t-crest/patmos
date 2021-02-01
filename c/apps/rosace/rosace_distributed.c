#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h> //just for fabs
#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/rtc.h>
#include "assemblage_includes.h"
#include "assemblage.h"
#include "rosace_dist_common.h"

#ifdef AIRCRAFT_NODE
#include "rosace_aircraft_schedule.h"
#elif FILTER_NODE
#include "rosace_filter_schedule.h"
#elif CONTROL_NODE
#include "rosace_control_schedule.h"
#endif

static unsigned rxMsgCount;
static unsigned txMsgCount;

char* eth_protocol_names[] = {"UFF", "IP", "ICMP", "UDP", "TCP", "PTP", "ARP", "LLDP", "MDNS", "TTE_PCF", "TTE"};
unsigned int rx_buff_addr = 0x000;
unsigned int tx_buff_addr = 0x800;
unsigned char multicastip[4] = {224, 0, 0, 255};
unsigned char TTE_MAC[] = {0x02, 0x89, 0x1D, 0x00, 0x04, 0x00};
unsigned char TTE_CT[] = { 0xAB, 0xAD, 0xBA, 0xBE };
unsigned char TTE_DYN_VL[] = { 0x0F, 0xA1 };
unsigned char TTE_FILTER_VL[] = { 0x0F, 0xA2 };
unsigned char TTE_CTRL_VL[] = { 0x0F, 0xA3 };

#pragma region AIRCRAFT_NODE

__attribute__((noinline))
int vl_dyn_send_fun(void *args)
{
  GPIO |= (1U << SENDTASK_GPIO_BIT);
  int ans = 0;
  double *aircraft_to_filters_data[5] = {
    aircraft_dynamics495_Va_Va_filter_100449_Va,
    aircraft_dynamics495_az_az_filter_100458_az,
    aircraft_dynamics495_Vz_Vz_filter_100452_Vz,
    aircraft_dynamics495_q_q_filter_100455_q,
    aircraft_dynamics495_h_h_filter_100446_h
  };
  size_t length = 2*sizeof(aircraft_to_filters_data);
	if(isNodeSyncStable())
	{
		unsigned char ethType[2];
		ethType[0]=((4+length)>>8) & 0xFF;
		ethType[1]=(4+length) & 0xFF;
		//Header
		#pragma loopbound min 4 max 4
		for(int i=0; i<4; i++)
		{
			mem_iowr_byte(tx_buff_addr + i, TTE_CT[i]);
		}
		mem_iowr_byte(tx_buff_addr + 4, TTE_DYN_VL[0]);
		mem_iowr_byte(tx_buff_addr + 5, TTE_DYN_VL[1]);

		#pragma loopbound min 6 max 6
		for(int i=6; i<12; i++)
		{
			mem_iowr_byte(tx_buff_addr + i, TTE_MAC[i-6]);
		}
		mem_iowr_byte(tx_buff_addr + 12, ethType[0]);
		mem_iowr_byte(tx_buff_addr + 13, ethType[1]);
		//Data
		#pragma loopbound min 10 max 10
		for(int j=0; j<5; j++)
    {
		  for(int i=0; i<2; i++)
			{
        mem_iowr(tx_buff_addr + 18 + 0, ((schedtime_t) aircraft_to_filters_data[i][j] & 0xFFFFFFFF));
        mem_iowr(tx_buff_addr + 18 + 4, ((schedtime_t) aircraft_to_filters_data[i][j] >> 32));
			}
		}
		eth_mac_send_nb(tx_buff_addr, 18+4+length);
		txMsgCount++;
    ans = 1;
	}
	GPIO &= (0U << SENDTASK_GPIO_BIT);
  return ans;
}

__attribute__((noinline))
int vl_ctrl_recv_fun(void* args)
{
  GPIO |= (1U << RECVTASK_GPIO_BIT);
  int ans = 0;
  double ctrl_to_aircraft_data[4] = {
    Vz_control_50483_delta_e_c_elevator489_delta_e_c,
    Vz_control_50483_delta_e_c_delta_e_c,
    Va_control_50474_delta_th_c_delta_th_c,
    Va_control_50474_delta_th_c_engine486_delta_th_c
  };
  size_t length = 2*sizeof(ctrl_to_aircraft_data);
	if(isNodeSyncStable()){
		unsigned short ethType = UNSUPPORTED;
		schedtime_t listen_start = get_cpu_usecs();
		#pragma loopbound min 1 max 1
		do{
			if(eth_mac_receive_nb(rx_buff_addr))
			{
				ethType = (unsigned short) ((mem_iord_byte(rx_buff_addr + 12) << 8) + (mem_iord_byte(rx_buff_addr + 13)));
				LEDS &= 0xF0;
				LEDS |= (ethType & 0xF);
				if(ethType != 0x891D)
				{
					break;
				}
			}
		}while(get_cpu_usecs() - listen_start < 2*TTE_RECV_WINDOW_HALF*NS_TO_USEC);
		if(ethType != UNSUPPORTED)
		{
			//Data
      #pragma loopbound min 4 max 4
      for(int i=0; i<4; i++)
      {
          ctrl_to_aircraft_data[i] = (double)(((schedtime_t)mem_iord(rx_buff_addr + 18 + 4) << 32) || (mem_iord(rx_buff_addr + 18) & 0xFFFFFFFF));
      }
			rxMsgCount++;
			LEDS |= (3U << 4);
		} 
		else 
		{
			LEDS |= (1U << 5);
		}
	}
	GPIO &= (0U << RECVTASK_GPIO_BIT);
  return ans;
}

#pragma endregion AIRCRAFT_NODE

#pragma region FILTER_NODE

 __attribute__((noinline))
int vl_dyn_recv_fun(void *args)
{
  GPIO |= (1U << RECVTASK_GPIO_BIT);
  int ans = 0;
  double *aircraft_to_filters_data[5] = {
    aircraft_dynamics495_Va_Va_filter_100449_Va,
    aircraft_dynamics495_az_az_filter_100458_az,
    aircraft_dynamics495_Vz_Vz_filter_100452_Vz,
    aircraft_dynamics495_q_q_filter_100455_q,
    aircraft_dynamics495_h_h_filter_100446_h
  };
  size_t length = 2*sizeof(aircraft_to_filters_data);
  if(isNodeSyncStable()){
		unsigned short ethType = UNSUPPORTED;
		schedtime_t listen_start = get_cpu_usecs();
		#pragma loopbound min 1 max 1
		do{
			if(eth_mac_receive_nb(rx_buff_addr))
			{
				ethType = (unsigned short) ((mem_iord_byte(rx_buff_addr + 12) << 8) + (mem_iord_byte(rx_buff_addr + 13)));
				LEDS &= 0xF0;
				LEDS |= (ethType & 0xF);
				if(ethType != 0x891D)
				{
					break;
				}
			}
		}while(get_cpu_usecs() - listen_start < 2*TTE_RECV_WINDOW_HALF*NS_TO_USEC);
		if(ethType != UNSUPPORTED)
		{
			//Data
      #pragma loopbound min 10 max 10
      for(int i=0; i<5; i++)
      {
        for(int j=0; j<2; j++)
        {
          aircraft_to_filters_data[i][j] = (double)(((schedtime_t)mem_iord(rx_buff_addr + 18 + 4) << 32) || (mem_iord(rx_buff_addr + 18) & 0xFFFFFFFF));
        }
      }
			rxMsgCount++;
			LEDS |= (3U << 4);
		} 
		else 
		{
			LEDS |= (1U << 5);
		}
	}
	GPIO &= (0U << RECVTASK_GPIO_BIT);
  return ans;
}

__attribute__((noinline))
int vl_filter_send_fun(void *args)
{
  GPIO |= (1U << SENDTASK_GPIO_BIT);
  int ans = 0;
  double *filters_to_ctrl_data[7] = {
    h_filter_100446_h_f_altitude_hold_50464_h_f,
    q_filter_100455_q_f_Va_control_50474_q_f,
    q_filter_100455_q_f_Vz_control_50483_q_f,
    az_filter_100458_az_f_Vz_control_50483_az_f,
    Vz_filter_100452_Vz_f_Va_control_50474_Vz_f,
    Vz_filter_100452_Vz_f_Vz_control_50483_Vz_f,
    Va_filter_100449_Va_f_Va_control_50474_Va_f
  };
  size_t length = 2*sizeof(filters_to_ctrl_data);
	if(isNodeSyncStable())
	{
		unsigned char ethType[2];
		ethType[0]=((4+length)>>8) & 0xFF;
		ethType[1]=(4+length) & 0xFF;
		//Header
		#pragma loopbound min 4 max 4
		for(int i=0; i<4; i++)
		{
			mem_iowr_byte(tx_buff_addr + i, TTE_CT[i]);
		}
		mem_iowr_byte(tx_buff_addr + 4, TTE_DYN_VL[0]);
		mem_iowr_byte(tx_buff_addr + 5, TTE_DYN_VL[1]);

		#pragma loopbound min 6 max 6
		for(int i=6; i<12; i++)
		{
			mem_iowr_byte(tx_buff_addr + i, TTE_MAC[i-6]);
		}
		mem_iowr_byte(tx_buff_addr + 12, ethType[0]);
		mem_iowr_byte(tx_buff_addr + 13, ethType[1]);
		//Data
		#pragma loopbound min 10 max 10
		for(int j=0; j<7; j++)
    {
		  for(int i=0; i<2; i++)
			{
        mem_iowr(tx_buff_addr + 18 + 0, ((schedtime_t) filters_to_ctrl_data[i][j] & 0xFFFFFFFF));
        mem_iowr(tx_buff_addr + 18 + 4, ((schedtime_t) filters_to_ctrl_data[i][j] >> 32));
			}
		}
		eth_mac_send_nb(tx_buff_addr, 18+4+length);
		txMsgCount++;
    ans = 1;
	}
	GPIO &= (0U << SENDTASK_GPIO_BIT);
  return ans;
}

#pragma endregion FILTER_NODE

#pragma region CONTROL_NODE

__attribute__((noinline))
int vl_filter_recv_fun(void *args)
{
  GPIO |= (1U << RECVTASK_GPIO_BIT);
  int ans = 0;
  double *filters_to_ctrl_data[7] = {
    h_filter_100446_h_f_altitude_hold_50464_h_f,
    q_filter_100455_q_f_Va_control_50474_q_f,
    q_filter_100455_q_f_Vz_control_50483_q_f,
    az_filter_100458_az_f_Vz_control_50483_az_f,
    Vz_filter_100452_Vz_f_Va_control_50474_Vz_f,
    Vz_filter_100452_Vz_f_Vz_control_50483_Vz_f,
    Va_filter_100449_Va_f_Va_control_50474_Va_f
  };
  size_t length = 2*sizeof(filters_to_ctrl_data);
  if(isNodeSyncStable()){
		unsigned short ethType = UNSUPPORTED;
		schedtime_t listen_start = get_cpu_usecs();
		#pragma loopbound min 1 max 1
		do{
			if(eth_mac_receive_nb(rx_buff_addr))
			{
				ethType = (unsigned short) ((mem_iord_byte(rx_buff_addr + 12) << 8) + (mem_iord_byte(rx_buff_addr + 13)));
				LEDS &= 0xF0;
				LEDS |= (ethType & 0xF);
				if(ethType != 0x891D)
				{
					break;
				}
			}
		}while(get_cpu_usecs() - listen_start < 2*TTE_RECV_WINDOW_HALF*NS_TO_USEC);
		if(ethType != UNSUPPORTED)
		{
			//Data
      #pragma loopbound min 10 max 10
      for(int i=0; i<5; i++)
      {
        for(int j=0; j<2; j++)
        {
          filters_to_ctrl_data[i][j] = (double)(((schedtime_t)mem_iord(rx_buff_addr + 18 + 4) << 32) || (mem_iord(rx_buff_addr + 18) & 0xFFFFFFFF));
        }
      }
			rxMsgCount++;
			LEDS |= (3U << 4);
		} 
		else 
		{
			LEDS |= (1U << 5);
		}
	}
	GPIO &= (0U << RECVTASK_GPIO_BIT);
  return ans;
}

__attribute__((noinline))
int vl_ctrl_send_fun(void *args)
{
  GPIO |= (1U << SENDTASK_GPIO_BIT);
  int ans = 0;
  double ctrl_to_aircraft_data[4] = {
    Vz_control_50483_delta_e_c_elevator489_delta_e_c,
    Vz_control_50483_delta_e_c_delta_e_c,
    Va_control_50474_delta_th_c_delta_th_c,
    Va_control_50474_delta_th_c_engine486_delta_th_c
  };
  size_t length = 2*sizeof(ctrl_to_aircraft_data);
	if(isNodeSyncStable())
	{
		unsigned char ethType[2];
		ethType[0]=((4+length)>>8) & 0xFF;
		ethType[1]=(4+length) & 0xFF;
		//Header
		#pragma loopbound min 4 max 4
		for(int i=0; i<4; i++)
		{
			mem_iowr_byte(tx_buff_addr + i, TTE_CT[i]);
		}
		mem_iowr_byte(tx_buff_addr + 4, TTE_DYN_VL[0]);
		mem_iowr_byte(tx_buff_addr + 5, TTE_DYN_VL[1]);

		#pragma loopbound min 6 max 6
		for(int i=6; i<12; i++)
		{
			mem_iowr_byte(tx_buff_addr + i, TTE_MAC[i-6]);
		}
		mem_iowr_byte(tx_buff_addr + 12, ethType[0]);
		mem_iowr_byte(tx_buff_addr + 13, ethType[1]);
		//Data
		#pragma loopbound min 4 max 4
		for(int i=0; i<4; i++)
    {
      mem_iowr(tx_buff_addr + 18 + 0, ((schedtime_t) ctrl_to_aircraft_data[i] & 0xFFFFFFFF));
      mem_iowr(tx_buff_addr + 18 + 4, ((schedtime_t) ctrl_to_aircraft_data[i] >> 32));
		}
		eth_mac_send_nb(tx_buff_addr, 18+4+length);
		txMsgCount++;
    ans = 1;
	}
	GPIO &= (0U << SENDTASK_GPIO_BIT);
  return ans;
}

#pragma endregion CONTROL_NODE

#pragma region CYCLIC

__attribute__((noinline))
void rosace_init(MinimalTTTask *schedule)
{
	// Initial values
	outs.sig_outputs.Va = 0;
	outs.sig_outputs.Vz = 0;
	outs.sig_outputs.q  = 0;
	outs.sig_outputs.az = 0;
	outs.sig_outputs.h  = 0;
	outs.t_simu         = 0;
	step_simu           = 0;
  max_step_simu       = MAX_STEP_SIM;
  get_task_set(&num_of_tasks, &tasks);
  hyper_period = HYPER_PERIOD;
  num_of_tasks = NUM_OF_TASKS;
  printf("\nInitializing ROSACE Task Set (Hyper Period = %llu)\n", hyper_period);
  #pragma loopbound min 6 max 9
  for(unsigned i=0; i<num_of_tasks; i++)
  {
    schedule[i].id = LEDS = i;
    schedule[i].period = (schedtime_t) tasks_periods[i];
    schedule[i].nr_releases = tasks_insts_counts[i];
    schedule[i].release_times = tasks_schedules[i];
    schedule[i].last_time = 0;
    schedule[i].delta_sum = 0;
    schedule[i].exec_count = 0;
    schedule[i].release_inst = 0;
    printf("-- init_task_%d{T=%llu, RLS=%lu}{", schedule[i].id, schedule[i].period, schedule[i].nr_releases);
    for(unsigned j=0; j<schedule[i].nr_releases; j++)
    {
      printf("%llu, ", schedule[i].release_times[j]); 
    }
    puts("}");
  }
}

uint8_t cyclic_dispatcher(MinimalTTTask *schedule, schedtime_t current_time, schedtime_t start_time)
{
  #pragma loopbound min 6 max 9
  for(unsigned task=0; task < num_of_tasks; task++)
  {
    if(current_time >= schedule[task].release_times[schedule[task].release_inst])
    {
      switch (schedule[task].id)
      {
      case SYNC_ID:
        sync_fun(start_time, current_time, schedule);
        break;
      #ifdef AIRCRAFT_NODE
      case LOGGING_ID:
        logging_fun(NULL);
        break;
      case ENGINE_ID:
        CALL(ENGINE);
        break;
      case ELEVATOR_ID:
        CALL(ELEVATOR);
        break;
      case AIRCRAFT_DYN_ID:
        GPIO |= (1U << 3);
        CALL(AIRCRAFT_DYN);
        GPIO &= (0U << 3);
        break;
      case VL_CTRL_RECV_ID:
        vl_ctrl_recv_fun(NULL);
        break;
      case VL_DYN_SEND_ID:
        vl_dyn_send_fun(NULL);
        break;
      #elif FILTER_NODE
      case H_FILTER_ID:
        CALL(H_FILTER);
        break;
      case Q_FILTER_ID:
        CALL(Q_FILTER);
        break;
      case VZ_FILTER_ID:
        CALL(VZ_FILTER);
        break;
      case AZ_FILTER_ID:
        CALL(AZ_FILTER);
        break;
      case VA_FILTER_ID:
        CALL(VA_FILTER);
        break;
      case VL_DYN_RECV_ID:
        vl_dyn_recv_fun(NULL);
        break;
      case VL_FILTER_SEND_ID:
        vl_filter_send_fun(NULL);
        break;
      #elif CONTROL_NODE
      case H_C0_ID:
        CALL(H_C0);
        break;
      case VA_C0_ID:
        CALL(VA_C0);
        break;
      case VZ_CONTROL_ID:
        CALL(VZ_CONTROL);
        break;
      case VA_CONTROL_ID:
        CALL(VA_CONTROL);
        break;
      case ALTI_HOLD_ID:
        CALL(ALTI_HOLD);
        break;
      case VL_FILTER_RECV_ID:
        vl_filter_recv_fun(NULL);
        break;
      case VL_CTRL_SEND_ID:
        vl_ctrl_send_fun(NULL);
        break;
      #endif
      }
      schedule[task].release_times[schedule[task].release_inst] += hyper_period;
      schedule[task].release_inst = (schedule[task].release_inst + 1) % schedule[task].nr_releases;
      schedule[task].delta_sum += schedule[task].last_time == 0 ? 0 : 
                                (current_time - get_tte_aligned_time(schedule[task].last_time, schedule[task].period));
      schedule[task].last_time = current_time;
      schedule[task].exec_count += 1;
      return 1;
    }
  }
  return 0;
}

__attribute__((noinline))
schedtime_t execute_cyclic_loop(MinimalTTTask *schedule)
{
  ROSACE_update_altitude_command(10000);
  schedtime_t start_time = get_ptp_nanos(PATMOS_IO_ETH);
  #pragma loopbound min 1 max 1
  while(step_simu < max_step_simu)
  {
    register unsigned long long schedule_time = get_ptp_nanos(PATMOS_IO_ETH);
		schedule_time = (unsigned long long) get_tte_aligned_time(schedule_time - start_time, HYPER_PERIOD);
    cyclic_dispatcher(schedule, schedule_time, start_time);
    // if(step_simu >= 2500)
    // {
    //   ROSACE_update_altitude_command(11000);
    // }
	}
  return get_ptp_nanos(PATMOS_IO_ETH);
}

#pragma endregion CYCLIC

int main()
{
  MinimalTTTask schedule[NUM_OF_TASKS];
  LEDS = 0x1FF;
  printf("\nWelcome to TTE ROSACE execution\n");
  printSegmentInt(0xABCD0123);
  
  //MAC controller settings
	set_mac_address(0x1D000400, 0x00000289);
	eth_iowr(MODER, (RECSMALL_BIT | CRCEN_BIT | FULLD_BIT | PRO_BIT | TXEN_BIT | RXEN_BIT));
	eth_iowr(INT_SOURCE, INT_SOURCE_RXB_BIT);
  eth_iowr(RX_BD_ADDR_BASE(eth_iord(TX_BD_NUM)), RX_BD_EMPTY_BIT | RX_BD_IRQEN_BIT | RX_BD_WRAP_BIT);
  eth_iowr(INT_SOURCE, INT_SOURCE_TXB_BIT);
	eth_iowr(TX_BD_ADDR_BASE, TX_BD_READY_BIT | TX_BD_IRQEN_BIT | TX_BD_PAD_EN_BIT);
  init_sync();
  
  // Rosace Execution
  rosace_init((MinimalTTTask*) schedule);
  LEDS = 0x000;
  printf("\nRosace started @ %.3f us (max_sim_time = %llu us)\n", get_ptp_nanos(PATMOS_IO_ETH) * NS_TO_USEC, max_step_simu * STEP_TIME_SCALE * 1000);
  puts("\nT,Va,az,q,Vz,h,delta_th_c,delta_e_c");
  schedtime_t end_time = execute_cyclic_loop((MinimalTTTask*) schedule);
  
  // Reporting
  printf("Rosace ended @ %.3f us\n", end_time * NS_TO_USEC);
	puts("---------------------------------------------------------------------------");
	puts("Task log:");
	for(unsigned short task=0; task<num_of_tasks; task++){
		float avgDelta = (float) schedule[task].delta_sum/ (float) schedule[task].exec_count;
		printf("--task[%12s] avg. dt = %5.3f us (avg. jitter = %5.3f us) from a total of %lu executions\n", 
          tasks_names[task], avgDelta, fabs(schedule[task].period - avgDelta), schedule[task].exec_count);
	}
	puts("---------------------------------------------------------------------------");
  LEDS = 0x000;
  return 0;
}
