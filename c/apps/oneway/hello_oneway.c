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
#define ONEWAY_BASE *((volatile _SPM int *) 0xE8000000)
#define WORDS 4

// Shared data in main memory for the return value
volatile _UNCACHED static int field;
volatile _UNCACHED static int end_time;

// The main function for the other thread on the another core
void work(void* arg) {

  // Measure execution time with the clock cycle timer
  volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
  volatile _SPM int *mem_ptr = (volatile _IODEV int *) (0xE8000000);

  for (int i=0; i<CNT; ++i) {
    for (int j=0; j<WORDS; ++j) {
      *mem_ptr++ = ((i+1) << 8) + (j+1);
    }
  }
}

int main() {

  // Pointer to the deadline device
  volatile _IODEV int *dead_ptr = (volatile _IODEV int *) PATMOS_IO_DEADLINE;
  // Measure execution time with the clock cycle timer
  volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
  volatile _SPM int *mem_ptr = (volatile _IODEV int *) (0xE8000000);

  printf("Number of cores: %d\n", get_cpucnt());
  field = 42;
  corethread_create(1, &work, NULL); 
  printf("Field %d\n", field);
  for (int i=0; i<CNT*WORDS; ++i) {
    printf("%04x\n", *mem_ptr++);
  }

  return 0;
}
