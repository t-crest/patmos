/*
    Small test program for the One-Way Shared Memory

    Author: Martin Schoeberl
    Copyright: DTU, BSD License
*/

#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>

#include "../../libcorethread/corethread.h"

#define CNT 4

// Shared data in main memory for the return value
volatile _UNCACHED static int field;
volatile _UNCACHED static int end_time;

// The main function for the other thread on the another core
void work(void* arg) {

  // Measure execution time with the clock cycle timer
  volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);

  field = 13;
}

int main() {

  // Pointer to the deadline device
  volatile _IODEV int *dead_ptr = (volatile _IODEV int *) PATMOS_IO_DEADLINE;
  // Measure execution time with the clock cycle timer
  volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);

  printf("Number of cores: %d\n", get_cpucnt());
  field = 42;
  corethread_create(1, &work, NULL); 
  printf("Field %d\n", field);
  printf("Field %d\n", field);

  return 0;
}
