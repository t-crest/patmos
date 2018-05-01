/*
  Small test program for the distributed shared memory
*/

#include <stdio.h>
#include <machine/spm.h>
#include "../../libcorethread/corethread.h"

#define CNT 4
#define WORDS 128

#define LED ( *((volatile _IODEV unsigned *)0xF0090000))


#define DELAY_TIME 3000000

#define DELAY(time) for (volatile int i = time; i != 0; i--)

// The main function for the other threads on the other cores
void work(void* arg) {

  volatile _SPM int *mem = (volatile _SPM int *) (0xE8000000);

  int id = get_cpuid();
	
    // Initialize own value to cpuid, to be able to discern between
    // the three cores
    mem[id*WORDS] = id;

	while(1){
  	  //Slave cores write to their own memory.
        mem[id*WORDS] = mem[id*WORDS] + 1;
        DELAY(DELAY_TIME);
	}
}

int main() {

  volatile _SPM int *rxMem = (volatile _SPM int *) (0xE8000000);

  for (int i=1; i<get_cpucnt(); ++i) {
    corethread_create(i, &work, NULL); 
  }



  printf("\n");
  printf("Number of cores: %d\n", get_cpucnt());

  //Print content of all the slaves memory.
  for (int i=0; i<CNT; ++i) {
    for (int j=0; j<4; ++j) {
      printf("%08x\n", rxMem[i*WORDS + j]);
    }
  }

    while(1){
        for (int i=1; i<CNT; ++i) {
            printf("Core:%d, %08x\n", i,rxMem[i*WORDS]);
        }
        DELAY(DELAY_TIME);
        printf("\n");
    }

  return 0;
}
