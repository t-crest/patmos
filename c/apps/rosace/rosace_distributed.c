#include <stdlib.h>
#include <inttypes.h>
#include <math.h> //just for fabs
#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/rtc.h>
#include "assemblage_includes.h"
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

aircraft_state_message aircraft_msg;
filter_state_message filter_msg;
control_state_message control_msg;

static uint16_t rxMsgCount;
static uint16_t txMsgCount;

char* eth_protocol_names[] = {"UFF", "IP", "ICMP", "UDP", "TCP", "PTP", "ARP", "LLDP", "MDNS", "TTE_PCF", "TTE"};
unsigned char multicastip[4] = {224, 0, 0, 255};
unsigned char TTE_CT[] = { 0xAB, 0xAD, 0xBA, 0xBE };
unsigned char TTE_DYN_VL[] = { 0x0F, 0xA1 };
unsigned char TTE_FILTER_VL[] = { 0x0F, 0xA2 };
unsigned char TTE_CTRL_VL[] = { 0x0F, 0xA3 };

uint8_t enable_communication = 0;

unsigned __USE_HWFPU__ = 0;

#pragma region AIRCRAFT_NODE

int logging_fun()
{
  printf("%3.2f,%5.3f,%5.3f,%5.4f,%5.3f,%5.3f,%5.4f,%5.4f\n", 
  (aircraft_msg.step * STEP_TIME_SCALE)/1000.0f, aircraft_msg.dynamics.Va, aircraft_msg.dynamics.az, 
  aircraft_msg.dynamics.q, aircraft_msg.dynamics.Vz, aircraft_msg.dynamics.h,
  control_msg.delta_th_c, control_msg.delta_e_c);
  return 1;
}

__attribute__((noinline))
int vl_dyn_send_filter_fun()
{
  GPIO |= (1U << SENDTASK_GPIO_BIT);
  int ans = 0;
  if(nodeSyncStable)
  {
    aircraft_msg.enable_filter = enable_communication;
    udp_send_tte(tx_buff_addr, TTE_CT, TTE_DYN_VL, TTE_MAC, (unsigned char[4]){192, 168, 1, 11}, node_ip, ROSACE_PORT, ROSACE_PORT, (unsigned char*) &aircraft_msg, sizeof(aircraft_msg), txMsgCount);
    txMsgCount++;
    aircraft_msg.step += 1;
    ans = 1;
  }
  GPIO &= (0U << SENDTASK_GPIO_BIT);
  return ans;
}

