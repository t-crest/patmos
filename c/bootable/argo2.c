//#include <string.h>
#include <machine/spm.h>
#include "include/bootable.h"
#include "include/patio.h"
#include "libnoc/noc.h"

#define __IO_CALC_ADDRESS_DYNAMIC(BASE, BANK, OFFSET) \
  ((void _SPM *)(((unsigned char*)BASE) + (BANK) + (OFFSET)))

#define IORD_32DIRECT(BASE, BANK, OFFSET) \
  *((volatile unsigned long int _SPM *)(__IO_CALC_ADDRESS_DYNAMIC ((BASE), (BANK), (OFFSET))))

#define IORD_16DIRECT(BASE, BANK, OFFSET) \
  *((volatile unsigned short int _SPM *)(__IO_CALC_ADDRESS_DYNAMIC ((BASE), (BANK), (OFFSET))))

#define IORD_8DIRECT(BASE, BANK, OFFSET) \
  *((volatile unsigned char _SPM *)(__IO_CALC_ADDRESS_DYNAMIC ((BASE), (BANK), (OFFSET))))



#define IOWR_32DIRECT(BASE, BANK, OFFSET, DATA) \
  *((volatile unsigned long int _SPM *)(__IO_CALC_ADDRESS_DYNAMIC ((BASE), (BANK), (OFFSET)))) = (DATA)

#define IOWR_16DIRECT(BASE, BANK, OFFSET, DATA) \
  *((volatile unsigned short int _SPM *)(__IO_CALC_ADDRESS_DYNAMIC ((BASE), (BANK), (OFFSET)))) = (DATA)

#define IOWR_8DIRECT(BASE, BANK, OFFSET, DATA) \
  *((volatile unsigned char _SPM *)(__IO_CALC_ADDRESS_DYNAMIC ((BASE), (BANK), (OFFSET)))) = (DATA)


/* Native bus access functions */

#define __IO_CALC_ADDRESS_NATIVE(BASE, REGNUM) \
  ((void *)(((unsigned char*)BASE) + ((REGNUM) * (SYSTEM_BUS_WIDTH/8))))

#define IORD(BASE, REGNUM) \
  *((volatile unsigned long int _SPM *)(__IO_CALC_ADDRESS_NATIVE ((BASE), (REGNUM))))
#define IOWR(BASE, REGNUM, DATA) \
  *((volatile unsigned long int _SPM *)(__IO_CALC_ADDRESS_NATIVE ((BASE), (REGNUM)))) = (DATA)


#define OFFSET_WIDTH  (11+2)
#define BANK(ID)      (ID<<OFFSET_WIDTH)

#define DMA_BANK      BANK(0)
#define SCHED_BANK    BANK(1)
#define TDM_BANK      BANK(2)
#define CLOCK_BANK    BANK(3)
#define IRQ_BANK      BANK(4)
#define DATA_BANK     BANK(5)
#define ERROR_BANK    BANK(6)
#define PERF_BANK     BANK(7)

int main(void) __attribute__((noreturn));

int main() {
  
  int tmp1 = 0;
  int tmp2 = 0;
  // Reading the 64 bit counter
  tmp1 = IORD_32DIRECT(NOC_DMA_BASE,TDM_BANK,3<<2);
  tmp2 = IORD_32DIRECT(NOC_DMA_BASE,TDM_BANK,2<<2);


  if (tmp1 == 0) {
    WRITE("ERROR0\n",7); 
  }
  if (tmp2 == tmp1) {
    WRITE("ERROR1\n",7); 
  } else {
    WRITE("OK1\n",4);
  }

  // Setting the first entry of the schedule table
  tmp1 = 0x77 << (8+3+5) | 0 << (3+5) | 1 << 5| 5;
  IOWR_32DIRECT(NOC_DMA_BASE,SCHED_BANK,0<<2,tmp1);
  tmp2 = IORD_32DIRECT(NOC_DMA_BASE,SCHED_BANK,0<<2);

  if (tmp2 == tmp1) {
    WRITE("OK2\n",4); 
  }

  // Setting stbl_maxp1 to 2 and stbl_min to 0
  tmp1 = 4 << 16 | 0;
  IOWR_32DIRECT(NOC_DMA_BASE,TDM_BANK,8<<2,tmp1);
  tmp2 = IORD_32DIRECT(NOC_DMA_BASE,TDM_BANK,8<<2);

  if (tmp2 == tmp1) {
    WRITE("OK3\n",4); 
  }

  // Setting run to 1
  tmp1 = 1;
  IOWR_32DIRECT(NOC_DMA_BASE,TDM_BANK,128<<2,tmp1);
  asm volatile ("nop");
  tmp2 = IORD_32DIRECT(NOC_DMA_BASE,TDM_BANK,128<<2);

  if (tmp2 == tmp1) {
    WRITE("OK4\n",4); 
  }

}
