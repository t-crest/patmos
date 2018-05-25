#ifndef _HARDLOCK_
#ifndef _SSPM_
#ifndef _ASYNCLOCK_
#ifndef _CASPM_
#define _HARDLOCK_
#endif
#endif
#endif
#endif

#include <stdio.h>
#include <machine/patmos.h>
#include <machine/spm.h>
#include "libcorethread/corethread.h"



#include <pthread.h>
#include <sys/types.h>

__off_t h;



#define TIMER_CLK_LOW *((volatile _IODEV int *) (PATMOS_IO_TIMER + 0x4))
#define TIMER_US_LOW *((volatile _IODEV int *) (PATMOS_IO_TIMER + 0xc))

#ifdef _HARDLOCK_
#include "hardlock.h"
#define __lock(lockid) lock(lockid);
#define __unlock(lockid) unlock(lockid);
#define _NAME "Hardlock"
void locks_init() {}
#endif

#ifdef _ASYNCLOCK_
#include "asynclock.h"
#define __lock(lockid) lock(lockid);
#define __unlock(lockid) unlock(lockid);
#define _NAME "Asynclock"
void locks_init() {}
#endif

#ifdef _CASPM_
#include "caspm.h"
#define __lock(lockid) lock(lockid)
#define __unlock(lockid) unlock(lockid)
#define _NAME "CASPM"
void locks_init() {}
#endif

#ifdef _SSPM_
#include "../sspm/atomic.h"
#include "../sspm/sspm_properties.h"
#define __lock(lockid) lock(locks[lockid]);
#define __unlock(lockid) release(locks[lockid]);
#define _NAME "SSPM"
volatile _SPM lock_t *locks[MAX_LCK_CNT];
void locks_init() {
  for(int i = 0; i < MAX_LCK_CNT; i++)
    locks[i] = (volatile _SPM lock_t*) (LOWEST_SSPM_ADDRESS+(i*4));
}
#endif

#ifndef MAX_CORE_CNT
#define MAX_CORE_CNT 20
#endif

#ifndef MAX_LCK_CNT
#define MAX_LCK_CNT 8
#endif
