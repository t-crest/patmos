#include <stdio.h>
#include <machine/patmos.h>
#include <machine/spm.h>
#include "libcorethread/corethread.h"

#define MAX_CORE_CNT 20
#define MAX_CNT 10000
#define LCK_CNT 8
#define WAIT 1

#define TIMER_CLK_LOW *((volatile _IODEV int *) (PATMOS_IO_TIMER + 0x4))
#define TIMER_US_LOW *((volatile _IODEV int *) (PATMOS_IO_TIMER + 0xc))

