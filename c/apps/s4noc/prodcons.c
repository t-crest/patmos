/*
  Producer/consumer example for the S4NOC.
  Slow down the producer until no tokens are lost.

  Author: Martin Schoeberl
*/

#include <stdio.h>
#include <machine/spm.h>
#include "../../libcorethread/corethread.h"

#include "s4noc.h"

#ifndef DELAY
#define DELAY 10
#endif

volatile _UNCACHED int started;
volatile _UNCACHED int done;
volatile _UNCACHED int result;
volatile _UNCACHED int time;

void work(void* arg) {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);
  int ts;
  int sum = 0;

  // get started
  started = 1;

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
}

int main() {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);

  done = 0;
  result = 0;
  started = 0;

  int val = 0;

  // Receiver for time slot 0 depends in schedule, which depends on number of cores
  // This should better come from s4noc.h
  int rcv = get_cpucnt() == 4 ? 3 : 7;

  corethread_create(rcv, &work, NULL);

  while (!started) {
    ;
  }

  // Give the other threads some head start to be ready
  *dead_ptr = 1000;
  val = *dead_ptr;

  // start timing
  time = *timer_ptr;

  for (int i=0; i<LEN/BUF_LEN; ++i) {
    for (int j=0; j<BUF_LEN; ++j) {
      // when we do the delay, could we also drop checking for TX free?
/*
      while (!s4noc[TX_FREE]) {
        ;
      }
*/
      s4noc[SEND_SLOT] = 1;
      *dead_ptr = DELAY;
      val = *dead_ptr;
    }
  }

  printf("Number of cores: %d\n", get_cpucnt());
  // now, after the print, we should be done
  if (done) {
    printf("%d received in %d cycles, %g cycles per word\n", result, time, 1. * time/result);
  } else {
    printf("Not done\n");
    // feed more tokens to get the consumer finished
    while (!done) {
      s4noc[SEND_SLOT] = 0;
    }
    printf("%d out of %d received\n", result, LEN);
  }
}
