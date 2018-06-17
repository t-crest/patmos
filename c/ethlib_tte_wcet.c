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

unsigned int rx_addr = 0x000;
unsigned char VL0[] = {0x0F,0xA1};
unsigned char VL1[] = {0x0F,0xA2};

int tte_loop(char reply,int i) __attribute__((noinline));
int tte_loop(char reply,int i){
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
  unsigned long long r_pit[N];  //for logging
  unsigned long long p_pit[N];
  unsigned long long s_pit[N];
  unsigned int int_pd[N];
  unsigned long long trans_clk[N];

  unsigned char CT[] = {0xAB,0xAD,0xBA,0xBE};
  unsigned char reply;

  tte_initialize(0xC3500,200,CT,2); //0xC3500 = 10ms in clock cycles, cluster cycle is 20ms, CT, 2 virtual links
  tte_init_VL(0, 26,40); //VL 4001 starts at 2.6ms and has a period of 4ms
  tte_init_VL(1, 10,20); //VL 4002 starts at 1ms and has a period of 2ms
  tte_start_ticking();
  
  for (int i =0; i<N;){
    eth_mac_receive(rx_addr, 0);
    reply=tte_receive(rx_addr);
    reply=tte_receive_log(rx_addr,r_pit,p_pit,s_pit,int_pd,trans_clk,i); //for logging
    i=tte_loop(reply,i);
    tte_clock_tick(); //usually called by interrupt, but WCET analysis does not recognize this
  }
  tte_stop_ticking();

  for (int i =0; i<N; i++){ //logging
    printf("%llu %llu %llu %d %llu\n",r_pit[i],p_pit[i],s_pit[i],int_pd[i],trans_clk[i]);
  }

  return 0;
}


