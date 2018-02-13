#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/rtc.h>
#include "../../libcorethread/corethread.h"
#include "../../libmp/mp.h"
#include "led.h"

#define MP_CHAN_NUM_BUF 1
#define CHANNEL_BUFFER_CAPACITY (100) 	// Number of words in one message burst
#define ACTIVE_CORES (1 + 4)			// Number of cores to send to +1 (for the sender)
#define TIMES (1000)					// The number of messages to send to each core

const int NOC_MASTER = 0;

void slave(void* args){
	led_on();
	int cpuid = get_cpuid();
	int local_buffer[CHANNEL_BUFFER_CAPACITY];
	qpd_t * chan = mp_create_qport(cpuid, SINK, CHANNEL_BUFFER_CAPACITY*sizeof(int),MP_CHAN_NUM_BUF);
	mp_init_ports();
	
	for(int i = 0; i<(TIMES*(ACTIVE_CORES-cpuid)); i++){
		mp_recv(chan,0);
		// Load the received values into memory
		for(int k = 0; k<CHANNEL_BUFFER_CAPACITY; k++){
			local_buffer[k] = ((volatile int _SPM *)(chan->read_buf))[k];
		}
		mp_ack(chan,0);
	}

	led_off();
}

int main(){
	led_on();
	
	qpd_t* chan[ACTIVE_CORES];

	// Start receivers
	for(int c = 1; c<ACTIVE_CORES; c++){
		corethread_create(c, &slave, NULL);
	}

	// Initialize channels
	for(int c = 1; c<ACTIVE_CORES; c++){
		chan[c] = mp_create_qport(c, SOURCE, CHANNEL_BUFFER_CAPACITY*sizeof(int),MP_CHAN_NUM_BUF);
	}
	mp_init_ports();
	
	int start, end;

	// Run bench
	for(int cores_to_send_to = 1; cores_to_send_to<ACTIVE_CORES; cores_to_send_to++){
		asm volatile ("" : : : "memory");
		start = get_cpu_cycles();
		asm volatile ("" : : : "memory");
		for(int i = 0; i<TIMES; i++){
			for(int c = 1; c<=cores_to_send_to; c++){
				int next_ack = ((TIMES*(cores_to_send_to-c))+i);
				
				// Wait for the receiver to acknowledge	the previous message
				unsigned int next_ack_actual = *(chan[c]->send_recv_count);	
				while(next_ack_actual != next_ack){
					next_ack_actual = *(chan[c]->send_recv_count);
				}
				
				// Put values to send
				for(int k = 0; k<CHANNEL_BUFFER_CAPACITY; k++){
					((volatile int _SPM *)(chan[c]->write_buf))[k] = i;
				}
				
				mp_send(chan[c],0);
			}
		}
		asm volatile ("" : : : "memory");
		end = get_cpu_cycles();
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




























































