/*
  Copyright 2018 Technical University of Denmark, DTU Compute.
  All rights reserved.

  TTEthernet test program for latency measurements

  Author: Maja Lund (maja_lala@hotmail.com)
*/

#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include "ethlib/eth_mac_driver.h"
#include "ethlib/tte.h"

void demo_mode(){
	int n = 2010;
	
	unsigned long long rec_point[n];
	unsigned long long sched_point[n];
	
	static unsigned long long receive_point;
	static unsigned long long send_point;

  	unsigned char CT[] = {0xAB,0xAD,0xBA,0xBE};
	unsigned char VL2[] = {0x0F,0xA3};
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

	set_mac_address(0x1D000400,0x00000289);

	//int_period = 10ms, cluster cycle=20ms, CT, 1 VL sending, max_delay, comp_delay, precision
	tte_initialize(100,200,CT,1,0x2A60,0x349,0x33E);

	//initialize other ethmac
	eth_iowr1(0x00, 0x0000A423);

	tte_init_VL(0, 82,100); //VL 4003 starts at 8.2ms and has a period of 10ms
	tte_start_ticking(1,0,0);
	eth_iowr(0x04, 0x00000004); //clear receive frame bit in int_source
        eth_iowr(cur_RX_BD+4, cur_RX); //set first receive buffer to store frame in 0x000
	eth_iowr(cur_RX_BD, 0x0000C000); //set empty and IRQ and not wrap
	eth_iowr(ext_RX_BD+4, ext_RX); //set second receive buffer to store frame in 0x800
	eth_iowr(ext_RX_BD, 0x00006000); //set NOT empty, IRQ and wrap

	for (int i =0; i<n;){
	  tte_wait_for_message(&receive_point); //eth0

	  tte_clear_free_rx_buffer(ext_RX_BD); //start receiving in other buffer

	  reply=tte_receive(cur_RX,receive_point);

	  if(reply==0){ //failed pcf
            printf("pcf out of schedule \n");
	    n=i+1;
	    break;
          }
	  if(reply==1){ //successfull pcf
	    if(i>=10){
	      send_point=*(_iodev_ptr_t)(__PATMOS_TIMER_LOCLK);
	      tte_prepare_test_data(0x2600,VL2,0x11,64);
  	      if(!tte_schedule_send(0x2600,64,0)) sched_errors++;
	      
	      reply=0;
	      while(reply==0){
	        //set a buffer descriptor to receive in other ethmac controller
	        eth_iowr1(0x04, 0x00000004); 
                eth_iowr1(0x604, 0x00); 
	        eth_iowr1(0x600, 0x0000E000); 

	        //wait for it to be received on other ethmac controller
	        while ((eth_iord1(0x04) & 0x4)==0){
	          receive_point = *(_iodev_ptr_t)(__PATMOS_TIMER_LOCLK); //to avoid delay error
	        };
	        receive_point = *(_iodev_ptr_t)(__PATMOS_TIMER_LOCLK);
	      
	        //check if received message is tte and correct VL
		if(mem_iord_byte1(0x00)==CT[0] &&
		  mem_iord_byte1(0x01)==CT[1] &&
		  mem_iord_byte1(0x02)==CT[2] &&
		  mem_iord_byte1(0x03)==CT[3] &&
		  mem_iord_byte1(0x04)==VL2[0] &&
		  mem_iord_byte1(0x05)==VL2[1]){
		  reply=1; 
		}
	      }
	      rec_point[i]=receive_point;
	      sched_point[i]=send_point;
	    }
	    i++;
          } else if (reply==2){ //incoming tte
	    tte++;
	  }
	  else{ //incoming ethernet msg
	    eth++;
	  }
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
	for (int i =10; i<n; i++){ //logging
		printf("%lld %lld %lld\n",sched_point[i],send_times[i-10],rec_point[i]);
	}
	return;
}

int main(){
	char c;

	demo_mode();

	return 0;
}


