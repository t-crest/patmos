#ifndef _HARDLOCK_H_
#define _HARDLOCK_H_

#include <machine/patmos.h>
#define HARDLOCK_BASE ((volatile _SPM int *) 0xE8000000)

#define lock(lockid) HARDLOCK_BASE = (lockid << 1) + 1
#define unlock(lockid) HARDLOCK_BASE = (lockid << 1) + 0

#endif
