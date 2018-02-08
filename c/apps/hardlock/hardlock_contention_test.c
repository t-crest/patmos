#include "setup.h"
#include "hardlock.h"

_UNCACHED int data[LCK_CNT];
_UNCACHED int cnt1 = MAX_CNT;
_UNCACHED int cnt2 = MAX_CNT;
_UNCACHED int sync = 0;

int _main()
{
  int cpuid = get_cpuid();
  int cpucnt = get_cpucnt();
  int stop;
  for(int i = 1; i <= LCK_CNT; i++)
  {
    lock(0);
    if(sync == cpucnt-1)
    {
      for (int j = 0; j < LCK_CNT; j++) {
        data[j] = 0;
      }
      cnt1 = MAX_CNT;
      cnt2 = MAX_CNT;
    }
    sync++;
    unlock(0);
    while(sync < cpucnt) {asm("");}

    if(cpuid == 0)
      stop = TIMER_CLK_LOW;
    while(1)
    {
      lock(0);
      if(cnt1 == 0)
      {
        unlock(0);
        while(cnt2 > 0) {asm("");}
        break;
      } 
      int _cnt = cnt1--;
      unlock(0);
      int fldid = _cnt%i;
      int lckid = fldid%LCK_CNT;

      // Field specific lock
      lock(lckid);
      data[fldid]++;
      for(int j = 0; j < WAIT; j++)
        asm("");
      unlock(lckid);

      lock(0);
      if(cnt2 == 1)
        sync = 0;
      cnt2--;
      unlock(0);
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
        return i;
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

  int threads[MAX_CORE_CNT];
  int len = sizeof threads / sizeof *threads;
  if(get_cpucnt() < len)
    len = get_cpucnt();

  printf("%d iterations in each set\n", MAX_CNT);
  printf("%d wait operations in each iteration\n", WAIT);
  printf("Starting %d cores\n",len);

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




