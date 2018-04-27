#include "setup.h"

#ifndef MAX_CNT
#define MAX_CNT 10000
#endif
#ifndef WAIT
#define WAIT 10
#endif

_UNCACHED int cpucnt = MAX_CORE_CNT;
_UNCACHED int data[MAX_LCK_CNT];
_UNCACHED int cnt1 = MAX_CNT;
_UNCACHED int cnt2 = MAX_CNT;
_UNCACHED int sync = 0;

int _main()
{
  int cpuid = get_cpuid();
  int stop;
  for(int i = 1; i <= MAX_LCK_CNT; i++)
  {
    __lock(0);
    if(sync == cpucnt-1)
    {
      for (int j = 0; j < MAX_LCK_CNT; j++) {
        data[j] = 0;
      }
      cnt1 = MAX_CNT;
      cnt2 = MAX_CNT;
    }
    sync++;
    __unlock(0);
    while(sync < cpucnt) {asm("");}

    if(cpuid == 0)
      stop = TIMER_CLK_LOW;
    while(1)
    {
      __lock(0);
      if(cnt1 == 0)
      {
        __unlock(0);
        while(cnt2 > 0) {asm("");}
        break;
      }
      int _cnt = cnt1--;
      __unlock(0);
      int lckid = _cnt%i;

      __lock(lckid);
      data[lckid]++;
      for(int j = 0; j < WAIT; j++)
        asm("");
      __unlock(lckid);

      __lock(0);
      if(cnt2 == 1)
        sync = 0;
      cnt2--;
      __unlock(0);
    }

    if(cpuid == 0)
    {
      stop = TIMER_CLK_LOW - stop;
      printf("Iteration with %d fields finished in %d cycles\n", i, stop);
    }

    int cnt = 0;
    for (int j = 0; j < i; j++)
      cnt += data[j];
 
    if(cnt != MAX_CNT)
        return cnt;
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

#ifdef _SSPM_
  for(int i = 0; i < MAX_LCK_CNT; i++)
    locks[i] = (volatile _SPM lock_t*) (LOWEST_SSPM_ADDRESS+(i*4));
#endif

  int threads[MAX_CORE_CNT];
  cpucnt = MAX_CORE_CNT;
  if(get_cpucnt() < cpucnt)
    cpucnt = get_cpucnt();

  printf("%d locks implemented using "_NAME" \n", MAX_LCK_CNT);
  printf("%d iterations in each set\n", MAX_CNT);
  printf("%d wait operations in each iteration\n", WAIT);
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
    void * res;
    corethread_join(threads[i], &res);
  }

  return ret;
}




