//#include <string.h>
#include <stdio.h>
#include <machine/patmos.h>
#include <machine/spm.h>
#include "libcorethread/corethread.h"
#include "crlu.h"
#include <include/patio.h>
#define MAX_CNT 20


_UNCACHED char data[MAX_CNT] = "AAAAAAAAAAAAAAAAAAAA";
_UNCACHED volatile int cnt;
_UNCACHED int acquisitions[MAX_CNT];
_UNCACHED int releases[MAX_CNT];
_UNCACHED int acquirels[MAX_CNT];

int _main()
{
  int id = get_cpuid();
  int cnt = get_cpucnt();
  const int shift = 10;
  const int iter = 1 << shift;
  int acquire = 0;
  int release = 0;
  int acquirel = 0;
  
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

  acquisitions[id] = acquire >> shift;
  releases[id] = release >> shift;
  acquirels[id] = acquirel >> shift;
  data[id] = 'F';
  if(id == 0) {
    for (int i = 0; i < cnt; i++) {
      printf("Waiting for core %d\n", i);
      while(data[i] != 'F') {
        asm volatile("");
      }
    }
    printf("Iterations: %d\n", iter);
    printf("Average cycles for\n");
    printf("Acquisitions:\n");
    for (int i = 0; i < cnt; i++) {
      printf("%d\n", acquisitions[i]);
    }
    printf("Releases:\n");
    for (int i = 0; i < cnt; i++) {
      printf("%d\n", releases[i]);
    }
    printf("Acquisition and immediate releases:\n");
    for (int i = 0; i < cnt; i++) {
      printf("%d\n", acquirels[i]);
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




