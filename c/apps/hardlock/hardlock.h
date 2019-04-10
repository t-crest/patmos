#ifndef _HARDLOCK_H_
#define _HARDLOCK_H_

#include <machine/patmos.h>
#define HARDLOCK_BASE ((_iodev_ptr_t) PATMOS_IO_HARDLOCK)

#define lock(lockid) do {asm volatile ("" : : : "memory"); *HARDLOCK_BASE = ((lockid) << 1) + 1; asm volatile ("" : : : "memory");} while(0)
#define unlock(lockid) do {asm volatile ("" : : : "memory"); *HARDLOCK_BASE = ((lockid) << 1) + 0; asm volatile ("" : : : "memory");} while(0)

#endif
