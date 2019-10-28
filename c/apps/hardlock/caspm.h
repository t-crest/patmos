#ifndef _CASPM_H_
#define _CASPM_H_

#include <machine/patmos.h>
#define CASPM_BASE ((_iodev_ptr_t) PATMOS_IO_CASPM)

#define set_exp_val(val) *CASPM_BASE = val
#define set_new_val(val) *(CASPM_BASE+1) = val

int cas(_iodev_ptr_t ptr, int exp, int new)
{
	set_exp_val(exp);
	set_new_val(new); 
	return *ptr;
}

int caspm_read(_iodev_ptr_t ptr)
{
	set_exp_val(0);
	set_new_val(0); 
	return *ptr;
}

#ifndef _CASPM_SUPRESS_LOCK_

#define lock(lockid) do {asm volatile ("" : : : "memory"); set_exp_val(0); set_new_val(1); while(*(CASPM_BASE+lockid) != 0){asm("");} asm volatile ("" : : : "memory");} while(0)
#define unlock(lockid) do {asm volatile ("" : : : "memory"); set_exp_val(1); set_new_val(0); while(*(CASPM_BASE+lockid) != 1){asm("");} asm volatile ("" : : : "memory");} while(0)

#endif

#endif
