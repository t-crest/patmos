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

#ifdef _HARDLOCK_
#define _NAME "POSIX Mutex(Hardlock)"
#endif

#ifdef _ASYNCLOCK_
#define _NAME "POSIX Mutex(Asynclock)"
#endif

#ifdef _CASPM_
#define _NAME "POSIX Mutex(CASPM)"
#endif

#else

#ifdef _HARDLOCK_
#define _NAME "Hardlock"
#endif

#ifdef _ASYNCLOCK_
#define _NAME "Asynclock"
#endif

#ifdef _CASPM_
#define _NAME "CASPM"
#endif

#endif
