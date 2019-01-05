/*
  Producer/intermediate/consumer example for the S4NOC.
  Slow down the producer until no tokens are lost.

  Author: Martin Schoeberl and Luca Pezzarossa
*/

#include <stdio.h>
#include <machine/spm.h>
#include "../../libcorethread/corethread.h"

#include "s4noc.h"

#define SLOT_PRODU_TO_INTER 0
#define INTERMEDIATE_CORE 7
//#define SLOT_INTER_TO_CONSU 0
//#define CONSUMER_CORE 5

//#define SLOT_INTER_TO_CONSU 4
//#define CONSUMER_CORE 4

#define SLOT_INTER_TO_CONSU 1
#define CONSUMER_CORE 8

volatile _UNCACHED int started_consumer;
volatile _UNCACHED int started_intermediate;
volatile _UNCACHED int done;
volatile _UNCACHED int result;
volatile _UNCACHED int time;

void consumer(void* arg) {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);
  int sum = 0;

  started_consumer=1;

  for (int i=0; i<LEN/BUF_LEN; ++i) {
    for (int j=0; j<BUF_LEN; ++j) {
      while (!s4noc[RX_READY]) {;}
      sum += s4noc[IN_DATA];
    }
  }
  
  time = *timer_ptr - time;
  result = sum;
  done = 1;

	return;
}

void intermediate(void* arg) {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);

  started_intermediate=1;

  while(1) {
    while (!s4noc[RX_READY] || !s4noc[TX_FREE]) {;}
    s4noc[SLOT_INTER_TO_CONSU] = s4noc[IN_DATA];
  }

	return;
}
 
int main() {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);
  int *retval;
  int val = 0;

  done = 0;
  result = 0;
  started_consumer = 0;
  started_intermediate = 0;

  val = 0;

  printf("Delay %d:\n", DELAY);
  corethread_create(CONSUMER_CORE, &consumer, NULL);
  corethread_create(INTERMEDIATE_CORE, &intermediate, NULL);


  while (started_intermediate == 0 || started_consumer == 0) {;}
  //printf("All threads started!\n");

  // Give the other threads some head start to be ready
  *dead_ptr = 10000;
  val = *dead_ptr;

  // start timing
  time = *timer_ptr;

  for (int i=0; i<LEN/BUF_LEN; ++i) {
    for (int j=0; j<BUF_LEN; ++j) {
      // when we do the delay, could we also drop checking for TX free?

      while (!s4noc[TX_FREE]) {;}
      
      s4noc[SLOT_PRODU_TO_INTER] = 1;
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
      s4noc[SLOT_PRODU_TO_INTER] = 0;
    }
    printf("  %d out of %d received\n", result, LEN);
  }
    
  printf("Goodbye!\n");
  
}