__attribute__((noinline))
int vl_dyn_recv_ctrl_fun()
{
  GPIO |= (1U << RECVTASK_GPIO_BIT);
  LEDS &= 0xF0;
  int ans = 0;
  int ethType = eth_mac_poll_for_frames();
  if(ethType != TIMEOUT)
  {
    if(ethType == 0x0800)
    {
      udp_get_data(rx_buff_addr, (unsigned char*) &control_msg, udp_get_data_length(rx_buff_addr));
      rxMsgCount++;
      ans = 1;
      LEDS |= (control_msg.controlling << 8) + (1U << 4) + (ethType & 0xF);
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
int vl_filter_send_ctrl_fun()
{
  GPIO |= (1U << SENDTASK_GPIO_BIT);
  int ans = 0;
  udp_send_tte(tx_buff_addr, TTE_CT, TTE_FILTER_VL, TTE_MAC, (unsigned char[4]){192, 168, 1, 12}, node_ip, ROSACE_PORT, ROSACE_PORT, (unsigned char*) &filter_msg, sizeof(filter_msg), txMsgCount);
  txMsgCount++;
  ans = 1;
  GPIO &= (0U << SENDTASK_GPIO_BIT);
  return ans;
}

 __attribute__((noinline))
int vl_filter_recv_dyn_fun()
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
      udp_get_data(rx_buff_addr, (unsigned char*) &aircraft_msg, udp_get_data_length(rx_buff_addr));
      filter_msg.enable_control = aircraft_msg.enable_filter;
      filter_msg.step = aircraft_msg.step;
      rxMsgCount++;
      LEDS |= (filter_msg.enable_control << 8) + (1U << 4) + (ethType & 0xF);
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
int vl_ctrl_send_dyn_fun()
{
  GPIO |= (1U << SENDTASK_GPIO_BIT);
  int ans = 0;
  udp_send_tte(tx_buff_addr, TTE_CT, TTE_CTRL_VL, TTE_MAC, (unsigned char[4]){192, 168, 1, 10}, node_ip, ROSACE_PORT, ROSACE_PORT, (unsigned char*) &control_msg, sizeof(control_msg), txMsgCount);
  txMsgCount++;
  ans = 1;
  GPIO &= (0U << SENDTASK_GPIO_BIT);
  return ans;
}

__attribute__((noinline))
int vl_ctrl_recv_filter_fun()
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
      udp_get_data(rx_buff_addr, (unsigned char*) &filter_msg, udp_get_data_length(rx_buff_addr));
      control_msg.controlling = filter_msg.enable_control;
      rxMsgCount++;
      LEDS |= (filter_msg.enable_control << 8) + (1U << 4) + (ethType & 0xF);
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
  aircraft_msg.dynamics.Va = 0;
  aircraft_msg.dynamics.Vz = 0;
  aircraft_msg.dynamics.q  = 0;
  aircraft_msg.dynamics.az = 0;
  aircraft_msg.dynamics.h  = 0;
  aircraft_msg.engine_dynamics_T = 41813.9211946;
  aircraft_msg.elevator_dynamics_delta_e = 0.0120096156525;
  aircraft_msg.step          = 0;
  aircraft_msg.enable_filter = 0;
  filter_msg.enable_control = 0;
  control_msg.h_c = 10000;
  control_msg.Va_c = 0.0;
  control_msg.Vz_c = -2.5;
  control_msg.delta_e_c    = -0.05;
  control_msg.delta_th_c   = 1.0;
  max_step_simu       = MAX_STEP_SIM;
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
    schedule[i].exec_time = 0;
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
      schedtime_t act_time = get_ptp_nanos(PATMOS_IO_ETH);
      switch (schedule[task].id)
      {
      case SYNC_ID:
        sync_fun(start_time, current_time, schedule);
        break;
      #ifdef AIRCRAFT_NODE
      case LOGGING_ID:
        if (nodeSyncStable && enable_communication) 
          logging_fun();
        break;
      case ENGINE_ID:
        if (nodeSyncStable && enable_communication) 
          aircraft_msg.engine_dynamics_T = engine(control_msg.delta_th_c);
        break;
      case ELEVATOR_ID:
        if (nodeSyncStable && enable_communication) 
          aircraft_msg.elevator_dynamics_delta_e = elevator(control_msg.delta_e_c);
        break;
      case AIRCRAFT_DYN_ID:
        GPIO |= (1U << SCOPE_GPIO_BIT);
        if (nodeSyncStable && enable_communication) 
          aircraft_dynamics(aircraft_msg.elevator_dynamics_delta_e, aircraft_msg.engine_dynamics_T, &aircraft_msg.dynamics);
        GPIO &= (0U << SCOPE_GPIO_BIT);
        break;
      case VL_CTRL_RECV_ID:
        vl_dyn_recv_ctrl_fun();
        break;
      case VL_DYN_SEND_ID:
        if (nodeSyncStable && enable_communication) 
          vl_dyn_send_filter_fun();
        break;
      #elif FILTER_NODE
      case H_FILTER_ID:
        if (nodeSyncStable && aircraft_msg.enable_filter) 
          filter_msg.h_meas = h_filter_25(aircraft_msg.dynamics.h);
        break;
      case Q_FILTER_ID:
        if (nodeSyncStable && aircraft_msg.enable_filter)
          filter_msg.q_meas = q_filter_25(aircraft_msg.dynamics.q);
        break;
      case VZ_FILTER_ID:
        if (nodeSyncStable && aircraft_msg.enable_filter) 
          filter_msg.vz_meas = Vz_filter_25(aircraft_msg.dynamics.Vz);
        break;
      case AZ_FILTER_ID:
        if (nodeSyncStable && aircraft_msg.enable_filter) 
          filter_msg.az_meas = az_filter_25(aircraft_msg.dynamics.az);
        break;
      case VA_FILTER_ID:
        if (nodeSyncStable && aircraft_msg.enable_filter) 
          filter_msg.va_meas = Va_filter_25(aircraft_msg.dynamics.Va);
        break;
      case VL_DYN_RECV_ID:
        vl_filter_recv_dyn_fun();
        break;
      case VL_FILTER_SEND_ID:
        if (nodeSyncStable && enable_communication) 
          vl_filter_send_ctrl_fun();
        break;
      #elif CONTROL_NODE
      case H_C0_ID:
        if (nodeSyncStable && filter_msg.enable_control)
        {
          if(filter_msg.step >= ALT_COMMAND_STEPSIM)  //50 seconds
          {
            control_msg.h_c = 11000;
          } else {
            control_msg.h_c = 10000;
          }
        }
        break;
      case VA_C0_ID:
        if (nodeSyncStable && filter_msg.enable_control)
        break;
      case VZ_CONTROL_ID:
        if (nodeSyncStable && filter_msg.enable_control)
          control_msg.delta_e_c = Vz_control_12(filter_msg.vz_meas, control_msg.Vz_c, filter_msg.q_meas, filter_msg.az_meas);
        break;
      case VA_CONTROL_ID:
        if (nodeSyncStable && filter_msg.enable_control)
          control_msg.delta_th_c = Va_control_12(filter_msg.va_meas, filter_msg.vz_meas, filter_msg.q_meas, control_msg.Va_c);
        break;
      case ALTI_HOLD_ID:
        if (nodeSyncStable && filter_msg.enable_control) 
          control_msg.Vz_c = altitude_hold_12(filter_msg.h_meas, control_msg.h_c);
        break;
      case VL_FILTER_RECV_ID:
        vl_ctrl_recv_filter_fun();
        break;
      case VL_CTRL_SEND_ID:
        if (nodeSyncStable && enable_communication) vl_ctrl_send_dyn_fun();
        break;
      #endif
      }
      schedule[task].release_times[schedule[task].release_inst] += hyper_period;
      schedule[task].release_inst = (schedule[task].release_inst + 1) % schedule[task].nr_releases;
      schedule[task].delta_sum += schedule[task].last_time == 0 ? 0 : 
                                (current_time - get_tte_aligned_time(schedule[task].last_time, schedule[task].period));
      schedule[task].last_time = current_time;
      schedule[task].exec_count += 1;
      schedtime_t act_delta = get_ptp_nanos(PATMOS_IO_ETH) - act_time;
      schedule[task].exec_time = schedule[task].exec_time < act_delta ? act_delta : schedule[task].exec_time;
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
#ifdef AIRCRAFT_NODE
  while(aircraft_msg.step < max_step_simu)
  {
#elif FILTER_NODE
  while(filter_msg.step < max_step_simu-1)
  {
#elif CONTROL_NODE
  while(control_msg.step < max_step_simu-1)
  {
#endif
    register unsigned long long schedule_time = get_ptp_nanos(PATMOS_IO_ETH);
    schedule_time = (unsigned long long) get_tte_aligned_time(schedule_time - start_time, HYPER_PERIOD);
    cyclic_dispatcher(schedule, schedule_time, start_time);
    if(KEYS == 0xE)
      enable_communication = 0x1;
    else if (KEYS == 0xD)
      enable_communication = 0x0; 
    else if (KEYS == 0xB)
      return get_ptp_nanos(PATMOS_IO_ETH);
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
    printf("--task[%12s] avg. dt = %5.3f us (max. et = %5.3f, avg. jitter = %5.3f us) from a total of %lu executions\n", 
          tasks_names[task], avgDelta * NS_TO_USEC, schedule[task].exec_time * NS_TO_USEC, 
          fabs(schedule[task].period - avgDelta) * NS_TO_USEC, schedule[task].exec_count);
  }
  puts("---------------------------------------------------------------------------");
  LEDS = 0x000;
  return 0;
}
