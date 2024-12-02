#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/rtc.h>
#include "../../libcorethread/corethread.h"
#include "../../libmp/mp.h"
#include "../../libmp/mp_internal.h"
#include "sspm_properties.h"
#include "led.h"

#define MP_CHAN_NUM_BUF 2
#define MP_CHAN_BUF_SIZE 40
#define CHANNEL_BUFFER_CAPACITY (256)

const int TIMES_TO_WRITE = 1000;
int start_clock[TIMES_TO_WRITE];
int end_clock[TIMES_TO_WRITE];

int main(){
	led_on();
	int cpuid = get_cpuid();
	
	int start, end;

	for(int i = 0; i<TIMES_TO_WRITE; i++){
		asm volatile ("" : : : "memory");
		start = get_cpu_cycles();
		asm volatile ("" : : : "memory");
		
		for(int k = 0; k<CHANNEL_BUFFER_CAPACITY; k++){
			(( volatile int _SPM * ) LOWEST_SSPM_ADDRESS)[k] = i;
		}
		asm volatile ("" : : : "memory");
		end = get_cpu_cycles();
		asm volatile ("" : : : "memory");
		start_clock[i] = start;
		end_clock[i] = end;
	}
	
	printf("Write bursts: %d\n", CHANNEL_BUFFER_CAPACITY);
	for(int i = 0; i<TIMES_TO_WRITE; i++){
		printf("%d\n", end_clock[i]-start_clock[i]);
	}
	led_off();
	return 0;
}








