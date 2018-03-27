#ifndef _SPMPOOL_H_
#define _SPMPOOL_H_

#include <machine/patmos.h>
#define SPMPOOL_BASE ((volatile _SPM int *) 0xE8000000)
#define SPMPOOL_CTRL_BASE ((volatile _SPM int *) 0xE8003C00)
#define SPMPOOL_CTRL ((volatile _SPM int *) 0xE8003FFC)
#define spm_request() *SPMPOOL_CTRL
#define schedule_write(spmid,bitvector) *(SPMPOOL_CTRL_BASE+spmid) = bitvector
#define schedule_read(spmid) *(SPMPOOL_CTRL_BASE+spmid)
#define spm_write(spmid, addr, data) *(SPMPOOL_BASE+(spmid << 8)+addr) = data
#define spm_read(spmid, addr) *(SPMPOOL_BASE+(spmid << 8)+addr)

#endif
