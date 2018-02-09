//#include <string.h>
#include <stdio.h>
#include <machine/patmos.h>
#include <machine/spm.h>
#include "libcorethread/corethread.h"
#include "hardlock.h"
#define MAX_CNT 20

#define TIMER_CLK_LOW *((volatile _IODEV int *) (PATMOS_IO_TIMER + 0x4))
#define TIMER_US_LOW *((volatile _IODEV int *) (PATMOS_IO_TIMER + 0xc))


_UNCACHED char data[MAX_CNT];
_UNCACHED int acquisitions_avg[MAX_CNT];
_UNCACHED int acquisitions_max[MAX_CNT];
_UNCACHED int acquisitions_min[MAX_CNT];
_UNCACHED int releases_avg[MAX_CNT];
_UNCACHED int releases_max[MAX_CNT];
_UNCACHED int releases_min[MAX_CNT];
_UNCACHED int acquirels_avg[MAX_CNT];
_UNCACHED int acquirels_max[MAX_CNT];
_UNCACHED int acquirels_min[MAX_CNT];

int _main()
{
  int id = get_cpuid();
  int cnt = get_cpucnt();
  const int shift = 10;
  const int iter = 1 << shift;
  const int MIN_START = 10000;
  int acquire = 0;
  int acquire_avg = 0;
  int acquire_max = 0;
  int acquire_min = MIN_START;
  int release_avg = 0;
  int release = 0;
  int release_max = 0;
  int release_min = MIN_START;
  int acquirel = 0;
  int acquirel_avg = 0;
  int acquirel_max = 0;
  int acquirel_min = MIN_START;
  
  int stop1;
  int stop2;
  int stop3;

  for(int i = 0; i < iter; i++)
  {

    stop1 = TIMER_CLK_LOW;
    lock(id);
    stop2 = TIMER_CLK_LOW;
    unlock(id);
    stop3 = TIMER_CLK_LOW;
    // The release first to prevent reordering the last time read
    release = stop3 - stop2;
    release_avg += release;

    if(release > release_max)
      release_max = release;
    else if(release < release_min)
      release_min = release;

    acquire = stop2 - stop1;
    acquire_avg += acquire;
    
    if(acquire > acquire_max)
      acquire_max = acquire;
    else if(acquire < acquire_min)
      acquire_min = acquire;
    
    
  }

  for(int i = 0; i < iter; i++)
  {
    
    stop1 = TIMER_CLK_LOW;
    lock(id);
    unlock(id);
    stop2 = TIMER_CLK_LOW;

    acquirel = stop2 - stop1;
    acquirel_avg += acquirel;
    if(acquirel > acquirel_max)
      acquirel_max = acquirel;
    else if(acquirel < acquirel_min)
      acquirel_min = acquirel;
  }

  acquisitions_avg[id] = acquire_avg >> shift;
  acquisitions_max[id] = acquire_max;
  acquisitions_min[id] = acquire_min;
  releases_avg[id] = release_avg >> shift;
  releases_max[id] = release_max;
  releases_min[id] = release_min;
  acquirels_avg[id] = acquirel_avg >> shift;
  acquirels_max[id] = acquirel_max;
  acquirels_min[id] = acquirel_min;
  data[id] = 'F';
  if(id == 0) {
    for (int i = 0; i < cnt; i++) {
      printf("Waiting for core %d\n", i);
      while(data[i] != 'F') {
        asm volatile("");
      }
    }
    printf("Iterations: %d\n", iter);
    printf("Acquisitions\n");
    printf("Average:\n");
    for (int i = 0; i < cnt; i++) {
      printf("%d\n", acquisitions_avg[i]);
    }
    printf("Max:\n");
    for (int i = 0; i < cnt; i++) {
      printf("%d\n", acquisitions_max[i]);
    }
    printf("Min:\n");
    for (int i = 0; i < cnt; i++) {
      printf("%d\n", acquisitions_min[i]);
    }
    printf("Releases\n");
    printf("Average:\n");
    for (int i = 0; i < cnt; i++) {
      printf("%d\n", releases_avg[i]);
    }
    printf("Max:\n");
    for (int i = 0; i < cnt; i++) {
      printf("%d\n", releases_max[i]);
    }
    printf("Min:\n");
    for (int i = 0; i < cnt; i++) {
      printf("%d\n", releases_min[i]);
    }
    printf("Acquisition and immediate releases:\n");
    printf("Average:\n");
    for (int i = 0; i < cnt; i++) {
      printf("%d\n", acquirels_avg[i]);
    }
    printf("Max:\n");
    for (int i = 0; i < cnt; i++) {
      printf("%d\n", acquirels_max[i]);
    }
    printf("Min:\n");
    for (int i = 0; i < cnt; i++) {
      printf("%d\n", acquirels_min[i]);
    }
  }
  return 0;
}

void worker_func(void* arg) {
  int worker_param = *((int*)arg);
  int ret = _main();
  corethread_exit(&ret);
  return;
}

int main() {

  int threads[MAX_CNT];
  int len = sizeof threads / sizeof *threads;
  if(get_cpucnt() < len)
    len = get_cpucnt();

  printf("Starting cores\n");

  for(int i = 1; i < len; i++)
  {
    threads[i] = i;
    int worker_param = 1;
    corethread_create(threads[i],&worker_func,&worker_param);
  }

  int ret = _main();
  for(int i = 1; i < len; i++)
  {
    void * res;
    corethread_join(threads[i], &res);
  }
  return ret;
}




