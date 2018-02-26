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

// for faster debugging
#include "patio.h"

void print_hex(int val) {
  for (int j=0; j<8; ++j) {
    int c = XDIGIT((val >> (4 * (7-j))) & 0x0f);
    WRITECHAR(c);
  }
  WRITECHAR('\n');
}


#define CNT 4
#define ONEWAY_BASE *((volatile _SPM int *) 0xE8000000)
#define WORDS 4

// Shared data in main memory for the return value
volatile _UNCACHED static int field;
volatile _UNCACHED static int end_time;

// The main function for the other threads on the another cores
void work(void* arg) {

  // Measure execution time with the clock cycle timer
  volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
  volatile _SPM int *mem_ptr = (volatile _IODEV int *) (0xE8000000);

  *mem_ptr = 0xabcd;

  int id = get_cpuid();
  for (int i=0; i<CNT; ++i) {
    for (int j=0; j<WORDS; ++j) {
      *mem_ptr++ = id*0x100 + i*0x10 + j;
    }
  }
}

int main() {

  // Pointer to the deadline device
  volatile _IODEV int *dead_ptr = (volatile _IODEV int *) PATMOS_IO_DEADLINE;
  // Measure execution time with the clock cycle timer
  volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
  volatile _SPM int *mem_ptr = (volatile _IODEV int *) (0xE8000000);

  int result[CNT*WORDS];

  for (int i=1; i<get_cpucnt(); ++i) {
    corethread_create(i, &work, NULL); 
  }
  printf("Number of cores: %d\n", get_cpucnt());
  for (int i=0; i<CNT*WORDS; ++i) {
    result[i] = *(mem_ptr++);
    // print_hex(*(mem_ptr++));
  }
  for (int i=0; i<CNT*WORDS; ++i) {
    printf("%04x\n", result[i]);
  }
  return 0;
}
