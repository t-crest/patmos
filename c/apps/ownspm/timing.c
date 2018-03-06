/*
    Test the access time of the shared SPM.

    Author: Martin Schoeberl
*/

#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>


#include "../../libcorethread/corethread.h"

unsigned _SPM *data_spm = SPM_BASE;

#define CNT 20

void do_delay_times() {
  // delay times
  for (int i=0; i<CNT; ++i) {
    data_spm[i] = rand() & 0xff;
  }
}

int main() {

  // Pointer to the deadline device
  volatile _IODEV int *dead_ptr = (volatile _IODEV int *) PATMOS_IO_DEADLINE;
  // Measure execution time with the clock cycle timer
  volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
  volatile _SPM int *spm = (volatile _SPM int *) (0xE8000000);

  int start, end;
  int val;
  int acc, add;
  int min = 10000;
  int max = 0;

  // To avoid compiler optimizing all code away a result
  acc = 0;

  start = *timer_ptr;
  val = *timer_ptr-start;
  printf("Empty measurement %d\n", val);

  do_delay_times();
  printf("Access with a random delay\n");
  for (int i=0; i<CNT; ++i) {
    *dead_ptr = data_spm[i];
    val = *dead_ptr; // delay by a random value
    // minimum is 4 clock cycles on the shared SPM. Why? Should be just 2.
    // Constant access time of 3 (instead of 1) cycles to the OwnSPM.
    // This code is probably not using a single load instruction.
    start = *timer_ptr;
    add = *spm;
    val = *timer_ptr-start;
    acc += add;
    --val; // remove 1 cycle for the measurement overhead
    printf("%d ", val);
    if (min>val) min = val;
    if (max<val) max = val;
  }
  printf("\n");

  printf("Min: %d max: %d\n", min, max);
  printf("Acc result %d\n", acc);
  return 0;
}
