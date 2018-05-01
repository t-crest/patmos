/*
  Producer/consumer example for the S4NOC.

  Author: Martin Schoeberl
*/

#include <stdio.h>
#include <machine/spm.h>
#include "../../libcorethread/corethread.h"

#include "s4noc.h"

// #define CREDITS 1

#define WAIT_ON_START 1

volatile _UNCACHED int started;
volatile _UNCACHED int done;
volatile _UNCACHED int result;
volatile _UNCACHED int time;

void work(void* arg) {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);
  int ts;
  int sum = 0;

#ifdef WAIT_ON_START
  // get started, time to insert some credits before signaling start
  started = 1;
#endif

  // Wait for RX FIFO data available for first time stamp
  while (!s4noc[RX_READY]) {
    ;
  }
  ts = *timer_ptr;

  int credit = 0;

  for (int i=0; i<LEN/BUF_LEN; ++i) {
    for (int j=0; j<BUF_LEN; ++j) {
      while (!s4noc[RX_READY]) {
        ;
      }
      sum += s4noc[IN_DATA];
#ifdef CREDITS
      ++credit;
      if (credit == HANDSHAKE) {
        credit = 0;
        s4noc[CREDIT_SLOT] = 13;
      } 
#endif
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

  corethread_create(RCV, &work, NULL);

  int credit = 0;

#ifdef WAIT_ON_START
  while (!started) {
    ;
  }
#endif

  for (int i=0; i<LEN/BUF_LEN; ++i) {
    for (int j=0; j<BUF_LEN; ++j) {
      // wait for TX FIFO ready
      while (!s4noc[TX_FREE]) {
        ;
      }
      s4noc[SEND_SLOT] = 1;
#ifdef CREDITS
      ++credit;
      // wait for consumers credit
      if (credit == HANDSHAKE) {
        credit = 0;
        while (!s4noc[RX_READY]) {
          ;
        }
        s4noc[IN_DATA]; // consume it
      }
#endif
    }
  }

  printf("Number of cores: %d\n", get_cpucnt());
  // now, after the print, we should be done
  if (done) {
    printf("%d sum in %d cycles\n", result, time);
  } else {
    printf("Not done\n");
  }
  // feed more tokens to get the consumer finished
  while (!done) {
    s4noc[SEND_SLOT] = 0;
  }
  printf("%d out of %d received\n", result, LEN);
}
