#include <stdio.h>
#include <machine/patmos.h>
#include <machine/spm.h>
#include "libcorethread/corethread.h"

#include "pthread.h"

#ifndef MAX_CORE_CNT
#define MAX_CORE_CNT 20
#endif

#ifndef MAX_LCK_CNT
#define MAX_LCK_CNT _MAX_HARDWARE_LOCKS_
#endif


#define TIMER_CLK_LOW *((volatile _IODEV int *) (PATMOS_IO_TIMER + 0x4))
#define TIMER_US_LOW *((volatile _IODEV int *) (PATMOS_IO_TIMER + 0xc))


#ifdef USE_PTHREAD_MUTEX
pthread_mutex_t LOCKS[MAX_LCK_CNT];

#define LOCKS_INIT locks_init();
void locks_init() {
  pthread_mutexattr_t dummy;
  for(int i = 1; i < MAX_LCK_CNT; i++)
    pthread_mutex_init(&LOCKS[i], &dummy);
}

#define __lock(lockid) pthread_mutex_lock(&LOCKS[lockid])
#define __unlock(lockid) pthread_mutex_unlock(&LOCKS[lockid])

#else

#define LOCKS_INIT
#define __lock(lockid) _lock(lockid)
#define __unlock(lockid) _unlock(lockid)
#endif

#ifdef _HARDLOCK_
#define _lock(lockid) lock(lockid)
#define _unlock(lockid) unlock(lockid)
#define _NAME "Hardlock"
#endif

#ifdef _ASYNCLOCK_
#define _lock(lockid) lock(lockid)
#define _unlock(lockid) unlock(lockid)
#define _NAME "Asynclock"
#endif

#ifdef _CASPM_
#define _lock(lockid) lock(lockid)
#define _unlock(lockid) unlock(lockid)
#define _NAME "CASPM"
#endif

#ifdef _SSPM_
#include "../sspm/atomic.h"
#include "../sspm/sspm_properties.h"
#define _lock(lockid) lock(locks[lockid])
#define _unlock(lockid) release(locks[lockid])
#define _NAME "SSPM"
volatile _SPM lock_t *locks[MAX_LCK_CNT];
#define LOCKS_INIT locks_init();
void locks_init() {
  for(int i = 0; i < MAX_LCK_CNT; i++)
    locks[i] = (volatile _SPM lock_t*) (LOWEST_SSPM_ADDRESS+(i*4));
}
#endif


