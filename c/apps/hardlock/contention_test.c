#include "setup.h"

#ifndef MAX_CNT
#define MAX_CNT 1000
#endif
#ifndef MAX_WAIT
#define MAX_WAIT 10000
#endif

#define shared_lock() _lock(MAX_LCK_CNT-2)
#define shared_unlock() _unlock(MAX_LCK_CNT-2)

#ifdef USE_PTHREAD_MUTEX
_UNCACHED unsigned int LOCKS[MAX_LCK_CNT];

void locks_init() {
  pthread_mutexattr_t dummy;
  for(int i = 0; i < MAX_LCK_CNT-1; i++)
    pthread_mutex_init(((pthread_mutex_t*)&LOCKS)+i, &dummy);
}

#define _lock(lockid) pthread_mutex_lock(((pthread_mutex_t*)&LOCKS)+lockid)
#define _unlock(lockid) pthread_mutex_unlock(((pthread_mutex_t*)&LOCKS)+lockid)

#endif

#ifdef VALIDATION
_UNCACHED int data[MAX_LCK_CNT];
#endif


_UNCACHED int _lckcnt;
_UNCACHED int _wait;
_UNCACHED int _cntmax;

volatile _IODEV int *dead_ptr = (volatile _IODEV int *) PATMOS_IO_DEADLINE;

void test(int lckcnt, int wait, int cnt)
{
  int cpuid = get_cpuid();
  int lckid = 0;
  for(int i = 0; i < cnt; i++)
  {
    _lock(lckid);
    *dead_ptr = wait;

#ifdef VALIDATION
    data[lckid]++;
#endif

    *dead_ptr;
    _unlock(lckid);
    if(++lckid >= lckcnt)
      lckid = 0;
  }
}

void worker_init(void* arg) {
  shared_lock();
  shared_unlock();
  test(_lckcnt,_wait,_cntmax);
  int ret = 0;
  corethread_exit(&ret);
  return;
}

int main() {

#ifdef USE_PTHREAD_MUTEX
  locks_init();
#endif



  int cpucnt = get_cpucnt();
  if(MAX_CORE_CNT < cpucnt)
    cpucnt = MAX_CORE_CNT;

  const int summax = MAX_CNT*cpucnt;

  printf("%d locks/iterations implemented using "_NAME" \n", MAX_LCK_CNT);
  printf("%d incrementations for each core for each iteration\n", MAX_CNT);
  printf("Starting %d cores\n",cpucnt);
  for(int wait = 10; wait <= MAX_WAIT; wait *= 10) {

    printf("%d cycle critical section for each incrementation\n", wait);

    for(int lckcnt = 1; lckcnt < MAX_LCK_CNT-2; lckcnt += 1) {
#ifdef VALIDATION
      for (int i = 0; i < lckcnt; i++) {
        data[i] = 0;
      }
#endif

      _lckcnt = lckcnt;
      _wait = wait;
      _cntmax = MAX_CNT;

      shared_lock();

      for(int i = 1; i < cpucnt; i++)
        corethread_create(i,&worker_init,NULL);

      int time = TIMER_CLK_LOW;
      shared_unlock();

      test(lckcnt,wait,MAX_CNT);

      for(int i = 1; i < cpucnt; i++) {
        void * res;
        corethread_join(i, &res);
      }
      time = TIMER_CLK_LOW - time;

#ifdef VALIDATION
      int sum = 0;
      for (int i = 0; i < lckcnt; i++)
        sum += data[i];

      if(sum != summax) {
        printf("Iteration with %d fields failed.\nProper sum:%d\nCalulated sum:%d\n", lckcnt, summax, sum);
        return sum;
      }
#endif

      printf("Iteration with %d fields finished in %d cycles\n", lckcnt, time);
    }
  }

  return 0;
}




