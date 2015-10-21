/**
* PROGRAM DESCRIPTION:
*
* This is a test program for state based communication through the Argo NoC.
*
*/

/*
	Author: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
	Copyright: DTU, BSD License
*/
const int NOC_MASTER = 0;
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/lock.h>
#include <machine/patmos.h>
#include <limits.h>
#include "libcorethread/corethread.h"
#include "libmp/mp.h"

#define MASTER_CORE 0
#define SLAVE_CORE 2
#define ITERATIONS 100
#define CHAN_ID_ONE 1
#define CHAN_ID_TWO 2
#define SAMPLE_SIZE 128

#define DEBUG_ENABLE

unsigned long long int min_time = ULONG_MAX;
unsigned long long int max_time = 0;
unsigned long long int accum_time = 0;
unsigned long long int cnt_time = 0;

volatile unsigned int _UNCACHED done = 0;

void func_worker_1(void* arg) {
  mp_init();
  
  spd_t * sport2 = mp_create_sport(CHAN_ID_TWO,SINK,MASTER_CORE,SAMPLE_SIZE*sizeof(short));
  spd_t * sport1 = mp_create_sport(CHAN_ID_ONE,SOURCE,MASTER_CORE,SAMPLE_SIZE*sizeof(short));
  if (sport1 == NULL || sport2 == NULL) {
    exit(1);
  }
  volatile short _SPM * sample = mp_alloc(SAMPLE_SIZE*sizeof(short));

  for (int i = 0; i < SAMPLE_SIZE; ++i) {
    sample[i] = i;
  }

  mp_init_chans();


  while(done == 0);

  //for (int i = 0; i < ITERATIONS; ++i) {
  //    int ret = mp_read(sport2,sample);
  //    for (int i = 0; i < SAMPLE_SIZE; ++i) {
  //      if(sample[i] != i) {
  //        break;
  //      }
  //    }
  //    for (int i = 0; i < 100000; ++i)
  //    {
  //      asm volatile (""::);
  //    }
  //}

  //for (int i = 0; i < ITERATIONS/2; ++i) {
  for (int i = 0; i < ITERATIONS*20; ++i) {
    mp_write(sport1,sample);
    for (int i = 0; i < SAMPLE_SIZE; ++i) {
      sample[i] = i;
    }
    //for (int i = 0; i < 100000; ++i) {
    //  asm volatile (""::);
    //}
    mp_write(sport1,sample);
    for (int i = 0; i < SAMPLE_SIZE; ++i) {
      sample[SAMPLE_SIZE-1-i] = i;
    }
    //for (int i = 0; i < 100000; ++i) {
    //  asm volatile (""::);
    //}
  }

  
  
  int ret = 0;
  corethread_exit(&ret);
  return;
}

int main() {
  
  corethread_t worker_1 = SLAVE_CORE; // For now the core ID
     
  corethread_create(&worker_1,&func_worker_1,(void*)&worker_1);
  puts("Corethread created");

  unsigned short int local_phase = 0;
  min_time = ULONG_MAX;
  max_time = 0;
  accum_time = 0;
  cnt_time = 0;

  unsigned long long int start = 0;
  unsigned long long int stop = 0;

  spd_t * sport1 = mp_create_sport(CHAN_ID_ONE,SINK,SLAVE_CORE,SAMPLE_SIZE*sizeof(short));
  spd_t * sport2 = mp_create_sport(CHAN_ID_TWO,SOURCE,SLAVE_CORE,SAMPLE_SIZE*sizeof(short));
  if (sport1 == NULL || sport2 == NULL) {
    exit(1);
  }
  volatile short _SPM * sample = mp_alloc(SAMPLE_SIZE*sizeof(short));

  mp_init_chans();

  done = 1;

  int balance = 0;
//  for (int i = 0; i < SAMPLE_SIZE; ++i) {
//    sample[i] = i;
//  }
//  for (int i = 0; i < ITERATIONS/2; ++i) {
//    mp_write(sport2,sample);
//    for (int i = 0; i < SAMPLE_SIZE; ++i) {
//      sample[i] = i;
//    }
//  }
//
//  for (int i = 0; i < ITERATIONS/2; ++i) {
//    mp_write(sport2,sample);
//    for (int i = 0; i < SAMPLE_SIZE; ++i) {
//      sample[SAMPLE_SIZE-1-i] = i;
//    }
//  }



  for (int i = 0; i < ITERATIONS; ++i) {
    start = get_cpu_usecs();
    int ret = mp_read(sport1,sample);
    stop = get_cpu_usecs();
    if (ret == 0)
    {
      puts("No value written yet.");
    } else {
      unsigned long long int exe_time = stop - start;
      min_time = (exe_time < min_time) ? exe_time : min_time;
      max_time = (exe_time > max_time) ? exe_time : max_time;
      accum_time += exe_time;
      cnt_time++;
      if (sample[0] == 0) {
        balance++;
        for (int i = 0; i < SAMPLE_SIZE; ++i) {
          if(sample[i] != i) {
            printf("Error: sample[%i] = %i\n",i,sample[i]);
            break;
          }
        }
      } else if (sample[0] == SAMPLE_SIZE-1) {
        balance--;
        for (int i = 0; i < SAMPLE_SIZE; ++i) {
          if(sample[SAMPLE_SIZE-1-i] != i) {
            printf("Error: sample[%i] = %i\n",i,sample[i]);
            break;
          }
        }
      } else {
        printf("Wrong sample values sample[0] = %i\n",sample[0]);
      }
    }
  }

  printf("Local phase: %d\n",local_phase);
  
  inval_dcache();

  int* res;
  corethread_join(worker_1,&res);

  printf("Balance: %i\n",balance);
  printf("Min time: %llu\tMax time: %llu\tAccumulated time: %llu\nCount time: %llu\tAverage time: %llu\n", min_time,max_time,accum_time,cnt_time,accum_time/cnt_time);

  puts("Corethread joined");

  return *res;  
}

