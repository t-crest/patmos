#include "setup.h"

#ifdef USE_PTHREAD_MUTEX
#define ___lock(lckid) __lock(lckid)
#define ___unlock(lckid) __unlock(lckid)
#else
#ifdef _HARDLOCK_
#define ___lock(lckid) *lockbase = rawlockid
#define ___unlock(lckid) *lockbase = rawunlockid
#endif
#ifdef _ASYNCLOCK_
#define ___lock(lckid) *lockbase
#define ___unlock(lckid) *lockbase = 0
#endif
#ifdef _CASPM_
#define ___lock(lckid) set_exp_val(0); set_new_val(1); while(*lockbase != 0){asm("");}
#define ___unlock(lckid) set_exp_val(1); set_new_val(0); while(*lockbase != 1){asm("");}
#endif
#endif

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

const int shift = 10;
const int iter = 1 << shift;
const int MIN_START = 10000;

void acquirethenrelease(int coreid, int lckid, int rawlockid, int rawunlockid, volatile _SPM int * lockbase) {

  int acquire = 0;
  int acquire_avg = 0;
  int acquire_max = 0;
  int acquire_min = MIN_START;
  int release_avg = 0;
  int release = 0;
  int release_max = 0;
  int release_min = MIN_START;
  
  int stop1;
  int stop2;
  int stop3;

  for(int i = 0; i < iter; i++)
  {
    asm("");
    stop1 = TIMER_CLK_LOW;
    ___lock(lckid);
    stop2 = TIMER_CLK_LOW;
    ___unlock(lckid);
    stop3 = TIMER_CLK_LOW;
    asm("");

    acquire = (stop2 - stop1) - 1;
    acquire_avg += acquire;
    
    if(acquire > acquire_max)
      acquire_max = acquire;
    else if(acquire < acquire_min)
      acquire_min = acquire;

    release = (stop3 - stop2) - 1;
    release_avg += release;

    if(release > release_max)
      release_max = release;
    else if(release < release_min)
      release_min = release;

  }

  acquisitions_avg[coreid] = acquire_avg >> shift;
  acquisitions_max[coreid] = acquire_max;
  acquisitions_min[coreid] = acquire_min;
  releases_avg[coreid] = release_avg >> shift;
  releases_max[coreid] = release_max;
  releases_min[coreid] = release_min;
}

void acquireandrelease(int coreid, int lckid, int rawlockid, int rawunlockid, volatile _SPM int * lockbase) {

  int acquirel = 0;
  int acquirel_avg = 0;
  int acquirel_max = 0;
  int acquirel_min = MIN_START;

  int stop1;
  int stop2;


  for(int i = 0; i < iter; i++)
  {
    asm("");
    stop1 = TIMER_CLK_LOW;
    ___lock(lckid);
    ___unlock(lckid);
    stop2 = TIMER_CLK_LOW;
    asm("");

    acquirel = (stop2 - stop1) - 1;
    acquirel_avg += acquirel;
    if(acquirel > acquirel_max)
      acquirel_max = acquirel;
    else if(acquirel < acquirel_min)
      acquirel_min = acquirel;
  }

  acquirels_avg[coreid] = acquirel_avg >> shift;
  acquirels_max[coreid] = acquirel_max;
  acquirels_min[coreid] = acquirel_min;
}

int _main()
{
  const int coreid = get_cpuid();
  const int lckid = coreid % MAX_LCK_CNT;


#ifdef USE_PTHREAD_MUTEX
  acquirethenrelease(coreid,lckid,0,0,0);
  acquireandrelease(coreid,lckid,0,0,0);
#else
#ifdef _HARDLOCK_
  const int rawlockid = (((lckid) << 1) + 1);
  const int rawunlockid = (((lckid) << 1) + 0);
  acquirethenrelease(coreid,lckid,rawlockid,rawunlockid,HARDLOCK_BASE);
  acquireandrelease(coreid,lckid,rawlockid,rawunlockid,HARDLOCK_BASE);
#endif
#ifdef _ASYNCLOCK_
  acquirethenrelease(coreid,lckid,0,0,ASYNCLOCK_BASE+lckid);
  acquireandrelease(coreid,lckid,0,0,ASYNCLOCK_BASE+lckid);
#endif
#ifdef _CASPM_
  acquirethenrelease(coreid,lckid,0,0,CASPM_BASE+lckid);
  acquireandrelease(coreid,lckid,0,0,CASPM_BASE+lckid);
#endif
#endif

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




