/*
 * Testcase for debug macros
 *
 * Author: Rasmus Bo Sorensen (rasmus@rbscloud.dk)
 * Copyright: DTU, BSD License
 *
 */

const int NOC_MASTER = 0;
#include "libmp/mp.h"
#include "libcorethread/corethread.h"
#include "include/debug.h"


void slave1_func(void* arg) {
  ENQUEUE_MSG("Hello");
  MOVE_TO_SHMEM;
  int ret = 0;
  corethread_exit(&ret);
  return;
}



void slave2_func(void* arg) {
  ENQUEUE_MSG("world");
  MOVE_TO_SHMEM;
  int ret = 0;
  corethread_exit(&ret);
  return;
}


qpd_t onetotwo;
qpd_t twotoone;

int main(){
  puts("Core 0 started");
  mp_chan_init(&onetotwo,1,2,8,2);
  mp_chan_init(&twotoone,2,1,8,2);

  puts("Channels initialized");

  INIT_QUEUES;

  puts("Debug queues initialized");
  
  wait(10000);

  int slave_param = 0;
  int slave1 = 1;
  int slave2 = 2;
  corethread_create(slave1,&slave1_func,(void*)&slave_param);
  corethread_create(slave2,&slave2_func,(void*)&slave_param);
  ENQUEUE_MSG("Crap");
  MOVE_TO_SHMEM;

  puts("Corethreads started");

  //TODO: Print out debug messages from slaves
  PRINT_MSG;

  puts("Done printing debug info");

  int* res;
  corethread_join(slave1,&res);
  corethread_join(slave2,&res);

  return 0;
}
