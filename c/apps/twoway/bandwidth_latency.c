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

void latency_from_master_lean() {
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

void latency_from_master() {
  int start, end, val;

  // 1 for measurement, 1 for constant load cycle between the measurement
  const int const_measurement = 2;

  // printf("\n\n---- LATENCY TEST ----\n");

  // External read/write
  const int CNT = 1 << 10;
  const int TRYS = 1;
  // printf("Measuring min/max values over %d attempts\n", CNT);
  int min = 100;
  int max = 0;
  // printf("\n");
  for (int t = 1; t <= TRYS; t++) {
    // printf("-- Try #%d --\n", t);
    for (int i = 0; i < get_cpucnt(); i++) {
      volatile _SPM int *ptr =
          (volatile _IODEV int *)(mem_ptr + (i << BLOCKWIDTH));
      // printf("Core 0 -> Core %d:\n", i);
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
      // printf("Write:  Min: %d max: %d\n", min, max);
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
      // printf("Read:   Min: %d max: %d\n", min, max);
      // printf("\n");
      min = 100;
      max = 0;
    }
  }
}

#define write                                                                  \
  "swl  [%4] = %5;"                                                            \
  "swl	[%3] = %5;"                                                             \
  "swl	[%2] = %5;"

#define write2 write write
#define write4 write2 write2
#define write8 write4 write4
#define write16 write8 write8
#define write32 write16 write16
#define write64 write32 write32
#define write128 write64 write64
#define write256 write128 write128

void bandwidth_2x2() {
  int sum = 0;
  volatile int start, stop, val;
  start = *timer_ptr; // Use timer ptr so we can refer to it in inline assembly
                      // by name

  // printf("-- MAXIMUM BANDWITH --\n");
  // printf("\n");
  // printf("Write a word to all cores, following the schedule\n");
  // Hardcoded for 2x2 NoC

  _SPM int *mem_ptr_nv = (_IODEV int *)(0xE8000000);
  _SPM int *ptr1 = (_IODEV int *)(mem_ptr_nv + (1 << BLOCKWIDTH));
  _SPM int *ptr2 = (_IODEV int *)(mem_ptr_nv + (2 << BLOCKWIDTH));
  _SPM int *ptr3 = (_IODEV int *)(mem_ptr_nv + (3 << BLOCKWIDTH));

  // 256 times, write a word to each of the other 3 cores in the 2x2 network
  const int n = 256 * 3;
  int v = 10; // Value to be written

  // Timing is done inline to assure that the timer ptr is not spilled to stack
  asm("li	$r10 = timer_ptr;"
      "lwc	%0 = [$r10];" // start = *timer_ptr;
      "nop;"
      "lwl	%0 = [%0];"
      // We do not want the compiler to reload the volatile pointers each
      // iteration, so we manually state the sequence of writes
      write256

      "lwc	%1 = [$r10];"
      "nop;"
      "lwl	%1 = [%1];" // stop = *timer_ptr
      : "=r"(start), "=r"(stop)
      : "r"(ptr1), "r"(ptr2), "r"(ptr3), "r"(v)
      : "$r10" /* temporary timer ptr reg*/);

  val = stop - start;
  printf("%d %d \n", start, stop);
  printf("Bandwith: %f word/cycle\n", (n / (double)val));
}

int main() {
  // latency_from_master();
  bandwidth_2x2();

  // printf("Cummulative bandwidth of all nodes: %f\n");
}
