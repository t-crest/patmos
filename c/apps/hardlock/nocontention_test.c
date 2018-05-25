#include "setup.h"

_UNCACHED int cpucnt = MAX_CORE_CNT;
_UNCACHED int acquisitions_avg[MAX_CORE_CNT];
_UNCACHED int acquisitions_max[MAX_CORE_CNT];
_UNCACHED int acquisitions_min[MAX_CORE_CNT];
_UNCACHED int releases_avg[MAX_CORE_CNT];
_UNCACHED int releases_max[MAX_CORE_CNT];
_UNCACHED int releases_min[MAX_CORE_CNT];
_UNCACHED int acquirels_avg[MAX_CORE_CNT];
_UNCACHED int acquirels_max[MAX_CORE_CNT];
_UNCACHED int acquirels_min[MAX_CORE_CNT];

const int shift = 2;
const int iter = 1 << shift;
const int MIN_START = 10000;

int _main()
{
  int coreid = get_cpuid();
  int lckid = coreid % MAX_LCK_CNT;
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
    __lock(lckid);
    stop2 = TIMER_CLK_LOW;
    __unlock(lckid);
    stop3 = TIMER_CLK_LOW;
    // The release first to prevent reordering the last time read
    release = (stop3 - stop2) - 1;
    release_avg += release;

    if(release > release_max)
      release_max = release;
    else if(release < release_min)
      release_min = release;

    acquire = (stop2 - stop1) - 1;
    acquire_avg += acquire;
    
    if(acquire > acquire_max)
      acquire_max = acquire;
    else if(acquire < acquire_min)
      acquire_min = acquire;
  }


  for(int i = 0; i < iter; i++)
  {
    asm("");
    stop1 = TIMER_CLK_LOW;
    __lock(lckid);
    __unlock(lckid);
    stop2 = TIMER_CLK_LOW;
    asm("");
    asm("");
    acquirel = (stop2 - stop1) - 1;
    acquirel_avg += acquirel;
    if(acquirel > acquirel_max)
      acquirel_max = acquirel;
    else if(acquirel < acquirel_min)
      acquirel_min = acquirel;
  }

  acquisitions_avg[coreid] = acquire_avg >> shift;
  acquisitions_max[coreid] = acquire_max;
  acquisitions_min[coreid] = acquire_min;
  releases_avg[coreid] = release_avg >> shift;
  releases_max[coreid] = release_max;
  releases_min[coreid] = release_min;
  acquirels_avg[coreid] = acquirel_avg >> shift;
  acquirels_max[coreid] = acquirel_max;
  acquirels_min[coreid] = acquirel_min;
  return 0;
}

void worker_func(void* arg) {
  int worker_param = *((int*)arg);
  int ret = _main();
  corethread_exit(&ret);
  return;
}

int main() {

  int threads[MAX_CORE_CNT];
  if(get_cpucnt() < cpucnt)
    cpucnt = get_cpucnt();

  printf("Starting %d cores\n",cpucnt);

  for(int i = 1; i < cpucnt; i++)
  {
    threads[i] = i;
    int worker_param = 1;
    corethread_create(threads[i],&worker_func,&worker_param);
  }

  int ret = _main();
  for(int i = 1; i < cpucnt; i++)
  {
    printf("Waiting for core %d\n", i);
    void * res;
    corethread_join(threads[i], &res);
  }

  printf("Iterations: %d\n", iter);
  printf("Acquisitions\n");
  printf("Average:\n");
  for (int i = 0; i < cpucnt; i++) {
    printf("%d\n", acquisitions_avg[i]);
  }
  printf("Max:\n");
  for (int i = 0; i < cpucnt; i++) {
    printf("%d\n", acquisitions_max[i]);
  }
  printf("Min:\n");
  for (int i = 0; i < cpucnt; i++) {
    printf("%d\n", acquisitions_min[i]);
  }
  printf("Releases\n");
  printf("Average:\n");
  for (int i = 0; i < cpucnt; i++) {
    printf("%d\n", releases_avg[i]);
  }
  printf("Max:\n");
  for (int i = 0; i < cpucnt; i++) {
    printf("%d\n", releases_max[i]);
  }
  printf("Min:\n");
  for (int i = 0; i < cpucnt; i++) {
    printf("%d\n", releases_min[i]);
  }
  printf("Acquisition and immediate releases:\n");
  printf("Average:\n");
  for (int i = 0; i < cpucnt; i++) {
    printf("%d\n", acquirels_avg[i]);
  }
  printf("Max:\n");
  for (int i = 0; i < cpucnt; i++) {
    printf("%d\n", acquirels_max[i]);
  }
  printf("Min:\n");
  for (int i = 0; i < cpucnt; i++) {
    printf("%d\n", acquirels_min[i]);
  }
  return ret;
}




