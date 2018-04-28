/*
  Small test program for the One-Way Shared Memory.

  Fills TX memories with core specific patterns and reads out
  the RX memory of core 0 to see which blocks of data come from where.

  Author: Martin Schoeberl
*/

#include <stdio.h>
#include <machine/spm.h>
#include "../../libcorethread/corethread.h"

#define CNT 4
#define WORDS 256

// The main function for the other threads on the other cores
void work(void* arg) {

  volatile _SPM int *txMem = (volatile _SPM int *) (0xE8000000);

  int id = get_cpuid();
  //Slave cores write to their own memory.
  for(int i = 0; i < CNT; i++){
    txMem[id*CNT + i] = id*0x10000 + 0x100 + i;
  }
}

int main() {

  volatile _SPM int *rxMem = (volatile _SPM int *) (0xE8000000);

  for (int i=1; i<get_cpucnt(); ++i) {
    corethread_create(i, &work, NULL); 
  }



  printf("\n");
  printf("Number of cores: %d\n", get_cpucnt());

  //Print content off all the slaves memory.

  for (int i=0; i<CNT; ++i) {
    for (int j=0; j<4; ++j) {
      printf("%08x\n", rxMem[i*CNT + j]);
    }
  }

  return 0;
}
