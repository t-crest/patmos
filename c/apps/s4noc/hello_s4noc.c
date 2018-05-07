/*
  Small test program for the S4NOC.

  Core 0 sends in the first time slot. All other cores wait to receive and
  the single receiving core writes its id into the global variable done.
  Should work for any configuration.

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
    done = get_cpuid();
  }
}

int main() {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);

  done = 0;
  for (int i=1; i<get_cpucnt(); ++i) {
    corethread_create(i, &work, NULL);
  }

  // wait for TX free
  while (!s4noc[TX_FREE]) ;

  s4noc[0] = 0xcafebabe; // send at time slot 0 (goes to core 3 in a 2x2 NoC
                         // and to 7 in a 3x3 NoC)

  printf("Number of cores: %d\n", get_cpucnt());
  // now, after the print, we should have some result
  if (done) {
    printf("0xcafebabe received by core %d\n", done);
    return 0;
  } else {
    printf("Not done\n");
    return 1;
  }
}
