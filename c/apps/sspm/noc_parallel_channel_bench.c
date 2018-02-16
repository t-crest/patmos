#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/rtc.h>
#include "../../libcorethread/corethread.h"
#include "../../libmp/mp.h"
#include "led.h"

#define MP_CHAN_NUM_BUF 1
#define CHANNEL_BUFFER_CAPACITY (2) 	// Number of words in one message burst
#define MAX_PARALLEL_CHANNELS (4)		// Number of sender/receiver pairs
#define TIMES (1000)					// The number of messages to send to each core

const int NOC_MASTER = 0;
volatile _UNCACHED int ready = 0;
volatile _UNCACHED int done[MAX_PARALLEL_CHANNELS];

void slave_receiver(void* args){
	led_on();
	int cpuid = get_cpuid();
	int channelNr = (cpuid-2)/2;
	int local_buffer[CHANNEL_BUFFER_CAPACITY];
	qpd_t * chan = mp_create_qport(channelNr, SINK, CHANNEL_BUFFER_CAPACITY*sizeof(int),MP_CHAN_NUM_BUF);
	mp_init_ports();
	
	for(int i = 0; i<TIMES; i++){
		mp_recv(chan,0);
		// Load the received values into memory
		for(int k = 0; k<CHANNEL_BUFFER_CAPACITY; k++){
			local_buffer[k] = ((volatile int _SPM *)(chan->read_buf))[k];
		}
		mp_ack(chan,0);
	}

	led_off();
}

void slave_sender(void* args){
	led_on();
	int cpuid = get_cpuid();
	int channelNr = (cpuid-1)/2;
	qpd_t * chan = mp_create_qport(channelNr, SOURCE, CHANNEL_BUFFER_CAPACITY*sizeof(int),MP_CHAN_NUM_BUF);
	mp_init_ports();
	
	done[channelNr] = 0;

	// Wait for the main core to flag the start
	while(!ready){}
	
	for(int i = 0; i<TIMES; i++){
		// Put values to send
		for(int k = 0; k<CHANNEL_BUFFER_CAPACITY; k++){
			((volatile int _SPM *)(chan->write_buf))[k] = i;
		}
		
		mp_send(chan,0);

		int next_ack = chan->send_count;
				
		// Wait for the receiver to acknowledge	the message
		unsigned int next_ack_actual = *(chan->send_recv_count);	
		while(next_ack_actual != next_ack){
			next_ack_actual = *(chan->send_recv_count);
		}
	}
	
	done[channelNr] = 1;

	led_off();
}

int main(){
	led_on();
	
	for(int parallel_channels = 1; parallel_channels<=MAX_PARALLEL_CHANNELS; parallel_channels++){
		ready = 0;
		// Start pairs
		for(int c=0; c<parallel_channels; c++){
			corethread_create((c*2)+1, &slave_sender, NULL);
			corethread_create((c*2)+2, &slave_receiver, NULL);
		}

		//Sleep to allow slaves to get ready
		led_off_for(100);
		led_on_for(100);
		led_off_for(100);
		led_on();

		int start, end;
		asm volatile ("" : : : "memory");
		start = get_cpu_cycles();
		asm volatile ("" : : : "memory");
		ready = 1;	
	
		// Wait for the senders to finish
		for(int c = 0; c<parallel_channels; c++){
			while(!done[c]){}
		}

		asm volatile ("" : : : "memory");
		end = get_cpu_cycles();
		asm volatile ("" : : : "memory");
		
		// Make sure the thread have terminated
		for(int c = 0; c<parallel_channels; c++){
			int res;
			corethread_join((c*2)+1, (void **) &res);
			corethread_join((c*2)+2, (void **) &res);
		}		

		printf("Parallel channels: %d\n", parallel_channels);
		int cycles = end - start;
		double per_word_sent = ((double)cycles/(double)(TIMES*parallel_channels))/(double)CHANNEL_BUFFER_CAPACITY;	
		printf("%f\n", per_word_sent);	
	}
	led_off();
	return 0;
}




























































