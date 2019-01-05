#ifndef _SPMPOOL_H_
#define _SPMPOOL_H_

#include <machine/patmos.h>

#define __SPMPOOL_SPM_CNT_MAX 15 // 15 SPMs + CTRL
#define __SPMPOOL_DATA_ADDR_WIDTH 12 // log2(4096)
#define __SPMPOOL_CTRL_BASE ((_iodev_ptr_t)(PATMOS_IO_SPMPOOL + (__SPMPOOL_SPM_CNT_MAX << __SPMPOOL_DATA_ADDR_WIDTH)))
#define spm_base(id) (PATMOS_IO_SPMPOOL+(id << __SPMPOOL_DATA_ADDR_WIDTH))
#define spm_req() *(__SPMPOOL_CTRL_BASE+__SPMPOOL_SPM_CNT_MAX)
#define spm_sched_wr(spmid,bitvector) *(__SPMPOOL_CTRL_BASE+(spmid)) = bitvector
#define spm_sched_rd(spmid) *(__SPMPOOL_CTRL_BASE+(spmid))

#define SPMPOOL_NEXT (0x1000/4) // SPMs are placed every 4 KB 

#endif
