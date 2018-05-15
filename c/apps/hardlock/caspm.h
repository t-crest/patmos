#ifndef _CASPM_H_
#define _CASPM_H_

#include <machine/patmos.h>
#define CASPM_BASE ((volatile _SPM int *) 0xE8000000)

#define set_exp_val(val) *CASPM_BASE = val
#define set_new_val(val) *(CASPM_BASE+1) = val

#endif
