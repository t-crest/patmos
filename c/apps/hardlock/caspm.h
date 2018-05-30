#ifndef _CASPM_H_
#define _CASPM_H_

#include <machine/patmos.h>
#define CASPM_BASE ((volatile _SPM int *) 0xE8000000)

#define set_exp_val(val) *CASPM_BASE = val
#define set_new_val(val) *(CASPM_BASE+1) = val

#define lock(lockid) set_exp_val(0); set_new_val(1); while(*(CASPM_BASE+lockid) != 0){asm("");}
#define unlock(lockid) set_exp_val(1); set_new_val(0); while(*(CASPM_BASE+lockid) != 1){asm("");}

#endif
