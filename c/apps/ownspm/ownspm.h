#ifndef _OWNSPM_H_
#define _OWNSPM_H_

_iodev_ptr_t spm_ptr = (_iodev_ptr_t) PATMOS_IO_OWNSPM;

#define NEXT 0x10000/4 // SPMs are placed every 64 KB 

#endif
