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
 * Main function for ethlib (ethernet library) demo
 * extended to test some initial TTE stuff
 * 
 * Authors: Luca Pezzarossa (lpez@dtu.dk)
 *          Jakob Kenn Toft
 *          Jesper Lønbæk
 *          Russell Barnes
 *          Maja Lund
 */

#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/exceptions.h>
#include "ethlib/eth_mac_driver.h"
#include "ethlib/tte.h"

#define LEDS (*((volatile _IODEV unsigned *)0xf0090000))
#define SLEEP (*((volatile _IODEV unsigned *)0xf0010010))

unsigned int rx_addr = 0x000;
unsigned long long r_pit[2000];  //for logging
unsigned long long p_pit[2000];
unsigned long long s_pit[2000];
unsigned int int_pd[2000];
unsigned long long trans_clk[2000];

volatile char stop = 0;
volatile int ite = 0;
volatile int tte=0;

void intr_handler(void) __attribute__((naked));
void intr_handler(void) {
  exc_prologue();

  //LEDS ^= 1;
  //putc('0', stderr);
  volatile unsigned char reply = tte_receive_log(rx_addr,get_cpu_cycles(),r_pit,p_pit,s_pit,int_pd,trans_clk,ite);
  //char reply = 0;
  if(reply==0){ //failed pcf
    puts("0");
    stop=1;
  }
  else if(reply==1){
    ite++;
  }
  else if(reply==2){
    tte++;
  }
  if (!stop){
    eth_iowr(0x04, 0x00000004);
    eth_iowr(0x604, rx_addr);
    eth_iowr(0x600, 0x0000E000);
  }

  exc_epilogue();
}

void demo_mode() __attribute__((noinline));
void demo_mode(){
	unsigned char CT[] = {0xAB,0xAD,0xBA,0xBE};
 	unsigned char VL0[] = {0x0F,0xA1};
 	unsigned char VL1[] = {0x0F,0xA2};
	unsigned int send_addr[] = {0x800,0xE00,0x1400,0x1A00,0x2000};
	unsigned char send_i = 0;

	int sched_errors=0;
	int eth=0;
	unsigned char reply;

	tte_initialize(0xC3500,200,CT,2); //0xC3500 = 10ms in clock cycles, cluster cycle is 20ms, CT, 2 virtual links
	tte_init_VL(0, 26,40); //VL 4001 starts at 2.6ms and has a period of 4ms
	tte_init_VL(1, 10,20); //VL 4002 starts at 1ms and has a period of 2ms
	tte_start_ticking(1, &intr_handler);
	eth_iowr(0x04, 0x00000004);
	eth_iowr(0x604, rx_addr);
	eth_iowr(0x600, 0x0000E000);

	volatile int local_ite=0;
	while(local_ite<2000 && !stop){
	  while (local_ite>=ite){;}
	  if (local_ite>10000) printf("%d\n", local_ite); //black voodoo magic
	  local_ite=ite;
	  if(local_ite%2==0){
	    tte_prepare_test_data(0x2600,VL1,0x11,400);
  	    if(!tte_schedule_send(0x2600,400,1)) sched_errors++;
	    tte_prepare_test_data(0x2C00,VL1,0x22,300);
	    if(!tte_schedule_send(0x2C00,300,1)) sched_errors++;
	    tte_prepare_test_data(0x3200,VL1,0x33,800);
	    if(!tte_schedule_send(0x3200,800,1)) sched_errors++;
	    tte_prepare_test_data(0x3800,VL1,0x44,1514);
	    if(!tte_schedule_send(0x3800,1514,1)) sched_errors++;
	    tte_prepare_test_data(0x3E00,VL1,0x55,400);
	    if(!tte_schedule_send(0x3E00,400,1)) sched_errors++;
	    tte_prepare_test_data(0x4400,VL1,0x66,300);
	    if(!tte_schedule_send(0x4400,300,1)) sched_errors++;
	    tte_prepare_test_data(0x4A00,VL1,0x77,800);
	    if(!tte_schedule_send(0x4A00,800,1)) sched_errors++;
            tte_prepare_test_data(0x5000,VL1,0x88,1514);
	    if(!tte_schedule_send(0x5000,1514,1)) sched_errors++;
	    tte_prepare_test_data(0x5600,VL1,0x99,400);
	    if(!tte_schedule_send(0x5600,400,1)) sched_errors++;
	    tte_prepare_test_data(0x5C00,VL1,0x10,300);
	    if(!tte_schedule_send(0x5C00,300,1)) sched_errors++;
	  }
	  //printf("%d %d\n", ite,local_ite);
	}
	
	stop=1;
	tte_stop_ticking();
	SLEEP=0;
	intr_clear_all_pending();
	unsigned long long int start_time = get_cpu_usecs();
	while ((get_cpu_usecs()-start_time < 10000)){;};
	printf("out local:%d ite:%d\n",local_ite,ite);
	printf("sched errors: %d\n",sched_errors);
	printf("received tte: %d\n",tte);
	printf("received eth: %d\n",eth); 
	for (int i =0; i<=ite; i++){ //logging
		printf("%llu %llu %llu %d %llu\n",r_pit[i],p_pit[i],s_pit[i],int_pd[i],trans_clk[i]);
	}
	return;
}

int main(){
	char c;

	demo_mode();

	return 0;
}


