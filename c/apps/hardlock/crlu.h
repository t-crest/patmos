#ifndef _CRLU_H_
#define _CRLU_H_

#include <machine/patmos.h>
#define CRLU_BASE *((volatile _SPM int *) 0xE8000000)
#define lock(lockid) CRLU_BASE = (lockid << 1) + 1;
#define unlock(lockid) CRLU_BASE = (lockid << 1) + 0;

#endif
