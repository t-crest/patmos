/*
  Small test program for the S4NOC. The (slot) constants are for the 2x2 configuration.

  Author: Martin Schoeberl
*/

#include <stdio.h>
#include <machine/spm.h>
#include "../../libcorethread/corethread.h"

#include "s4noc.h"

volatile _UNCACHED int done;

void work(void* arg) {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);

  // Wait for RX data available
  while (!s4noc[RX_READY]) ;

  int val = s4noc[IN_DATA];
  if (val == 0xcafebabe) {
    done = 1;
  }
}

int main() {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);

  done = 0;

  corethread_create(RCV, &work, NULL);

  // wait for TX free
  while (!s4noc[TX_FREE]) ;

  s4noc[0] = 0xcafebabe; // send at time slot 0 that goes to core 3

  printf("Number of cores: %d\n", get_cpucnt());
  // now, after the print, we should have some result
  if (done) {
    printf("0xcafebabe received\n");
    return 0;
  } else {
    printf("Not done\n");
    return 1;
  }
}
