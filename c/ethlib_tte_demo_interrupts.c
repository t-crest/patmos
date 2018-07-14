/*
  Copyright 2018 Technical University of Denmark, DTU Compute.
  All rights reserved.

  TTEthernet test program for interrupt solution

  Author: Maja Lund (maja_lala@hotmail.com)
*/

#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/exceptions.h>
#include "ethlib/eth_mac_driver.h"
#include "ethlib/tte.h"

unsigned int rx_addr = 0x000;
signed long long error[2000];  //for logging
unsigned long long r_pit[2000]; //for logging
static unsigned long long receive_point;


volatile char stop = 0;
volatile int ite = 0;
volatile int tte=0;
volatile int eth=0;

void intr_handler(void) __attribute__((naked));
void intr_handler(void) {
  exc_prologue();

  receive_point = get_cpu_cycles();
  volatile unsigned char reply = tte_receive_log(rx_addr,receive_point,error,ite);
  r_pit[ite]=receive_point;
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
  else if(reply==3){
    eth++;
  }
  if (!stop){
    eth_iowr(0x04, 0x00000004);
    eth_iowr(0x604, rx_addr);
    eth_iowr(0x600, 0x0000E000);
  }

  exc_epilogue();
}

void demo_mode(){
	unsigned char CT[] = {0xAB,0xAD,0xBA,0xBE};
 	unsigned char VL0[] = {0x0F,0xA1};
 	unsigned char VL1[] = {0x0F,0xA2};
	unsigned int send_addr[] = {0x800,0xE00,0x1400,0x1A00,0x2000};
	unsigned char send_i = 0;

	int sched_errors=0;

	set_mac_address(0x1D000400,0x00000289);

	//int_period = 10ms, cluster cycle=20ms, CT, 2 VLs sending, max_delay, comp_delay, precision
	tte_initialize(100,200,CT,2,0x2A60,0x349,0x67C);
	tte_init_VL(0, 8,40); //VL 4001 starts at 0.8ms and has a period of 4ms
	tte_init_VL(1, 10,20); //VL 4002 starts at 1ms and has a period of 2ms
	tte_start_ticking(0,1, &intr_handler);
	eth_iowr(0x04, 0x00000004);
	eth_iowr(0x604, rx_addr);
	eth_iowr(0x600, 0x0000E000);

	volatile int local_ite=0;
	while(local_ite<2000 && !stop){
	  while (local_ite>=ite && !stop){;}
	  local_ite=ite;
	  if(local_ite%2==0){
	    tte_prepare_test_data(0x2600,VL0,0xAA,400);
  	    if(!tte_schedule_send(0x2600,400,0)) sched_errors++;
	    tte_prepare_test_data(0x2C00,VL0,0xBB,300);
	    if(!tte_schedule_send(0x2C00,300,0)) sched_errors++;
	    tte_prepare_test_data(0x3200,VL0,0xCC,800);
	    if(!tte_schedule_send(0x3200,800,0)) sched_errors++;
	    tte_prepare_test_data(0x3800,VL0,0xDD,1514);
	    if(!tte_schedule_send(0x3800,1514,0)) sched_errors++;
	    tte_prepare_test_data(0x3E00,VL0,0xEE,400);
	    if(!tte_schedule_send(0x3E00,400,0)) sched_errors++;
	    tte_prepare_test_data(0x4400,VL1,0x11,300);
	    if(!tte_schedule_send(0x4400,300,1)) sched_errors++;
	    tte_prepare_test_data(0x4A00,VL1,0x22,800);
	    if(!tte_schedule_send(0x4A00,800,1)) sched_errors++;
            tte_prepare_test_data(0x5000,VL1,0x33,1514);
	    if(!tte_schedule_send(0x5000,1514,1)) sched_errors++;
	    tte_prepare_test_data(0x5600,VL1,0x44,400);
	    if(!tte_schedule_send(0x5600,400,1)) sched_errors++;
	    tte_prepare_test_data(0x5C00,VL1,0x55,300);
	    if(!tte_schedule_send(0x5C00,300,1)) sched_errors++;
	    tte_prepare_test_data(0x6200,VL1,0x66,300);
	    if(!tte_schedule_send(0x6200,300,1)) sched_errors++;
	    tte_prepare_test_data(0x6800,VL1,0x77,800);
	    if(!tte_schedule_send(0x6800,800,1)) sched_errors++;
            tte_prepare_test_data(0x6E00,VL1,0x88,1514);
	    if(!tte_schedule_send(0x6E00,1514,1)) sched_errors++;
	    tte_prepare_test_data(0x7400,VL1,0x99,400);
	    if(!tte_schedule_send(0x7400,400,1)) sched_errors++;
	    tte_prepare_test_data(0x7A00,VL1,0x10,300);
	    if(!tte_schedule_send(0x7A00,300,1)) sched_errors++;
	  }
	}
	
	stop=1;
	tte_stop_ticking();
	intr_clear_all_pending();
	printf("out local:%d ite:%d\n",local_ite,ite);
	printf("sched errors: %d\n",sched_errors);
	printf("received tte: %d\n",tte);
	printf("received eth: %d\n",eth); 
	for (int i =0; i<=ite; i++){ //logging
		printf("%lld %llu\n",error[i],r_pit[i]);
	}
	return;
}

int main(){
	char c;

	demo_mode();

	return 0;
}


