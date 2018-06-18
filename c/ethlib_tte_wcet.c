/*
   Copyright 2014 Technical University of Denmark, DTU Compute. 
   All rights reserved.
   
   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

/* 
 * file that calls TTE functions for WCET analysis
 * 
 * Authors: Maja Lund
 */

#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include "ethlib/tte.h"

#define N 2000

unsigned char VL0[] = {0x0F,0xA1};
unsigned char VL1[] = {0x0F,0xA2};

//copied from rtc.h
static unsigned long long get_cpu_cycles_cp(void) __attribute__((noinline));
static unsigned long long get_cpu_cycles_cp(void) {
  unsigned clo, chi;

  // TODO this code is identical to libgloss/patmos/time.c, share code.

  // Prevent the compiler from moving the read over other instructions 
  // or into a call delay slot behind the call miss stall
  asm volatile ("" : : : "memory");

  _iodev_ptr_t hi_clock = (_iodev_ptr_t)(__PATMOS_TIMER_HICLK);
  _iodev_ptr_t lo_clock = (_iodev_ptr_t)(__PATMOS_TIMER_LOCLK);

  // Order is important here
  clo = *lo_clock;
  chi = *hi_clock;

  asm volatile ("" : : : "memory");

  return (((unsigned long long) chi) << 32) | clo;
}

int tte_loop(char reply,unsigned int rx_addr,int i) __attribute__((noinline));
int tte_loop(char reply,unsigned int rx_addr,int i){
  int j=i;
  if(reply==0){ //failed pcf
    //printf("pcf out of schedule \n");
    j=N;
  }
  else if(reply==1){ //successfull pcf
    if(i>10){
      if(i%2==0){ //send the 10 allowed packets on VL1
	tte_prepare_test_data(0x2600,VL1,0x11,400);
	tte_schedule_send(0x2600,400,1);
	tte_prepare_test_data(0x2C00,VL1,0x22,300);
	tte_schedule_send(0x2C00,300,1);
	tte_prepare_test_data(0x3200,VL1,0x33,800);
	tte_schedule_send(0x3200,800,1);
	tte_prepare_test_data(0x3800,VL1,0x44,1514);
	tte_schedule_send(0x3800,1514,1);
	tte_prepare_test_data(0x3E00,VL1,0x55,400);
	tte_schedule_send(0x3E00,400,1);
	tte_prepare_test_data(0x4400,VL1,0x66,300);
	tte_schedule_send(0x4400,300,1);
	tte_prepare_test_data(0x4A00,VL1,0x77,800);
	tte_schedule_send(0x4A00,800,1);
        tte_prepare_test_data(0x5000,VL1,0x88,1514);
	tte_schedule_send(0x5000,1514,1);
	tte_prepare_test_data(0x5600,VL1,0x99,400);
	tte_schedule_send(0x5600,400,1);
	tte_prepare_test_data(0x5C00,VL1,0x10,300);
	tte_schedule_send(0x5C00,300,1);
      }
    }
    j++;
  } else if (reply==2){ //incoming tte
    reply=mem_iord_byte(rx_addr+14);
    tte_prepare_test_data(0x800,VL0,reply,200);
    tte_schedule_send(0x800,200,0);
  }
  return j;
}

int main(){
  signed long long error[N];  //for logging
  static unsigned long long receive_point;

  unsigned char CT[] = {0xAB,0xAD,0xBA,0xBE};
  unsigned char reply;
  unsigned int cur_RX = 0x000;
  unsigned int ext_RX = 0x800;
  unsigned int cur_RX_BD = 0x600;
  unsigned int ext_RX_BD = 0x608;

  tte_initialize(0xC3500,200,CT,2); //0xC3500 = 10ms in clock cycles, cluster cycle is 20ms, CT, 2 virtual links
  tte_init_VL(0, 26,40); //VL 4001 starts at 2.6ms and has a period of 4ms
  tte_init_VL(1, 10,20); //VL 4002 starts at 1ms and has a period of 2ms
  tte_start_ticking(0,0);
  eth_iowr(0x04, 0x00000004); //clear receive frame bit in int_source
  eth_iowr(cur_RX_BD+4, cur_RX); //set first receive buffer to store frame in 0x000
  eth_iowr(cur_RX_BD, 0x0000C000); //set empty and IRQ and not wrap
  eth_iowr(ext_RX_BD+4, ext_RX); //set second receive buffer to store frame in 0x800
  eth_iowr(ext_RX_BD, 0x00006000); //set NOT empty, IRQ and wrap
  
  for (int i =0; i<N;){
    while ((eth_iord(0x04) & 0x4)==0){;};
    receive_point = get_cpu_cycles_cp();
    tte_clear_free_rx_buffer(ext_RX_BD); //enable receiving in other buffer

    reply=tte_receive(cur_RX,receive_point);
    reply=tte_receive_log(cur_RX,receive_point,error,i); //for logging

    i=tte_loop(reply,cur_RX,i);
    tte_clock_tick(); //usually called by interrupt, but WCET analysis does not recognize this

    unsigned int extra = cur_RX;
    cur_RX = ext_RX;
    ext_RX = extra;
    extra = cur_RX_BD;
    cur_RX_BD = ext_RX_BD;
    ext_RX_BD = extra;
  }
  tte_stop_ticking();

  for (int i =0; i<N; i++){ //logging
    printf("%lld\n",error[i]);
  }

  return 0;
}


