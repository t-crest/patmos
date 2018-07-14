/*
  Copyright 2018 Technical University of Denmark, DTU Compute.
  All rights reserved.

  TTEthernet test program

  Author: Maja Lund (maja_lala@hotmail.com)
*/

#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include "ethlib/eth_mac_driver.h"
#include "ethlib/tte.h"

int main(){
	int n = 2000;
	
	signed long long error[n];  //for logging clock precision
	unsigned long long r_pit[n];
	
	static unsigned long long receive_point;

  	unsigned char CT[] = {0xAB,0xAD,0xBA,0xBE};
 	unsigned char VL0[] = {0x0F,0xA1};
 	unsigned char VL1[] = {0x0F,0xA2};

	int sched_errors=0;
	int tte=0;
	int eth=0;
	unsigned char reply;
	unsigned int cur_RX = 0x000;
	unsigned int ext_RX = 0x800;
	unsigned int cur_RX_BD = 0x600;
	unsigned int ext_RX_BD = 0x608;

	set_mac_address(0x1D000400,0x00000289);

	//int_period = 10ms, cluster cycle=20ms, CT, 2 VLs sending, max_delay, comp_delay, precision(0x33E) (in clock cycles)
	tte_initialize(100,200,CT,2,0x2A60,0x349,0x67C); 
	tte_init_VL(0, 8,40); //VL 4001 starts at 0.8ms and has a period of 4ms
	tte_init_VL(1, 10,20); //VL 4002 starts at 1ms and has a period of 2ms
	tte_start_ticking(0,0,0);
	eth_iowr(0x04, 0x00000004); //clear receive frame bit in int_source
        eth_iowr(cur_RX_BD+4, cur_RX); //set first receive buffer to store frame in 0x000
	eth_iowr(cur_RX_BD, 0x0000C000); //set empty and IRQ and not wrap
	eth_iowr(ext_RX_BD+4, ext_RX); //set second receive buffer to store frame in 0x800
	eth_iowr(ext_RX_BD, 0x00006000); //set NOT empty, IRQ and wrap

	for (int i =0; i<n;){
	  tte_wait_for_message(&receive_point);

	  tte_clear_free_rx_buffer(ext_RX_BD); //start receiving in other buffer

	  //reply=tte_receive(cur_RX,receive_point);
	  reply=tte_receive_log(cur_RX,receive_point,error,i); //for logging

	  if(reply==0){ //failed pcf, stop test
            printf("pcf out of schedule \n");
	    n=i+1;
	    break;
          }
	  if(reply==1){ //successfull pcf
	    if(i%2==0){
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
	    r_pit[i]=receive_point;
	    i++;
          } else if (reply==2){ //incoming tte
	    tte++;
	    reply=mem_iord_byte(cur_RX+14);
    	    tte_prepare_test_data(0x2000,VL0,reply,1514);
            if(!tte_schedule_send(0x2000,1514,0)) sched_errors++;
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
	for (int i =0; i<n; i++){ //logging
		printf("%lld %llu\n",error[i],r_pit[i]);
	}
	return 0;
}
