/*
    Small test program for the shared SPM

    Author: Martin Schoeberl
*/

#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>


#include "../../libcorethread/corethread.h"

#define CNT 4
#define SHARED_SPM *((volatile _SPM int *) 0xE8000000)

// Shared data in main memory for the return value
volatile _UNCACHED static int field;
volatile _UNCACHED static int end_time;

// The main function for the other threads on the another cores
void work(void* arg) {

}

int main() {

  // Measure execution time with the clock cycle timer
  volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
  volatile _SPM int *sspm = (volatile _SPM int *) (0xE8000000);

/*
  for (int i=1; i<get_cpucnt(); ++i) {
    corethread_create(i, &work, NULL); 
  }
*/

  printf("Number of cores: %d\n", get_cpucnt());
  for (int i=0; i<CNT; ++i) {
    sspm[i] = 0x100 * i;
  }
  for (int i=0; i<CNT; ++i) {
    printf("%08x\n", sspm[i]);
  }
  return 0;
}
