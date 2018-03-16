/*
    Small test program for shared SPMs with ownership.

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

// SPMs are placed every 64 KB (address bits n downto 16 are decoded)
// array counts in 32-bit integers
#define NEXT 0x10000/4

#define CNT 4

// The main function for the other threads on the another cores
void work(void* arg) {

  volatile _SPM int *sspm = (volatile _SPM int *) (0xE8000000);

  int id = get_cpuid();
  while (id != owner)
    ;
  for (int i=0; i<CNT; ++i) {
    sspm[CNT*id + i] = id*0x100 + i;
    sspm[CNT*id + i + NEXT] = id*0x1000 + i;
  }
  int val;
  for (int i=0; i<CNT; ++i) {
    val = sspm[CNT*id + i];
    if (id*0x100 + i != val) ok = 0;
    val = sspm[CNT*id + i + NEXT];
    if (id*0x1000 + i != val) ok = 0;
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
  printf("Test owner: wait\n");
  while(owner != 0)
    ;
  int id = get_cpuid();
  for (int i=0; i<CNT; ++i) {
    sspm[CNT*id + i] = id*0x100 + i;
    sspm[CNT*id + i + NEXT] = id*0x1000 + i;
  }
  int val;
  for (int i=0; i<CNT; ++i) {
    val = sspm[CNT*id + i];
    if (id*0x100 + i != val) ok = 0;
    val = sspm[CNT*id + i + NEXT];
    if (id*0x1000 + i != val) ok = 0;
  }
  // check one core's write data
  if (sspm[4] != 0x100) ok = 0;
  if (sspm[4+NEXT] != 0x1000) ok = 0;

  if (ok) {
    printf("Test ok\n");
  } else {
    printf("Test failed\n");
  }

  return 0;
}
