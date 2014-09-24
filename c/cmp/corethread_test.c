/**
* PROGRAM DESCRIPTION:
*
* This is an example program for using the libcorethread (corethread library).
*
*/

/*
	Author: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
	Copyright: DTU, BSD License
*/
const int NOC_MASTER = 0;
#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include "libcorethread/corethread.h"

// The starting address of the message passing structure for channel 1
#define WORKER_1 NOC_SPM_BASE
#define WORKER_1_SIZE sizeof(corethread_t)


int param_worker_1 = 5;

void func_worker_1(void *arg) {

}

int main() {
  corethread_worker();
  volatile corethread_t _SPM *worker_1;
  corethread_init(worker_1,1,NOC_SPM_BASE);

  puts("Master");

  corethread_create(worker_1,POLLING,&func_worker_1,&param_worker_1);

  return 0;  
}

