#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/rtc.h>
#include "../../libcorethread/corethread.h"
#include "../../libmp/mp.h"
#include "../../libmp/mp_internal.h"
#include "sspm_properties.h"

#define MP_CHAN_NUM_BUF 2
#define MP_CHAN_BUF_SIZE 40

const int NOC_MASTER = 0;

const int CHANNEL_BUFFER_CAPACITY = 64;
const int TIMES_TO_SEND = 1000;
volatile _UNCACHED int send_clock[TIMES_TO_SEND];
volatile _UNCACHED int recv_clock[TIMES_TO_SEND];
volatile _UNCACHED int ack_recv_clock[TIMES_TO_SEND];

typedef enum{
	TRANSMIT,
	ACKNOWLEDGE
} TRANSMISSION_STATE;

void sender_slave(void* args){
	volatile _SPM int* flag = (volatile _SPM int*) LOWEST_SSPM_ADDRESS;

	int send, ack_recv;

	for(int k = 0; k< TIMES_TO_SEND; k++){
		asm volatile ("" : : : "memory");
		send = get_cpu_cycles();
		asm volatile ("" : : : "memory");

		*flag = TRANSMIT;
		while(*flag != ACKNOWLEDGE){}
		
		asm volatile ("" : : : "memory");
		ack_recv = get_cpu_cycles();
		asm volatile ("" : : : "memory");

		send_clock[k] = send;
		ack_recv_clock[k] = ack_recv;
	}
}

void receiver_slave(void* args){
	int cpuid = get_cpuid();
	volatile _SPM int* flag = (volatile _SPM int*) LOWEST_SSPM_ADDRESS;

	int recv;
	
	for(int k = 0; k<TIMES_TO_SEND; k++){
		while(*flag != TRANSMIT){}
		asm volatile ("" : : : "memory");
		recv = get_cpu_cycles();
		asm volatile ("" : : : "memory");

		*flag = ACKNOWLEDGE;

		asm volatile ("" : : : "memory");
		recv_clock[k] = recv;
		asm volatile ("" : : : "memory");
	}
}


int main(){
	volatile _SPM int* flag = (volatile _SPM int*) LOWEST_SSPM_ADDRESS;
	*flag = ACKNOWLEDGE;

	corethread_create(1, &sender_slave, NULL);
	corethread_create(2, &receiver_slave, NULL);

	int res;
	corethread_join(1, (void **) &res);
	corethread_join(2, (void **) &res);
	
	printf("Burst: %d\n", CHANNEL_BUFFER_CAPACITY);

	printf("Send clocks:\n");
	for(int i = 0; i < TIMES_TO_SEND; i++){
		printf("%d\n", (recv_clock[i] - send_clock[i]));
	}
	
	printf("Ack clocks:\n");
	for(int i = 0; i < TIMES_TO_SEND; i++){
		printf("%d\n", (ack_recv_clock[i] - recv_clock[i]));
	}

	return 0;
}








