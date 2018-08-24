#ifndef _ASYNCLOCK_H_
#define _ASYNCLOCK_H_

#include <machine/patmos.h>
#define ASYNCLOCK_BASE ((_iodev_ptr_t) PATMOS_IO_ASYNCLOCK)

#define lock(lockid) *(ASYNCLOCK_BASE + lockid)
#define unlock(lockid) *(ASYNCLOCK_BASE + lockid) = 0

#endif
