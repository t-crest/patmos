/*
  Producer/intermediate/consumer example for the S4NOC.
  Slow down the producer until no tokens are lost.

  Author: Martin Schoeberl and Luca Pezzarossa
*/

#include <stdio.h>
#include <machine/spm.h>
#include "../../libcorethread/corethread.h"

#include "s4noc.h"

#ifndef DELAY
#define DELAY 21
#endif

volatile _UNCACHED int started_consumer;
volatile _UNCACHED int started_intermediate;
volatile _UNCACHED int done;
volatile _UNCACHED int result;
volatile _UNCACHED int time;

volatile _UNCACHED int buffer_a[BUF_LEN];
volatile _UNCACHED int buffer_b[BUF_LEN];

void consumer(void* arg) {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);
  int ts;
  int sum = 0;

  // get started
  started_consumer=1;

  for (int i=0; i<LEN/BUF_LEN; ++i) {
    for (int j=0; j<BUF_LEN; ++j) {
      while (!s4noc[RX_READY]) {
        ;
      }
      sum += s4noc[IN_DATA];
    }
  }
  time = *timer_ptr - time;
  result = sum;
  done = 1;
  //join thread
  int ret = 0;
	corethread_exit(&ret);
	return;
}

void intermediate(void* arg) {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);
  volatile _UNCACHED int * receive_buffer = buffer_a;
  volatile _UNCACHED int * send_buffer = buffer_b;

  // get started
  started_intermediate=1;


  while(1) {
    //for (int j=0; j<BUF_LEN; ++j) {
      while (!s4noc[RX_READY]) {
        ;
      }
      s4noc[SEND_SLOT] = s4noc[IN_DATA];
      // Uncomment to see the effect of the intermediate
      //*dead_ptr = 100; 
      //val = *dead_ptr;
    //}
  }

  //join thread
  int ret = 0;
	corethread_exit(&ret);
	return;
}
 
int main() {
  printf("I am alive!\n");

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);
  int *retval;
  int val = 0;

  int intermediate_id = 7; //Producer sends in SLOT 0
  int consumer_id = 5; //Intermediate sends in SLOT 0

  done = 0;
  result = 0;
  started_consumer = 0;
  started_intermediate = 0;

  val = 0;

  corethread_create(consumer_id, &consumer, NULL);
  corethread_create(intermediate_id, &intermediate, NULL);

  while (started_intermediate && started_consumer) {;}

  // Give the other threads some head start to be ready
  *dead_ptr = 1000;
  val = *dead_ptr;

  // start timing
  time = *timer_ptr;

  for (int i=0; i<LEN/BUF_LEN; ++i) {
    for (int j=0; j<BUF_LEN; ++j) {
      s4noc[SEND_SLOT] = 1;
      *dead_ptr = DELAY;
      val = *dead_ptr;
    }
  }

  printf("Number of cores: %d\n", get_cpucnt());
  // now, after the print, we should be done
  if (done) {
    printf("  %d received in %d cycles, %g cycles per word.\n", result, time, 1. * time/result);
  } else {
    printf("  Not done.\n");
    // feed more tokens to get the consumer finished
    while (!done) {
      s4noc[SEND_SLOT] = 0;
    }
    printf("  %d out of %d received\n", result, LEN);
  }
  
  //join the threads
  corethread_join(consumer_id, (void **)&retval);
  //corethread_join(intermediate_id, (void **)&retval);
    
  printf("Goodbye!\n");
  
}















