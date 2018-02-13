#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/rtc.h>
#include "../../libcorethread/corethread.h"
#include "../../libmp/mp.h"
#include "sspm_properties.h"
#include "led.h"

#define CHANNEL_BUFFER_CAPACITY (100) 	// Number of words in one message burst
#define ACTIVE_CORES (1 + 4)			// Number of cores to send to +1 (for the sender)
#define TIMES (1000)					// The number of messages to send to each core

const int NOC_MASTER = 0;

typedef enum{
	TRANSMIT,
	ACKNOWLEDGE
} TRANSMISSION_STATE;

void slave(void* args){
	led_on();
	int cpuid = get_cpuid();
	int local_buffer[CHANNEL_BUFFER_CAPACITY];
	
	volatile _SPM int* flag = (volatile _SPM int*) (LOWEST_SSPM_ADDRESS + ((cpuid-1) * ((CHANNEL_BUFFER_CAPACITY+1)*sizeof(int))));
	volatile _SPM int* chan = &(flag[1]);

	for(int i = 0; i<(TIMES*(ACTIVE_CORES-cpuid)); i++){
		// wait to receive something
		while(*flag != TRANSMIT){}
		// Load the received values into memory
		for(int k = 0; k<CHANNEL_BUFFER_CAPACITY; k++){
			local_buffer[k] = chan[k];
		}
		*flag = ACKNOWLEDGE;
	}

	led_off();
}



int main(){
	led_on();
	
	volatile _SPM int* flag[ACTIVE_CORES];
	volatile _SPM int* chan[ACTIVE_CORES];

	// Calculate channel addresses
	for(int c = 1; c<ACTIVE_CORES; c++){
		flag[c] = (volatile _SPM int*) (LOWEST_SSPM_ADDRESS + ((c-1) * ((CHANNEL_BUFFER_CAPACITY+1)*sizeof(int))));
		chan[c] = &(flag[c][1]);
	}

	// Start receivers
	for(int c = 1; c<ACTIVE_CORES; c++){
		corethread_create(c, &slave, NULL);
	}

	// Run bench
	for(int cores_to_send_to = 1; cores_to_send_to<ACTIVE_CORES; cores_to_send_to++){
		asm volatile ("" : : : "memory");
		double start = get_cpu_cycles();
		asm volatile ("" : : : "memory");
		for(int i = 0; i<TIMES; i++){
			for(int c = 1; c<=cores_to_send_to; c++){
				// Wait for the receiver to acknowledge	the previous message		
				while(*flag[c] != ACKNOWLEDGE){}
			
				// Put values to send
				for(int k = 0; k<CHANNEL_BUFFER_CAPACITY; k++){
					*chan[c] = i;
				}

				*flag[c] = TRANSMIT;
			}
		}
		asm volatile ("" : : : "memory");
		double end = get_cpu_cycles();
		asm volatile ("" : : : "memory");
		
		printf("Sending to %d\n", cores_to_send_to);
		int cycles = end - start;
		double per_word_sent = ((double)cycles/(double)(TIMES*cores_to_send_to))/(double)CHANNEL_BUFFER_CAPACITY;
		//printf("%d\n", cycles);
		printf("%f\n", per_word_sent);
	}

	for(int c = 1; c<ACTIVE_CORES; c++){
		int res;
		corethread_join(c, (void **) &res);
	}

	led_off();
	return 0;
}
