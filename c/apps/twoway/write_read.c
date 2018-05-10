/*
    Small test program for the One-Way Shared Memory
    Author: Martin Schoeberl
    Copyright: DTU, BSD License
*/

#include <machine/patmos.h>
#include <machine/spm.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../libcorethread/corethread.h"

#define BLOCKWIDTH 7

// Pointer to the deadline device
volatile _IODEV int *dead_ptr = (volatile _IODEV int *)PATMOS_IO_DEADLINE;
// Measure execution time with the clock cycle timer
volatile _IODEV int *timer_ptr = (volatile _IODEV int *)(PATMOS_IO_TIMER + 4);

volatile _SPM int *mem_ptr = (volatile _IODEV int *)(0xE8000000);

void measure_from_master_lean() {
  int start, end, val;
  for (int i = 0; i < get_cpucnt(); i++) {
    volatile _SPM int *ptr =
        (volatile _IODEV int *)(mem_ptr + (i << BLOCKWIDTH));
    *(ptr) = 10;
  }

  for (int i = 0; i < get_cpucnt(); i++) {
    volatile _SPM int *ptr =
        (volatile _IODEV int *)(mem_ptr + (i << BLOCKWIDTH));
    val = *ptr;
  }
}

void randDelay() {
  for (volatile int i = rand() % 50; i != 0; i--) {
  }
}

void measure_from_master() {
  int start, end, val;

  // 1 for measurement, 1 for constant load cycle between the measurement
  const int const_measurement = 2;

  printf("\n\n---- TIMING TEST ----\n");

  printf("\n---- EXTERNAL MEASUREMENTS ----\n");

  // External read/write
  const int CNT = 1 << 10;
  const int TRYS = 1;
  printf("Measuring min/max values over %d attempts\n", CNT);
  int min = 100;
  int max = 0;
  printf("\n");
  for (int t = 1; t <= TRYS; t++) {
    printf("-- Try #%d --\n", t);
    for (int i = 0; i < get_cpucnt(); i++) {
      volatile _SPM int *ptr =
          (volatile _IODEV int *)(mem_ptr + (i << BLOCKWIDTH));
      printf("master to core %d:\n", i);
      // External write
      for (int k = 0; k < CNT; k++) {
        randDelay();
        start = *timer_ptr;
        *(ptr) = 10;
        val = *timer_ptr - start;
        if (min > val)
          min = val;
        if (max < val)
          max = val;
      }
      printf("Write:  Min: %d max: %d\n", min, max);
      min = 100;
      max = 0;

      // External read
      for (int k = 0; k < CNT; k++) {
        randDelay();
        start = *timer_ptr;
        val = *(ptr);
        val = *timer_ptr - start;
        if (min > val)
          min = val;
        if (max < val)
          max = val;
      }
      printf("Read:   Min: %d max: %d\n", min, max);
      printf("\n");
      min = 100;
      max = 0;
    }
  }
}

int main() { measure_from_master(); }
