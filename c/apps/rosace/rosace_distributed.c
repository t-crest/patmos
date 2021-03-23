#include <stdlib.h>
#include <inttypes.h>
#include <math.h> //just for fabs
#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/rtc.h>
#include "assemblage_includes.h"
#include "assemblage.h"
#include "rosace_dist_common.h"

#define ROSACE_PORT (uint16_t) 0x2B8

#ifdef AIRCRAFT_NODE
#include "rosace_aircraft_schedule.h"
unsigned char node_ip[4] = {192, 168, 1, 10};
unsigned char TTE_MAC[] = {0x00, 0x80, 0x6E, 0xF0, 0xDA, 0x42};
#elif FILTER_NODE
#include "rosace_filter_schedule.h"
unsigned char node_ip[4] = {192, 168, 1, 11};
unsigned char TTE_MAC[] = {0x00, 0x80, 0x6E, 0xF0, 0xDA, 0x43};
#elif CONTROL_NODE
#include "rosace_control_schedule.h"
unsigned char node_ip[4] = {192, 168, 1, 12};
unsigned char TTE_MAC[] = {0x00, 0x80, 0x6E, 0xF0, 0xDA, 0x44};
#endif

static uint16_t rxMsgCount;
static uint16_t txMsgCount;

char* eth_protocol_names[] = {"UFF", "IP", "ICMP", "UDP", "TCP", "PTP", "ARP", "LLDP", "MDNS", "TTE_PCF", "TTE"};
unsigned char multicastip[4] = {224, 0, 0, 255};
unsigned char TTE_CT[] = { 0xAB, 0xAD, 0xBA, 0xBE };
unsigned char TTE_DYN_VL[] = { 0x0F, 0xA1 };
unsigned char TTE_FILTER_VL[] = { 0x0F, 0xA2 };
unsigned char TTE_CTRL_VL[] = { 0x0F, 0xA3 };

uint8_t enable_communication = 0;
uint8_t enable_control = 0;

unsigned __USE_HWFPU__ = 0;

#pragma region AIRCRAFT_NODE

__attribute__((noinline))
int vl_dyn_send_filter_fun(void *args)
{
  GPIO |= (1U << SENDTASK_GPIO_BIT);
  int ans = 0;
  if(nodeSyncStable)
  {
    aircraft_state_message dynamics_msg = {
      step_simu,
      aircraft_dynamics495_Va_Va_filter_100449_Va[step_simu % 2],
      aircraft_dynamics495_az_az_filter_100458_az[step_simu % 2],
      aircraft_dynamics495_Vz_Vz_filter_100452_Vz[step_simu % 2],
      aircraft_dynamics495_q_q_filter_100455_q[step_simu % 2],
      aircraft_dynamics495_h_h_filter_100446_h[step_simu % 2]
    };
    udp_send_tte(tx_buff_addr, TTE_CT, TTE_DYN_VL, TTE_MAC, (unsigned char[4]){192, 168, 1, 11}, node_ip, ROSACE_PORT, ROSACE_PORT, (unsigned char*) &dynamics_msg, sizeof(dynamics_msg), txMsgCount);
    txMsgCount++;
    step_simu += 1;
    ans = 1;
  }
  GPIO &= (0U << SENDTASK_GPIO_BIT);
  return ans;
}

__attribute__((noinline))
int vl_dyn_recv_ctrl_fun(void* args)
{
  GPIO |= (1U << RECVTASK_GPIO_BIT);
  LEDS &= 0xF0;
  int ans = 0;
  int ethType = eth_mac_poll_for_frames();
  if(ethType != TIMEOUT)
  {
    if(ethType == 0x0800)
    {
      control_state_message control_msg;
      udp_get_data(rx_buff_addr, (unsigned char*) &control_msg, udp_get_data_length(rx_buff_addr));
      Vz_control_50483_delta_e_c_elevator489_delta_e_c = control_msg.vz_control_elevator_delta_e_c;
      Vz_control_50483_delta_e_c_delta_e_c = control_msg.vz_control_delta_e_c;
      Va_control_50474_delta_th_c_engine486_delta_th_c = control_msg.va_control_engine_delta_th_c;
      Va_control_50474_delta_th_c_delta_th_c = control_msg.va_control_delta_th_c;
      rxMsgCount++;
      ans = 1;
      LEDS |= (1U << 4) + (ethType & 0xF);
    } 
    else 
    {
      LEDS |= (1U << 5) + (ethType & 0xF);
    }
    swap_eth_rx_buffers();  
  }
  GPIO &= (0U << RECVTASK_GPIO_BIT);
  return ans;
}

