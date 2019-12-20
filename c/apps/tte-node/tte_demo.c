/*
  Copyright 2018 Technical University of Denmark, DTU Compute.
  All rights reserved.

  TTEthernet test program

  Author: Maja Lund (maja_lala@hotmail.com)
*/

#include "ethlib/eth_mac_driver.h"
#include "ethlib/tte.h"
#include <machine/patmos.h>
#include <stdio.h>
#include <stdlib.h>

#define NS_TO_SEC 0.000000001
#define NS_TO_USEC 0.001
#define USEC_TO_NS 1000
#define USEC_TO_SEC 0.000001
#define SEC_TO_NS 1000000000
#define SEC_TO_USEC 1000000
#define SEC_TO_HOUR 0.000277777778

#define MS_TO_MAJA 10 //1 ms to tenths of us

#define INT_PERIOD 10 //ms
#define CYC_PERIOD 100 //ms
#define TTE_MAX_TRANS_DELAY 10848 //clock cycles from net_config (eclipse project)
#define TTE_COMP_DELAY 4000
#define TTE_PRECISION 800 //clock cycles from network_description (eclipse project)

#define N 10000 / INT_PERIOD

volatile _IODEV int *led_ptr = (volatile _IODEV int *) PATMOS_IO_LED;
volatile _IODEV unsigned* disp_ptr = (volatile _IODEV unsigned*)PATMOS_IO_SEGDISP;

int i = 0;
signed long long error[N]; // for logging clock precision
unsigned long long r_pit[N];
static unsigned long long receive_point; //clock cycles at 80MHz (12.5 ns)

unsigned char CT[] = { 0xAB, 0xAD, 0xBA, 0xBE };
unsigned char VL0[] = { 0x0F, 0xA1 };
unsigned char VL1[] = { 0x0F, 0xA2 };

int sched_errors = 0;
int tte = 0;
int eth = 0;

unsigned char reply;
unsigned int cur_RX = 0x000;
unsigned int ext_RX = 0x800;
unsigned int cur_RX_BD = 0x600;
unsigned int ext_RX_BD = 0x608;


void printSegmentInt(unsigned number)
{
  *(disp_ptr + 0) = number & 0xF;
  *(disp_ptr + 1) = (number >> 4) & 0xF;
  *(disp_ptr + 2) = (number >> 8) & 0xF;
  *(disp_ptr + 3) = (number >> 12) & 0xF;
  *(disp_ptr + 4) = (number >> 16) & 0xF;
  *(disp_ptr + 5) = (number >> 20) & 0xF;
  *(disp_ptr + 6) = (number >> 24) & 0xF;
  *(disp_ptr + 7) = (number >> 28) & 0xF;
}

__attribute__((noinline))
void run(){
  set_mac_address(0x1D000400, 0x00000289);
  // int_period = 10ms, cluster cycle=20ms, CT, 2 VLs sending, max_delay , comp_delay, precision (in clock cycles)
  tte_initialize(INT_PERIOD*MS_TO_MAJA, CYC_PERIOD*MS_TO_MAJA, CT, 2, TTE_MAX_TRANS_DELAY, TTE_COMP_DELAY, TTE_PRECISION);
  tte_init_VL(0, 8, 40);  // VL 4001 starts at 0.8ms and has a period of 4ms
  tte_init_VL(1, 10, 20); // VL 4002 starts at 1ms and has a period of 2ms
  // tte_start_ticking(0, 0, 0);
  eth_iowr(0x04, 0x00000004); // clear receive frame bit in int_source
  eth_iowr(cur_RX_BD + 4,
           cur_RX); // set first receive buffer to store frame in 0x000
  eth_iowr(cur_RX_BD, 0x0000C000); // set empty and IRQ and not wrap
  eth_iowr(ext_RX_BD + 4,
           ext_RX); // set second receive buffer to store frame in 0x800
  eth_iowr(ext_RX_BD, 0x00006000); // set NOT empty, IRQ and wrap

  for (i = 0; i < N;) {
    *led_ptr = 0x1;
    tte_wait_for_message(&receive_point);
    *led_ptr = 0x2;
    tte_clear_free_rx_buffer(ext_RX_BD); // start receiving in other buffer
    *led_ptr = 0x4;
    reply=tte_receive_log(cur_RX, receive_point, error, i);
    switch(reply){
      case 0: // failed pcf, stop test
        *led_ptr = 0x70;
        printf("pcf out of schedule \n");
        // n = i + 1;
        break;
      case 1: // successfull pcf
        *led_ptr = 0x10;
        r_pit[i] = receive_point;
        // printf("#%d\t0.000\t%.3f\n", i, (int) error[i]);
        printSegmentInt((unsigned) abs(error[i]));
        i++;
        break;
      case 2: // incoming tte
        *led_ptr = 0x20;
        tte++;
        reply = mem_iord_byte(cur_RX + 14);
        tte_prepare_data(0x2000, VL0, (unsigned char[]) {reply}, 1514);
        if (!tte_schedule_send(0x2000, 1514, 0))
          sched_errors++;  
      default:  // incoming ethernet msg
        *led_ptr = 0x40;
        eth++;
    } 
    // if(*led_ptr == 0x70) break;
    unsigned int extra = cur_RX;
    cur_RX = ext_RX;
    ext_RX = extra;
    extra = cur_RX_BD;
    cur_RX_BD = ext_RX_BD;
    ext_RX_BD = extra;
  }
}

int main()
{
  puts("TTE-node demo started");
  *led_ptr = 0x1FF;

  run();
  
  // tte_stop_ticking();
  *led_ptr = 0x1FF;
  for (int i = 0; i < N; i++) { // logging
        printf("#%d\t0.000\t%lld\n", i, error[i]);
  }
  puts("$");  
  printf("tte_receive_log = %llu clocks\n", tte_receive_log_max_time);
  printf("handle_integration_frame_log = %llu clocks\n", handle_integration_frame_log_max_time);
  printf("runs: %d\n\r", i);
  printf("sched errors: %d\n\r", sched_errors);
  printf("received tte: %d\n\r", tte);
  printf("received eth: %d\n\r", eth);
  printf("TTE-node demo exiting...\n\r");
  return 0;
}