/*
  Copyright 2018 Technical University of Denmark, DTU Compute.
  All rights reserved.

  TTEthernet library

  Author: Maja Lund (maja_lala@hotmail.com)
*/

#include <stdio.h>
#include <stdlib.h>
#include <machine/exceptions.h>
#include <machine/rtc.h>
#include "eth_patmos_io.h"
#include "eth_mac_driver.h"

#define CYCLES_PER_UNIT 8000

unsigned long long send_times[2000];

unsigned char is_pcf(unsigned int addr);

void tte_clear_free_rx_buffer(unsigned int addr);// __attribute__((noinline));

void tte_wait_for_message(unsigned long long * receive_point);

unsigned char tte_receive(unsigned int addr,unsigned long long rec_start);// __attribute__((noinline));

unsigned char tte_receive_log(unsigned int addr,unsigned long long rec_start,signed long long error[],int i);// __attribute__((noinline));

int handle_integration_frame(unsigned int addr,unsigned long long rec_start);// __attribute__((noinline));

int handle_integration_frame_log(unsigned int addr,unsigned long long rec_start,
  signed long long error[],int i);// __attribute__((noinline));

unsigned char is_tte(unsigned int addr);

void tte_initialize(unsigned int int_period, unsigned int cl_period, unsigned char CT[], unsigned char VLcount,
  unsigned int max_delay, unsigned int comp_delay, unsigned int precision);

void tte_init_VL(unsigned char i, unsigned int start, unsigned int period);

unsigned long long transClk_to_clk (unsigned long long transClk);// __attribute__((noinline));

void tte_prepare_data(unsigned int tx_addr, unsigned char VL[], unsigned char data[], int length);// __attribute__((noinline));

void tte_prepare_pcf(unsigned int addr,unsigned char VL[],unsigned char type);

void tte_send_data(unsigned char i);// __attribute__((noinline));

void tte_start_ticking(char log_sending,char enable_int, void (int_handler)(void));

void tte_stop_ticking();

char tte_schedule_send(unsigned int addr,unsigned int size,unsigned char i);// __attribute__((noinline));

void tte_clock_tick(void);// __attribute__((noinline));

void tte_clock_tick_log(void);// __attribute__((noinline));
