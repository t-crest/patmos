#ifndef _SPMPOOL_H_
#define _SPMPOOL_H_

#include <machine/patmos.h>

#define __SPMPOOL_SPM_CNT_MAX 15 // 15 SPMs + CTRL
#define __SPMPOOL_DATA_ADDR_WIDTH 10 // log2(4096) = 12
#define __SPMPOOL_CTRL_BASE ((_iodev_ptr_t) PATMOS_IO_SPMPOOL + (__SPMPOOL_SPM_CNT_MAX << __SPMPOOL_DATA_ADDR_WIDTH))
#define spm_base(id) ((_iodev_ptr_t)PATMOS_IO_SPMPOOL+(id << 10))
#define spm_req() *(__SPMPOOL_CTRL_BASE+__SPMPOOL_SPM_CNT_MAX)
#define spm_sched_wr(spmid,bitvector) *(__SPMPOOL_CTRL_BASE+spmid) = bitvector
#define spm_sched_rd(spmid) *(__SPMPOOL_CTRL_BASE+spmid)

#endif
