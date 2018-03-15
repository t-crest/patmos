/*
    This is a multicore test program that shall be loaded by a bootloader.

    Author: 
    Copyright: DTU, BSD License
*/

#include "include/patio.h"
#include "libcorethread/corethread.h"
#include <machine/spm.h>

#define MAX 20
#define LENGTH 1024
#define SPM ((volatile _SPM int *)0xE8000000)

volatile int _SPM *spm_ptr = SPM;

// The main function for the other threads on the another cores
void work(void *arg) {
  int val = *((int *)arg);
  
  int id = get_cpuid();
  int cnt = get_cpucnt();

   //write
  *(spm_ptr+id) = id;
  return;
}

int main() {

  unsigned i;

  int id = get_cpuid(); // id=0
  int cnt = get_cpucnt();

  printf("Total %d Cores\n",get_cpucnt()); // print core count

  for (i=1; i<cnt; ++i) {
    int core_id = i; // The core number

    printf("The core_id is %d\n",core_id); // print core_id number
    int parameter = 1; // dummy
    corethread_create(core_id, &work, &parameter); 
    
  }

  for(i=1; i<cnt; ++i) { 
    int parameter = 1;
    corethread_join(i,&parameter);
    printf("The Core O is reading %d \n", *(spm_ptr+i)); 

  }

}
