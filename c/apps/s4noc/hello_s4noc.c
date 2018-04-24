/*
  Small test program for the S4NOC.

  Author: Martin Schoeberl
*/

#include <stdio.h>
#include <machine/spm.h>
#include "../../libcorethread/corethread.h"

#include "s4noc.h"

volatile _UNCACHED int done;

void work(void* arg) {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);

  // Wait for RX FIFO data available
  for(;;) {
    if (s4noc[RX_READY]) break;
  }
  int val = s4noc[IN_DATA];
  if (val == 0xcafebabe) {
    done = 1;
  }
}

int main() {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);

  done = 0;

  corethread_create(RCV, &work, NULL);

  // wait for TX FIFO ready
  for(;;) {
    int status = s4noc[2]; 
    if (status & 1) break;
  }
  s4noc[0] = 0xcafebabe; // into time slot 0 that goes to core 3

  printf("Number of cores: %d\n", get_cpucnt());
  // now, after the print, we should be done
  if (done) {
    printf("0xcafebabe received\n");
    return 0;
  } else {
    printf("Not done\n");
    return 1;
  }
}
