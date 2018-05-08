/*
  Producer/consumer example for the S4NOC.

  Author: Martin Schoeberl
*/

#include <stdio.h>
#include <machine/spm.h>
#include "../../libcorethread/corethread.h"

#include "s4noc.h"

#ifndef NR_CREDITS
#define NR_CREDITS 2
#endif

volatile _UNCACHED int started;
volatile _UNCACHED int done;
volatile _UNCACHED int result;
volatile _UNCACHED int time;

void work(void* arg) {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);
  int ts;
  int sum = 0;

  // get started, time to insert some credits before signaling start
  for (int i=0; i<NR_CREDITS; ++i ) {
    while (!s4noc[TX_FREE]) {;}
    s4noc[CREDIT_SLOT] = 1;
  }
  started = 1;

  // Wait for RX FIFO data available for first time stamp
  while (!s4noc[RX_READY]) {
    ;
  }
  ts = *timer_ptr;

  for (int i=0; i<LEN/BUF_LEN; ++i) {
    for (int j=0; j<BUF_LEN; ++j) {
      while (!s4noc[RX_READY]) {
        ;
      }
      sum += s4noc[IN_DATA];
      s4noc[CREDIT_SLOT] = 1;
    }
  }
  time = *timer_ptr - ts;
  result = sum;
  done = 1;
}

int main() {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);

  done = 0;
  result = 0;
  started = 0;

  // Receiver for time slot 0 depends in schedule, which depends on number of cores
  int rcv = get_cpucnt() == 4 ? 3 : 7;

  corethread_create(rcv, &work, NULL);

  int credit = 0;

  while (!started) {
    ;
  }

  for (int i=0; i<LEN/BUF_LEN; ++i) {
    for (int j=0; j<BUF_LEN; ++j) {
      // wait for a credit
      while (!s4noc[RX_READY]) {
        ;
      }
      s4noc[IN_DATA]; // consume it
      // wait for TX FIFO ready
      // without it is 24 clock cycles, this costs another 8 cycles
      // In this case we do not really need it, as we know there will be a free slot
      // for each received credit
/*
      while (!s4noc[TX_FREE]) {
        ;
      }
*/
      s4noc[SEND_SLOT] = 1;
    }
  }

  printf("Number of cores: %d\n", get_cpucnt());
  // now, after the print, we should be done
  if (done) {
    printf("%d sum in %d cycles, %g cycles per word\n", result, time, 1. * time/result);
  } else {
    printf("Not done\n");
  }
  // feed more tokens to get the consumer finished
  while (!done) {
    s4noc[SEND_SLOT] = 0;
  }
  printf("%d out of %d received\n", result, LEN);
}
