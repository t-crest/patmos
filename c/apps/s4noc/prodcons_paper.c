/*
  Producer/consumer example for the S4NOC paper.
  Slow down the producer until no tokens are lost.

  Author: Martin Schoeberl and Luca Pezzarossa
*/

#include <stdio.h>
#include <machine/spm.h>
#include "../../libcorethread/corethread.h"

#include "s4noc.h"

#define DELAY 100

#define SLOT_PRODU_TO_CONSU 0
#define CONSUMER_CORE 7

//#define LEN  20 // in words
//#define BUF_LEN 16 // in words

volatile _UNCACHED int started;
volatile _UNCACHED int done;
volatile _UNCACHED int result;
volatile _UNCACHED int time;
volatile _UNCACHED int array[LEN];

void work(void* arg) {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);
  int sum = 0;

  // get started
  started = 1;

  for (int i=0; i<LEN; ++i) {
  //for (int i=0; i<LEN/BUF_LEN; ++i) {
  //  for (int j=0; j<BUF_LEN; ++j) {
      while (!s4noc[RX_READY]) {;}
      //sum += s4noc[IN_DATA];
      //array[i*BUF_LEN + j] = s4noc[IN_DATA];
      array[i] = s4noc[IN_DATA];
  //  }
  }
  time = *timer_ptr - time;
  result = sum;
  done = 1;
  //join thread
//  int ret = 0;
//	corethread_exit(&ret);
//	return;
}

int main() {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);
  int *retval;
  int val = 0;

  done = 0;
  result = 0;
  started = 0;

  val = 0;

  //printf("Delay: %d\n", DELAY);
  corethread_create(CONSUMER_CORE, &work, NULL);

  while (!started) {
    ;
  }

  // Give the other threads some head start to be ready
  *dead_ptr = 80000000;
  val = *dead_ptr;

  // start timing
  time = *timer_ptr;

  for (int i=0; i<LEN/BUF_LEN; ++i) {
    for (int j=0; j<BUF_LEN; ++j) {
      while (!s4noc[TX_FREE]) {;}
      *dead_ptr = DELAY/2;
      val = *dead_ptr;
      s4noc[SLOT_PRODU_TO_CONSU] = i*BUF_LEN + j;//1;
      *dead_ptr = DELAY/2;
      val = *dead_ptr;
    }
  }
        
  // Give the other threads some head start to be ready
  *dead_ptr = 80000000;
  val = *dead_ptr;
  
//  printf("Number of cores: %d\n", get_cpucnt());
  // now, after the print, we should be done
  if (done) {
    printf("  %d received in %d cycles, %g cycles per word.\n", result, time, 1. * time/result);
  } else {
    printf("  Not done.\n");
    // feed more tokens to get the consumer finished
    while (!done) {
      s4noc[SLOT_PRODU_TO_CONSU] = 0;
    }
    printf("  %d out of %d received\n", result, LEN);
  }
  
  
  for (int i=0; i<LEN/BUF_LEN; ++i) {
    for (int j=0; j<BUF_LEN; ++j) {
      printf("Array[%d] = %d;  ", i*BUF_LEN + j, array[i*BUF_LEN + j]);
    }
  }
    
  //join the threads
  //corethread_join(rcv, (void **)&retval);
    
    
}















