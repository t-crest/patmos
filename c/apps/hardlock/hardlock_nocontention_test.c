//#include <string.h>
#include <stdio.h>
#include <machine/patmos.h>
#include <machine/spm.h>
#include "libcorethread/corethread.h"
#include "hardlock.h"
#define MAX_CNT 20

#define TIMER_CLK_LOW *((volatile _IODEV int *) (PATMOS_IO_TIMER + 0x4))
#define TIMER_US_LOW *((volatile _IODEV int *) (PATMOS_IO_TIMER + 0xc))


_UNCACHED char data[MAX_CNT] = "AAAAAAAAAAAAAAAAAAAA";
_UNCACHED volatile int cnt;
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
  int acquire = 0;
  int acquire_max = 0;
  int acquire_min = 0;
  int release = 0;
  int release_max = 0;
  int release_min = 0;
  int acquirel = 0;
  int acquirel_max = 0;
  int acquirel_min = 0;
  
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
    acquire += stop2 - stop1;
    release += stop3 - stop2;
  }

  for(int i = 0; i < iter; i++)
  {
    
    stop1 = TIMER_CLK_LOW;
    lock(id);
    unlock(id);
    stop2 = TIMER_CLK_LOW;
    acquirel += stop2 - stop1;
  }

  acquisitions_avg[id] = acquire >> shift;
  acquisitions_max[id] = acquire_max;
  acquisitions_min[id] = acquire_min;
  releases_avg[id] = release >> shift;
  releases_max[id] = release_max;
  releases_min[id] = release_min;
  acquirels_avg[id] = acquirel >> shift;
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




