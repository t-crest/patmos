/**
* PROGRAM DESCRIPTION:
*
* This is an test case for the sampling message passing in libmp.
*
* The test sends the usec time from the slave core to the master core.
* The master core then checks the time difference between the local
* time and the time received from the clave core.
*
*
*        __________           _________
*        |        |           |       |
*        | Core 0 | <-------- | Slave |
*        |________|           |_______|
*                            
*
*/

/*
	Author: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
	Copyright: DTU, BSD License
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include "libcorethread/corethread.h"
#include "libmp/mp.h"
#include "include/debug.h"

#define MP_CHAN_1_ID 1
#define MP_CHAN_1_MSG_SIZE (sizeof(unsigned long long))
#define RUNTIME 1000000

volatile _UNCACHED int slave = 0;

void func_worker_1(void* arg) {
  // Cast and load the parameter
  int worker_1_param = *((int*)arg);
  // Create the queuing ports
  spd_t * chan = mp_create_sport(MP_CHAN_1_ID, SOURCE, MP_CHAN_1_MSG_SIZE);
  if (chan == NULL) {
    DEBUGF(chan);
    abort();
  }
  volatile unsigned long long _SPM * time_sample = mp_alloc(MP_CHAN_1_MSG_SIZE);
  // Initialize the communication channels
  int retval = mp_init_ports();
  // TODO: check on retval

  slave= 1;

  for (unsigned long long start = get_cpu_usecs(); start + RUNTIME > get_cpu_usecs(); ) {
    *time_sample = get_cpu_usecs();
    mp_write(chan,time_sample);
  }

  int ret = 0;
  corethread_exit(&ret);
  return;
}

int main() {

  puts("Master");
  int worker_1 = 1; // For now the core ID
  int worker_1_param = 1;

  corethread_create(worker_1,&func_worker_1,(void*)&worker_1_param);

  // Create the queuing ports
  spd_t * chan = mp_create_sport(MP_CHAN_1_ID, SINK, MP_CHAN_1_MSG_SIZE);

  volatile unsigned long long _SPM * time_sample = mp_alloc(MP_CHAN_1_MSG_SIZE);
  
  if (chan == NULL || time_sample == NULL) {
    DEBUGF(chan);
    abort();
  }

  // Initialize the communication channels
  int retval = mp_init_ports();
  // TODO: check on retval

  puts("Initialized ports");

  while(slave != 1){;}

  puts("Slave is ready");

  unsigned long long min_time_diff = -1;
  unsigned long long max_time_diff = 0;
  unsigned long long accum_time_diff = 0;
  unsigned long long cnt_time_diff = 0;
  unsigned long long percent = 0;
  int done = 0;
  unsigned long long start = get_cpu_usecs();
  while(!done){
    int success = mp_read(chan,time_sample);
    unsigned long long time_diff = get_cpu_usecs() - (*time_sample);
    if (success == 0) {
      printf("No sample received\n");
    } else if ((*time_sample) == 0) {
      printf("Received empty sample, newest: %u, sample size: %u\n",chan->newest,chan->sample_size);
    } else {
      if (time_diff > 2000 ) {
        // Time difference is larger than a micro second
        printf("Time sample: %llu\tdiff: %llu\n",*time_sample,time_diff);
      }
      cnt_time_diff++;
      if (time_diff < min_time_diff) {
        min_time_diff = time_diff;
      }
      if (time_diff > max_time_diff) {
        max_time_diff = time_diff;
      }
      accum_time_diff += time_diff;
    }

    if (start + percent < get_cpu_usecs()) {
      percent += RUNTIME/10;
      printf("+");
      fflush(stdout);
    }
    if ( start + RUNTIME < get_cpu_usecs())  {
      done = 1;
    }
  }
  printf("\n");

  printf("Status:\n\tMin time diff: %llu\n\tMax time diff: %llu\n\tAvg time diff: %llu\n", min_time_diff,max_time_diff,accum_time_diff/cnt_time_diff);

  int* res;
  corethread_join(worker_1,&res);

  return *res;  
}