#pragma endregion AIRCRAFT_NODE

#pragma region FILTER_NODE

__attribute__((noinline))
int vl_filter_send_ctrl_fun(void *args)
{
  GPIO |= (1U << SENDTASK_GPIO_BIT);
  int ans = 0;
  if(1)
  {
    filter_state_message filter_msg = {
      step_simu,
      h_filter_100446_h_f_altitude_hold_50464_h_f[step_simu % 2],
      q_filter_100455_q_f_Va_control_50474_q_f[step_simu % 2],
      q_filter_100455_q_f_Vz_control_50483_q_f[step_simu % 2],
      az_filter_100458_az_f_Vz_control_50483_az_f[step_simu % 2],
      Vz_filter_100452_Vz_f_Va_control_50474_Vz_f[step_simu % 2],
      Va_filter_100449_Va_f_Va_control_50474_Va_f[step_simu % 2],
      Vz_filter_100452_Vz_f_Vz_control_50483_Vz_f[step_simu % 2]
    };
    udp_send_tte(tx_buff_addr, TTE_CT, TTE_FILTER_VL, TTE_MAC, (unsigned char[4]){192, 168, 1, 12}, node_ip, ROSACE_PORT, ROSACE_PORT, (unsigned char*) &filter_msg, sizeof(filter_msg), txMsgCount);
    txMsgCount++;
    ans = 1;
  }
  GPIO &= (0U << SENDTASK_GPIO_BIT);
  return ans;
}

 __attribute__((noinline))
int vl_filter_recv_dyn_fun(void *args)
{
  GPIO |= (1U << RECVTASK_GPIO_BIT);
  LEDS &= 0xF0;
  int ans = 0;
  int ethType = eth_mac_poll_for_frames();
  if(ethType != TIMEOUT)
  {
    if(ethType == 0x0800)
    {
      //Data
      aircraft_state_message dynamics_msg;
      udp_get_data(rx_buff_addr, (unsigned char*) &dynamics_msg, udp_get_data_length(rx_buff_addr));
      step_simu = dynamics_msg.step;
      aircraft_dynamics495_Va_Va_filter_100449_Va[step_simu % 2] = dynamics_msg.dynamics_va_filter_va;
      aircraft_dynamics495_az_az_filter_100458_az[step_simu % 2] = dynamics_msg.dynamics_az_filter_az;
      aircraft_dynamics495_Vz_Vz_filter_100452_Vz[step_simu % 2] = dynamics_msg.dynamics_vz_filter_vz;
      aircraft_dynamics495_q_q_filter_100455_q[step_simu % 2] = dynamics_msg.dynamics_q_filter_q;
      aircraft_dynamics495_h_h_filter_100446_h[step_simu % 2] = dynamics_msg.dynamics_h_filter_h;
      enable_control = step_simu > 0;
      rxMsgCount++;
      LEDS |= (1U << 4) + (ethType & 0xF);
    } 
    else 
    {
      LEDS |= (1U << 5) + (ethType & 0xF);
    }
    swap_eth_rx_buffers();
  }
  GPIO &= (0U << RECVTASK_GPIO_BIT);
  return ans;
}

#pragma endregion FILTER_NODE

#pragma region CONTROL_NODE

