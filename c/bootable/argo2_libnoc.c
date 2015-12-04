//#include <string.h>
#include <machine/spm.h>
#include <machine/exceptions.h>
#include "include/bootable.h"
#include "include/patio.h"
#include "libnoc/noc.h"

const int NOC_MASTER = 0;

#define __IO_CALC_ADDRESS_DYNAMIC(BASE, BANK, OFFSET) \
  ((void _SPM *)(((unsigned char*)BASE) + (BANK) + (OFFSET)))

#define ARGO_CONFIG_RD32(BANK, OFFSET) \
  *((volatile unsigned long int _SPM *)(__IO_CALC_ADDRESS_DYNAMIC ((NOC_DMA_BASE), (BANK), (OFFSET))))

#define ARGO_CONFIG_RD16(BANK, OFFSET) \
  *((volatile unsigned short int _SPM *)(__IO_CALC_ADDRESS_DYNAMIC ((NOC_DMA_BASE), (BANK), (OFFSET))))

#define ARGO_CONFIG_RD8(BANK, OFFSET) \
  *((volatile unsigned char _SPM *)(__IO_CALC_ADDRESS_DYNAMIC ((NOC_DMA_BASE), (BANK), (OFFSET))))



#define ARGO_CONFIG_WR32(BANK, OFFSET, DATA) \
  *((volatile unsigned long int _SPM *)(__IO_CALC_ADDRESS_DYNAMIC ((NOC_DMA_BASE), (BANK), (OFFSET)))) = (DATA)

#define ARGO_CONFIG_WR16(BANK, OFFSET, DATA) \
  *((volatile unsigned short int _SPM *)(__IO_CALC_ADDRESS_DYNAMIC ((NOC_DMA_BASE), (BANK), (OFFSET)))) = (DATA)

#define ARGO_CONFIG_WR8(BANK, OFFSET, DATA) \
  *((volatile unsigned char _SPM *)(__IO_CALC_ADDRESS_DYNAMIC ((NOC_DMA_BASE), (BANK), (OFFSET)))) = (DATA)


#define COM_SPM_RD32(OFFSET) \
  *((volatile unsigned long int _SPM *)(__IO_CALC_ADDRESS_DYNAMIC ((NOC_SPM_BASE), (0), (OFFSET))))

#define COM_SPM_WR32(OFFSET, DATA) \
  *((volatile unsigned long int _SPM *)(__IO_CALC_ADDRESS_DYNAMIC ((NOC_SPM_BASE), (0), (OFFSET)))) = (DATA)



int main(void) __attribute__((noreturn));

int main() {
  noc_configure();

  intr_unmask_all();
  // clear pending flags
  intr_clear_all_pending();
  // enable interrupts
  intr_enable();
  if (get_cpuid() == 0) {
    // Enter user mode
    EXC_STATUS &= ~2; 
  }
  
  int tmp1 = 0;
  int tmp2 = 0;
  // Reading the 64 bit counter
  // 64 bit counter it temporarily disabled
  //tmp1 = ARGO_CONFIG_RD32(TDM_BANK, 3<<2);
  //tmp2 = ARGO_CONFIG_RD32(TDM_BANK, 2<<2);

//  if (tmp1 == 0) {
//    WRITE("ERROR0\n",7); 
//  }
//  if (tmp2 == tmp1) {
//    WRITE("ERROR1\n",7); 
//  } else {
//    WRITE("OK1\n",4);
//  }

  // Write to the COM_SPM
  COM_SPM_WR32(0<<2,0x11223344);
  COM_SPM_WR32(1<<2,0x55667788);
  COM_SPM_WR32(2<<2,0x99001122);
  COM_SPM_WR32(3<<2,0x33445566);

  if (get_cpuid() == 0) {
    noc_done(3);
    noc_dma(1,0,0,2);
  }
  


  // Setting the first entry of the DMA table
  //       Header field
  //       Pkt format | Write_ptr
  tmp1 =   0 << 14    | 5;
  //    Active bit | count   | Read_ptr
  tmp2 =  1 << 31  | 2 << 14 | 0 ;

  ARGO_CONFIG_WR32(DMA_BANK,0<<2,tmp1);
  ARGO_CONFIG_WR32(DMA_BANK,1<<2,tmp2);

  tmp2 = ARGO_CONFIG_RD32(DMA_BANK,0<<2);

  if (tmp2 == tmp1) {
    WRITE("OK3\n",4); 
  } else {
    WRITE("ERROR3\n",7); 
  }

  // Setting the second entry of the DMA table
  //       Header field
  //       Pkt format | Write_ptr
  tmp1 =   0 << 14    | 9;
  //    Active bit | count   | Read_ptr
  tmp2 =  1 << 31  | 2 << 14 | 0 ;
  ARGO_CONFIG_WR32(DMA_BANK,2<<2,tmp1);
  ARGO_CONFIG_WR32(DMA_BANK,3<<2,tmp2);

// Setting the third entry of the DMA table
  //       Header field
  //       Pkt format | Write_ptr
  tmp1 =   1 << 14    | 10;
  //    Active bit | count   | Read_ptr
  tmp2 =  1 << 31  | 1 << 14 | 0 ;
  ARGO_CONFIG_WR32(DMA_BANK,4<<2,tmp1);
  ARGO_CONFIG_WR32(DMA_BANK,5<<2,tmp2);

  // Setting stbl_maxp1 to 2 and stbl_min to 0
  tmp1 = 3 << 16 | 0;
  ARGO_CONFIG_WR32(MC_BANK,2<<2,tmp1);
  tmp2 = ARGO_CONFIG_RD32(MC_BANK,2<<2);

  if (tmp2 == tmp1) {
    WRITE("OK4\n",4); 
  }


  WRITE("Testing mode change\n",20);
  // Writing schedule table
  //        Route        | DMA_num    | Pktlen| t2n
  tmp1 = 13 << (8+3+5) | 0 << (3+5) | 2 << 5| 5;
  ARGO_CONFIG_WR32(SCHED_BANK,3<<2,tmp1);
  //        Route        | DMA_num    | Pktlen| t2n
  tmp1 = 13 << (8+3+5)   | 255 << (3+5) | 1 << 5| 4;
  ARGO_CONFIG_WR32(SCHED_BANK,4<<2,tmp1);
  //        Route        | DMA_num    | Pktlen| t2n
  tmp1 = 13 << (8+3+5)   | 255 << (3+5) | 3 << 5| 6;
  ARGO_CONFIG_WR32(SCHED_BANK,5<<2,tmp1);

  // Writing mode change table
  // Setting stbl_maxp1 to 5 and stbl_min to 2
  tmp1 = 6 << 16 | 3;
  ARGO_CONFIG_WR32(MC_BANK,3<<2,tmp1);

  if (get_cpuid() == 0) {
    tmp1 = 1;
    ARGO_CONFIG_WR32(MC_BANK,0<<2,tmp1);
  }

}
