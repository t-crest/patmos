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

#define ONEWAY_BASE *((volatile _SPM int *) 0xE8000000)
#define BLOCKWIDTH 8

#define TOKEN 0x42

#define N_TOKENS 3 // Number of times a core sees a token

// Shared data in main memory for the return value
volatile _UNCACHED static int field;
volatile _UNCACHED static int end_time;

// The main function for the other thread on the another core
void work(void* arg) {

  // Measure execution time with the clock cycle timer
  volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
  volatile _SPM int *mem_ptr = (volatile _IODEV int *) (0xE8000000);
    
    volatile _SPM int* readPtr = mem_ptr + (get_cpuid() << BLOCKWIDTH);
    volatile _SPM int* lock = readPtr + 1;

    //1 reads from 3, 2 from 1 and 3 from 2
    int partnerID = get_cpuid() - 1 == 0 ? get_cpucnt() - 1 : get_cpuid() - 1;
    volatile _SPM int* p_readPtr = mem_ptr + (partnerID << BLOCKWIDTH);
    volatile _SPM int* p_lock = readPtr + 1;

    if(get_cpuid() == 1){
        // Inject token inside core 1 @ 0b0100
        *readPtr = TOKEN;
    }
    
    int tokenCounter = 0;
    while(tokenCounter != N_TOKENS){

        while(*p_readPtr != TOKEN){
            // wait until we observe our token
        }
        *p_lock = 1; // Lock  the partner core
        *p_readPtr = 0; // reset the token

        // Bad utilization? wait until lock has been written to partner
        while(*p_lock != 1){

        }
        // partner is locked, write token to ourself and release our lock
        *readPtr = TOKEN;
        *lock = 0;

        // Increment token counter by 1:
        tokenCounter++;
    }

    
    return;
}

int main() {

  printf("Number of cores: %d\n", get_cpucnt());
  int parameter = 0;
  for(int i = 1; i < get_cpucnt(); i++){
    corethread_create(i, (void*)&work, (void*)&parameter); 
  }

  int* param;
  for(int i = 1; i < get_cpucnt(); i++){
      corethread_join(i, &param);
  }

  volatile _SPM int *mem_ptr = (volatile _IODEV int *) (0xE8000000);
  for(int i = 1; i < get_cpucnt(); i++){
      // Print values in cores
      printf("Core %d:\n", i);
      printf("%04x = %04x\n", (int)mem_ptr | (i << BLOCKWIDTH), *(volatile _SPM int*)((int)mem_ptr | (i << BLOCKWIDTH)));
      printf("%04x = %04x\n", (int)mem_ptr | (i << BLOCKWIDTH) | 1, *(volatile _SPM int*)((int)mem_ptr | (i << BLOCKWIDTH) | 1));
      printf("\n");
  }

  return 0;
}
