#include "setup.h"

#ifndef MAX_CNT
#define MAX_CNT 10000
#endif
#ifndef WAIT
#define WAIT 10
#endif

#define shared_lock() __lock(MAX_LCK_CNT-1)
#define shared_unlock() __unlock(MAX_LCK_CNT-1)

_UNCACHED int data[MAX_LCK_CNT];
_UNCACHED int cnt = MAX_CNT;


volatile _IODEV int *dead_ptr = (volatile _IODEV int *) PATMOS_IO_DEADLINE;

void work(int lckcnt)
{
  while(1)
  {
    shared_lock();

    if(cnt == 0)
    {
      shared_unlock();
      break;
    }

    int _cnt = --cnt;

    shared_unlock();

    int lckid = _cnt%lckcnt;

    __lock(lckid);
    *dead_ptr = WAIT;
    data[lckid]++;
    *dead_ptr;
    __unlock(lckid);

  }
}

void worker_init(void* arg) {
  int lckcnt = *((int*)arg);
  work(lckcnt);
  int ret = 0;
  corethread_exit(&ret);
  return;
}

int main() {

  LOCKS_INIT

  int cpucnt = MAX_CORE_CNT;
  if(get_cpucnt() < cpucnt)
    cpucnt = get_cpucnt();

  printf("%d locks/iterations implemented using "_NAME" \n", MAX_LCK_CNT);
  printf("%d incrementations for each iteration\n", MAX_CNT);
  printf("%d cycle critical section for each incrementation\n", WAIT);
  printf("Starting %d cores\n",cpucnt);

  for(int i = 1; i < MAX_LCK_CNT-1; i++) {

    for (int j = 0; j < i; j++) {
      data[j] = 0;
    }
    cnt = MAX_CNT;
    
    shared_lock();
    for(int j = 1; j < cpucnt; j++)
      corethread_create(j,&worker_init,&i);
    shared_unlock();

    int time = TIMER_CLK_LOW;
    work(i);
    for(int j = 1; j < cpucnt; j++) {
      void * res;
      corethread_join(j, &res);
    }
    time = TIMER_CLK_LOW - time;
    
    for (int j = 0; j < i; j++)
      cnt += data[j];
 
    if(cnt != MAX_CNT) {
      printf("Iteration with %d fields failed.\nProper sum:%d\nCalulated sum:%d\n", i, MAX_CNT, cnt);
      return cnt;
    }

    printf("Iteration with %d fields finished in %d cycles\n", i, time);

    
  }

  return 0;
}




