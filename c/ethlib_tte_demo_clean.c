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
#include "ethlib/eth_mac_driver.h"
#include "ethlib/tte.h"

void demo_mode() __attribute__((noinline));
void demo_mode(){
	int n = 2000;
	
	unsigned long long r_pit[n];  //for logging
	unsigned long long p_pit[n];
	unsigned long long s_pit[n];
	//unsigned int int_pd[n];
	//unsigned long long trans_clk[n];
	
	static unsigned long long receive_point;

  	unsigned char CT[] = {0xAB,0xAD,0xBA,0xBE};
 	unsigned char VL0[] = {0x0F,0xA1};
 	unsigned char VL1[] = {0x0F,0xA2};
	unsigned int send_addr[] = {0x800,0xE00,0x1400,0x1A00,0x2000};
	unsigned char send_i = 0;

	int sched_errors=0;
	int tte=0;
	int eth=0;
	unsigned char reply;
	unsigned int cur_RX = 0x000;
	unsigned int ext_RX = 0x800;
	unsigned int cur_RX_BD = 0x600;
	unsigned int ext_RX_BD = 0x608;

	tte_initialize(0xC3500,200,CT,1); //0xC3500 = 10ms in clock cycles, cluster cycle is 20ms, CT, 2 virtual links sending
	tte_init_VL(0, 26,40); //VL 4001 starts at 2.6ms and has a period of 4ms
	//tte_init_VL(0, 50,100);
	tte_init_VL(1, 10,20); //VL 4002 starts at 1ms and has a period of 2ms
	tte_prepare_test_data(0x2000,VL0,0x04,1514);
	tte_start_ticking(0,0);
	eth_iowr(0x04, 0x00000004); //clear receive frame bit in int_source
        eth_iowr(cur_RX_BD+4, cur_RX); //set first receive buffer to store frame in 0x000
	eth_iowr(cur_RX_BD, 0x0000C000); //set empty and IRQ and not wrap
	eth_iowr(ext_RX_BD+4, ext_RX); //set second receive buffer to store frame in 0x800
	eth_iowr(ext_RX_BD, 0x00006000); //set NOT empty, IRQ and wrap

        #pragma loopbound min 0 max 2000
	for (int i =0; i<n;){
	  while ((eth_iord(0x04) & 0x4)==0){;};
	  receive_point = get_cpu_cycles();
	  //start receiving in other buffer
	  eth_iowr(0x04, 0x00000004);
	  unsigned cur_data = eth_iord(ext_RX_BD);
    	  eth_iowr(ext_RX_BD, cur_data | (1<<15));

	  //reply=tte_receive(cur_RX,get_cpu_cycles());
	  reply=tte_receive_log(cur_RX,receive_point,r_pit/*,p_pit,s_pit,int_pd,trans_clk*/,i); //for logging

	  //reply=2;
	  if(reply==0){ //failed pcf
            printf("pcf out of schedule \n");
	    n=i+1;
	    break;
          }
	  if(reply==1){ //successfull pcf
	    //if(i%2==0){
	      /*tte_prepare_test_data(0x2600,VL1,0x11,400);
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
	      if(!tte_schedule_send(0x5C00,300,1)) sched_errors++;*/
	    //}
	    //else{
	      /*tte_prepare_test_data(0x2000,VL0,0xee,1514);*/
	      //if(!tte_schedule_send(0x2000,1514,0)) sched_errors++;
	      /*if(!tte_schedule_send(0x2000,1514,0)) sched_errors++;
	      if(!tte_schedule_send(0x2000,1514,0)) sched_errors++;
	      if(!tte_schedule_send(0x2000,1514,0)) sched_errors++;
	      if(!tte_schedule_send(0x2000,1514,0)) sched_errors++;*/
	      /*tte_prepare_test_data(0x1A00,VL0,0xdd,800);
	      if(!tte_schedule_send(0x1A00,800,0)) sched_errors++;
	      tte_prepare_test_data(0x1400,VL0,0xcc,300);
	      if(!tte_schedule_send(0x1400,300,0)) sched_errors++;
	      tte_prepare_test_data(0xE00,VL0,0xbb,400);
	      if(!tte_schedule_send(0xE00,400,0)) sched_errors++;
	      tte_prepare_test_data(0x800,VL0,0xaa,200);
	      if(!tte_schedule_send(0x800,200,0)) sched_errors++;*/
	    //}
	    //i++;
	    s_pit[i]=0;
          } else if (reply==2){ //incoming tte
	    tte++;
	    s_pit[i]=1;
	    /*volatile int k=5;
	    for(int j=0;j<1000;j++){
		k=j+k%23+2;
	    }*/
	    reply=mem_iord_byte(cur_RX+14);
    	    tte_prepare_test_data(0x2000,VL0,0x04,1514); //sometimes causes receive delay?
            if(!tte_schedule_send(0x2000,1514,0)) sched_errors++; //sometimes causes receive delay?
	    tte_schedule_send(0x2000,1514,0);
	    r_pit[i]=get_cpu_cycles()-receive_point;
	  }
	  else{ //incoming ethernet msg
	    eth++;
	  }
	  p_pit[i]=receive_point;
	  i++;
	  unsigned int extra = cur_RX;
	  cur_RX = ext_RX;
	  ext_RX = extra;
	  extra = cur_RX_BD;
	  cur_RX_BD = ext_RX_BD;
	  ext_RX_BD = extra;
	} 
	tte_stop_ticking();
	printf("sched errors: %d\n",sched_errors);
	printf("received tte: %d\n",tte);
	printf("received eth: %d\n",eth); 
	for (int i =0; i<n; i++){ //logging
		//printf("%llu %llu %llu %d %llu\n",r_pit[i],p_pit[i],s_pit[i],int_pd[i],trans_clk[i]);
		printf("%lld %llu %llu\n",r_pit[i],p_pit[i],s_pit[i]);
	}
	return;
}

int main(){
	char c;

	demo_mode();

	return 0;
}


