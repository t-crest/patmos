#ifndef _SPMPOOL_H_
#define _SPMPOOL_H_

#include <machine/patmos.h>

#define SPMPOOL_BASE ((_iodev_ptr_t) PATMOS_IO_SPMPOOL)
#define SPMPOOL_CTRL_BASE ((_iodev_ptr_t) 0xE8FFFC00)
#define SPMPOOL_CTRL ((_iodev_ptr_t) 0xE8FFFFFF)
#define spm_base(id) SPMPOOL_BASE+(id << 8)
#define spm_req() *SPMPOOL_CTRL
#define spm_sched_wr(spmid,bitvector) *(SPMPOOL_CTRL_BASE+spmid) = bitvector
#define spm_sched_rd(spmid) *(SPMPOOL_CTRL_BASE+spmid)

#endif
