/*
    Small test program for a single shared SPM with ownership.

    Author: Martin Schoeberl
*/

#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>


#include "../../libcorethread/corethread.h"

// Shared data in main memory for the result
volatile _UNCACHED static int ok;
volatile _UNCACHED static int owner;

// The main function for the other threads on the another cores
void work(void* arg) {

  volatile _SPM int *sspm = (volatile _SPM int *) (0xE8000000);

  int id = get_cpuid();
  while (id != owner)
    ;
  for (int i=0; i<4; ++i) {
    sspm[4*id + i] = id*0x100 + i;
  }
  int val;
  for (int i=0; i<4; ++i) {
    val = sspm[4*id + i];
    if (id*0x100 + i != val) ok = 0;
  }

  if (id < get_cpucnt() - 1) {
    ++owner;
  } else {
    owner = 0;
  }

}

int main() {

  volatile _SPM int *sspm = (volatile _SPM int *) (0xE8000000);

  ok = 1;
  owner = 0; // start with myself

  for (int i=1; i<get_cpucnt(); ++i) {
    corethread_create(i, &work, NULL); 
  }
  // get first core working
  owner = 1;
  printf("Wait for finish\n");
  while(owner != 0)
    ;
  int id = get_cpuid();
  for (int i=0; i<4; ++i) {
    sspm[4*id + i] = id*0x100 + i;
  }
  int val;
  for (int i=0; i<4; ++i) {
    val = sspm[4*id + i];
    if (id*0x100 + i != val) ok = 0;
  }
  // check one core's write data
  if (sspm[4] != 0x100) ok = 0;

  if (ok) {
    printf("Test ok\n");
  } else {
    printf("Test failed\n");
  }

  return 0;
}