__attribute__((noinline))
int vl_ctrl_send_dyn_fun(void *args)
{
  GPIO |= (1U << SENDTASK_GPIO_BIT);
  int ans = 0;
  if(1)
  {
    control_state_message control_msg = {
      step_simu,
      Vz_control_50483_delta_e_c_elevator489_delta_e_c,
      Vz_control_50483_delta_e_c_delta_e_c,
      Va_control_50474_delta_th_c_engine486_delta_th_c,
      Va_control_50474_delta_th_c_delta_th_c
    };
    udp_send_tte(tx_buff_addr, TTE_CT, TTE_CTRL_VL, TTE_MAC, (unsigned char[4]){192, 168, 1, 10}, node_ip, ROSACE_PORT, ROSACE_PORT, (unsigned char*) &control_msg, sizeof(control_msg), txMsgCount);
    txMsgCount++;
    ans = 1;
  }
  GPIO &= (0U << SENDTASK_GPIO_BIT);
  return ans;
}

__attribute__((noinline))
int vl_ctrl_recv_filter_fun(void *args)
{
  GPIO |= (1U << RECVTASK_GPIO_BIT);
  LEDS &= 0xF0;
  int ans = 0;
  int ethType = eth_mac_poll_for_frames();
  if(ethType != TIMEOUT)
  {
    if(ethType == 0x0800)
    {
      //Data
      filter_state_message filter_msg;
      udp_get_data(rx_buff_addr, (unsigned char*) &filter_msg, udp_get_data_length(rx_buff_addr));
      step_simu = filter_msg.step;
      h_filter_100446_h_f_altitude_hold_50464_h_f[step_simu % 2] = filter_msg.h_filter_alt_hold_h_f;
      q_filter_100455_q_f_Va_control_50474_q_f[step_simu % 2] = filter_msg.q_filter_va_control_q_f;
      q_filter_100455_q_f_Vz_control_50483_q_f[step_simu % 2] = filter_msg.q_filter_vz_control_q_f;
      az_filter_100458_az_f_Vz_control_50483_az_f[step_simu % 2] = filter_msg.az_filter_vz_control_az_f;
      Vz_filter_100452_Vz_f_Va_control_50474_Vz_f[step_simu % 2] = filter_msg.vz_filter_va_control_vz_f;
      Vz_filter_100452_Vz_f_Vz_control_50483_Vz_f[step_simu % 2] = filter_msg.vz_filter_vz_control_vz_f;
      Va_filter_100449_Va_f_Va_control_50474_Va_f[step_simu % 2] = filter_msg.va_filter_va_control_va_f;
      enable_control = step_simu > 0;
      rxMsgCount++;
      LEDS |= (1U << 4) + (ethType & 0xF);
    } 
    else 
    {
      LEDS |= (1U << 5) + (ethType & 0xF);
    }
    swap_eth_rx_buffers();
  }
  GPIO &= (0U << RECVTASK_GPIO_BIT);
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
  outs.sig_delta_e_c = 0;
  outs.sig_delta_th_c = 0;
  outs.t_simu         = 0;
  step_simu           = 0;
  max_step_simu       = MAX_STEP_SIM;
  get_task_set(&num_of_tasks, &tasks);
  hyper_period = HYPER_PERIOD;
  num_of_tasks = NUM_OF_TASKS;
  printf("\nLoading ROSACE Task Set (Hyper Period = %llu)\n", hyper_period);
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
    printf("-- init %s_task {T=%llu, RLS=%lu}{", tasks_names[i], schedule[i].period, schedule[i].nr_releases);
    for(unsigned j=0; j<schedule[i].nr_releases; j++)
    {
      printf("%llu, ", schedule[i].release_times[j]); 
    }
    puts("}");
  }
}

