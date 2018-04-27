#ifndef _ASYNCLOCK_H_
#define _ASYNCLOCK_H_

#include <machine/patmos.h>
#define ASYNCLOCK_BASE ((volatile _SPM int *) 0xE8000000)

#define lock(lockid) *(ASYNCLOCK_BASE + lockid)
#define unlock(lockid) *(ASYNCLOCK_BASE + lockid) = 0

#endif
