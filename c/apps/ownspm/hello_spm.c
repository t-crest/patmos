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
#define SHARED_SPM *((_iodev_ptr_t) PATMOS_IO_OWNSPM)

// Shared data in main memory for the result
volatile _UNCACHED static int ok;

// The main function for the other threads on the another cores
void work(void* arg) {

  _iodev_ptr_t sspm = (_iodev_ptr_t)PATMOS_IO_OWNSPM;

  int id = get_cpuid();
  for (int i=0; i<32; ++i) {
    sspm[32*id + i] = id*0x100 + i;
  }
  int val;
  for (int i=0; i<32; ++i) {
    val = sspm[32*id + i];
    if (id*0x100 + i != val) ok = 0;
  }

}

int main() {

  // Measure execution time with the clock cycle timer
  volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
  _iodev_ptr_t sspm = (_iodev_ptr_t) PATMOS_IO_OWNSPM;

  ok = 1;

  for (int i=1; i<get_cpucnt(); ++i) {
    corethread_create(i, &work, NULL); 
  }

  // back to back write - not really, needs some change
  sspm[0] = 0x123;
  sspm[1] = 0x456;
  int x = sspm[0];
  int y = sspm[1];
  if (x!=0x123 || y!=0x456) ok = 0;
  int id = get_cpuid();
  for (int i=0; i<32; ++i) {
    sspm[i] = id*0x100 + i;
  }
  int val;
  for (int i=0; i<32; ++i) {
    val = sspm[i];
    if (id*0x100 + i != val) ok = 0;
  }

  if (ok) {
    printf("Test ok\n");
  } else {
    printf("Test failed\n");
  }

  return 0;
}