__attribute__((noinline))
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
        if (nodeSyncStable && enable_communication) logging_fun(NULL);
        break;
      case ENGINE_ID:
        if (nodeSyncStable && enable_communication) CALL(ENGINE);
        break;
      case ELEVATOR_ID:
        if (nodeSyncStable && enable_communication) CALL(ELEVATOR);
        break;
      case AIRCRAFT_DYN_ID:
        GPIO |= (1U << SCOPE_GPIO_BIT);
        if (nodeSyncStable && enable_communication) CALL(AIRCRAFT_DYN);
        GPIO &= (0U << SCOPE_GPIO_BIT);
        break;
      case VL_CTRL_RECV_ID:
        vl_dyn_recv_ctrl_fun(NULL);
        break;
      case VL_DYN_SEND_ID:
        if (nodeSyncStable && enable_communication) vl_dyn_send_filter_fun(NULL);
        break;
      #elif FILTER_NODE
      case H_FILTER_ID:
        if (nodeSyncStable && enable_control) CALL(H_FILTER);
        break;
      case Q_FILTER_ID:
        if (nodeSyncStable && enable_control) CALL(Q_FILTER);
        break;
      case VZ_FILTER_ID:
        if (nodeSyncStable && enable_control) CALL(VZ_FILTER);
        break;
      case AZ_FILTER_ID:
        if (nodeSyncStable && enable_control) CALL(AZ_FILTER);
        break;
      case VA_FILTER_ID:
        if (nodeSyncStable && enable_control) CALL(VA_FILTER);
        break;
      case VL_DYN_RECV_ID:
        vl_filter_recv_dyn_fun(NULL);
        break;
      case VL_FILTER_SEND_ID:
        if (nodeSyncStable && enable_communication) vl_filter_send_ctrl_fun(NULL);
        break;
      #elif CONTROL_NODE
      case H_C0_ID:
        if (nodeSyncStable && enable_control) CALL(H_C0);
        break;
      case VA_C0_ID:
        if (nodeSyncStable && enable_control) CALL(VA_C0);
        break;
      case VZ_CONTROL_ID:
        if (nodeSyncStable && enable_control) CALL(VZ_CONTROL);
        break;
      case VA_CONTROL_ID:
        if (nodeSyncStable && enable_control) CALL(VA_CONTROL);
        break;
      case ALTI_HOLD_ID:
        if (nodeSyncStable && enable_control) CALL(ALTI_HOLD);
        break;
      case VL_FILTER_RECV_ID:
        vl_ctrl_recv_filter_fun(NULL);
        break;
      case VL_CTRL_SEND_ID:
        if (nodeSyncStable && enable_communication) vl_ctrl_send_dyn_fun(NULL);
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
  eth_mac_clear_rx_buffer(rx_buff_addr, rx_bd_addr);
  eth_mac_clear_rx_buffer(rx_buff2_addr, rx_bd2_addr);
  #pragma loopbound min 1 max 1
  while(step_simu < max_step_simu)
  {
    register unsigned long long schedule_time = get_ptp_nanos(PATMOS_IO_ETH);
    schedule_time = (unsigned long long) get_tte_aligned_time(schedule_time - start_time, HYPER_PERIOD);
    cyclic_dispatcher(schedule, schedule_time, start_time);
#ifdef CONTROL_NODE
    if(step_simu >= ALT_COMMAND_STEPSIM)  //50 seconds
    {
      ROSACE_update_altitude_command(11000);
    }
#endif
    if(KEYS == 0xE)
      enable_communication = 0x1;
    else if (KEYS == 0xD)
      enable_communication = 0x0; 
  }
  return get_ptp_nanos(PATMOS_IO_ETH);
}

#pragma endregion CYCLIC

int main()
{
  MinimalTTTask schedule[NUM_OF_TASKS];
  LEDS = 0x1FF;
  printSegmentInt(0xABCD0123);
  printf("\nWelcome to TTE ROSACE execution\n");
  
  
  //MAC controller settings
  config_ethmac();
  reset_sync();
  rosace_init((MinimalTTTask*) schedule);
  
  // Rosace Execution
  LEDS = 0x000;
  printf("\nRosace started @ %.3f us (max_sim_time = %lu us)\n", get_ptp_nanos(PATMOS_IO_ETH) * NS_TO_USEC, max_step_simu * STEP_TIME_SCALE * MS_TO_USEC);
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
