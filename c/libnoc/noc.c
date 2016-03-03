/*
   Copyright 2014 Technical University of Denmark, DTU Compute. 
   All rights reserved.
   
   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

/*
 * Functions to initialize and use the NoC.
 * 
 * Author: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 * Author: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
 *
 */


#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/boot.h>
#include <machine/exceptions.h>

//#include <stdio.h>
#include <stdlib.h>
#include "noc.h"

#define TRAP

void __remote_irq_handler(void)  __attribute__((naked));
void __remote_irq_handler(void) {
  exc_prologue();
  //WRITE("IRQ0\n",5);
  int tmp = *(NOC_IRQ_BASE);
  *(NOC_SPM_BASE+(tmp<<2)) = 0;
  intr_clear_pending(exc_get_source());
  exc_epilogue();
}

void __data_recv_handler(void) __attribute__((naked));
void __data_recv_handler(void) {
  exc_prologue();
  //WRITE("IRQ1\n",5); 
  int tmp = *(NOC_IRQ_BASE+1);
  *(NOC_SPM_BASE+(tmp<<2)) = 0;
  intr_clear_pending(exc_get_source());
  exc_epilogue();
}

#ifdef TRAP
void __noc_trap_handler(void)  __attribute__((naked));
void __noc_trap_handler(void) {
  
  unsigned int op,p1,p2,p3,p4;
  unsigned int ret_base,ret_offset;
  asm volatile("mov %0 = $r2;"
               "mov %1 = $r3;"
               "mov %2 = $r4;"
               "mov %3 = $r5;"
               "mov %4 = $r6;"
               "mfs %5 = $sxb;"
               "mfs %6 = $sxo;"
                 : "=r" (op), "=r" (p1), "=r" (p2), "=r" (p3), "=r" (p4), "=r" (ret_base), "=r" (ret_offset)
                 : : "$r2", "$r3", "$r4", "$r5", "$r6");
  intr_enable();
  //WRITE("TRAP\n",5);
  int ret = 0;
  switch (op) {
    case 0:
      //WRITE("calling noc_dma\n",16);
      ret = k_noc_dma(p1,(unsigned short)p2,(unsigned short)p3,(unsigned short)p4);
      break;
    case 1:
      //WRITE("Not done\n",9);
      ret = k_noc_done(p1);
      break;
    default:
      break;
      //WRITE("Illegal op\n",11);
  }
  intr_disable();
  //WRITE("Returning from trap\n",20);
  asm volatile("mts  $sxb = %0;"
               "mts  $sxo = %1;"
               "mov $r1 = %2;"
               "xretnd;"
                : : "r" (ret_base), "r" (ret_offset), "r" (ret)
                : "$r1");
}

#endif

  
// Find the size in 32-bit words
int find_mem_size(volatile int _SPM * mem_addr){
  int init = *(mem_addr);
  int tmp;
  *(mem_addr) = 0xFFEEDDCC;
  int i = 2;
  int j = 0;
  for(j = 0; j < 28; j++) {
    tmp = *(mem_addr+i);
    *(mem_addr+i) = 0;
    if (*(mem_addr) == 0) {
      *(mem_addr+i) = tmp;
      *(mem_addr) = init;
      return i*4;
    }
    i = i << 1;
    if (*(mem_addr) != 0xFFEEDDCC){
      *(mem_addr+i) = tmp;
      *(mem_addr) = init;
      return -1;
    }
    *(mem_addr+i) = tmp;
  }
  *(mem_addr) = init;
  return -1;
}

// Configure network interface according to initialization information
void noc_load_config(void) {
  unsigned int schedule_table_size = find_mem_size(NOC_SCHED_BASE);
  unsigned int noc_conf_base_ptr = 0;
  for (int j = 0; j < NOC_CONFS; ++j) {
    unsigned mode_idx = j * NOC_CORES * NOC_TABLES * NOC_SCHEDULE_ENTRIES;
    unsigned core_idx = get_cpuid() * NOC_TABLES * NOC_SCHEDULE_ENTRIES;
    unsigned short schedule_entries = noc_init_array[mode_idx+core_idx];
    if (noc_conf_base_ptr+schedule_entries >= schedule_table_size) {
      //WRITE("Configuration schedules do not fit in schedule table\n",53);
      //puts("Configuration schedules do not fit in schedule table");
    }
    for (unsigned i = 0; i < schedule_entries; ++i) {
      // Handling allocation of space in the mode change table
      // Add placement of first entry noc_conf_base_ptr to NOC_SCHED_BASE+i
      *(NOC_SCHED_BASE+noc_conf_base_ptr+i)
                                  = noc_init_array[mode_idx+core_idx + i + 1];
    }
    // Set the pointers to the start and to the end of the schedule
    //TODO: handle allocation of space in the mode change table
    // Add placement of first entry to schedule_entries and 0
    *(NOC_MC_BASE+2+j) = (schedule_entries+noc_conf_base_ptr) << 16 | noc_conf_base_ptr;
    noc_conf_base_ptr += schedule_entries;
  }
    
}

void noc_set_state(int state) {
  if (get_cpuid() == NOC_MASTER) {
    *(NOC_TDM_BASE+4) = state; // Set the run mode in the network
    while(*(NOC_TDM_BASE+4) != state);
  }
}

void noc_set_config(int config) {
  if (get_cpuid() == NOC_MASTER) {
    *(NOC_MC_BASE+0) = config; // Set the run mode in the network
    while(*(NOC_MC_BASE+0) != config);
  }
}

void noc_enable(void) {
  noc_set_state(1);
}

void noc_disable(void) {
  noc_set_state(0);
}

// Configure network interface according to initialization information
void noc_configure(void) {
  noc_load_config();
  
  exc_register(23,&__remote_irq_handler);
  exc_register(31,&__data_recv_handler);
#ifdef TRAP
  exc_register(8,&__noc_trap_handler);
#endif
}

// Synchronize start-up
static void noc_sync(void) {

  if (get_cpuid() == NOC_MASTER) {
    // Wait until all slaves have configured their network interface
    int done = 0;
    do {
      done = 1;
      for (unsigned i = 0; i < get_cpucnt(); i++) {
        if (boot_info->slave[i].status != STATUS_NULL &&
            boot_info->slave[i].status != STATUS_INITDONE && 
            i != NOC_MASTER) {
          done = 0;
        }
      }
    } while (!done);

    // TODO: start up network

    // Notify slaves that the network is started
    boot_info->master.status = STATUS_INITDONE;

  } else {
    // Notify master that network interface is configured
    boot_info->slave[get_cpuid()].status = STATUS_INITDONE;
    // Wait until master has started the network
    while (boot_info->master.status != STATUS_INITDONE) {
      /* spin */
    }
  }
}

// Initialize the NoC
void noc_init(void) {
  //if (get_cpuid() == NOC_MASTER) puts("noc_configure");
  noc_configure();
  //if (get_cpuid() == NOC_MASTER) puts("noc_sync");
  noc_sync();
  //if (get_cpuid() == NOC_MASTER) puts("noc_done");
  // The master synchronize with the other cores before setting
  // the network in run mode.
  noc_enable();
}

// Start a NoC data dma transfer
// The addresses and the size are in double-words and relative to the
// communication SPM
#ifdef TRAP
int noc_dma(unsigned dma_id,
            unsigned short write_ptr,
            unsigned short read_ptr,
            unsigned short size) {
    int zero = 0;
    unsigned int ret_base, ret_offset;
    asm volatile("mov $r2 = %2;"
                 "mov $r3 = %3;"
                 "mov $r4 = %4;"
                 "mov $r5 = %5;"
                 "mov $r6 = %6;"
                 "mfs %0 = $srb;"
                 "mfs %1 = $sro;"
                   : "=r" (ret_base), "=r" (ret_offset)
                   : "r" (zero), "r" (dma_id), "r" (write_ptr), "r" (read_ptr), "r" (size)
                   : "$r2", "$r3", "$r4", "$r5", "$r6" );
    trap(8);

//    asm volatile("retnd;"
//                  : : );

    int ret;
    asm volatile("mts $srb = %1;"
                 "mts $sro = %2;"
                 "mov %0 = $r1;"
                  : "=r" (ret)
                  : "r" (ret_base), "r" (ret_offset)
                  : "$r1");
//    WRITE("Returned: ",10);
//    char c = 48+ret;
//    WRITE(&c,1);
//    WRITE(" from trap\n",11);

    return ret;
}
#else
int noc_dma(unsigned dma_id,
            unsigned short write_ptr,
            unsigned short read_ptr,
            unsigned short size) {
    return k_noc_dma(dma_id,write_ptr,read_ptr,size);
}
#endif

// Start a NoC data dma transfer
// The addresses and the size are in double-words and relative to the
// communication SPM
int k_noc_dma(unsigned dma_id,
            unsigned short write_ptr,
            unsigned short read_ptr,
            unsigned short size) {
    //WRITE("k_noc_dma\n",10);
    // Only send if previous transfer is done
    if (!k_noc_done(dma_id)) {
      return 0;
    }

    // Read pointer and write pointer in the dma table
    *(NOC_DMA_BASE+(dma_id<<1)) = (DATA_PKT_TYPE << NOC_PTR_WIDTH) | write_ptr;
    // DWord count and valid bit, set active bit
    *(NOC_DMA_BASE+(dma_id<<1)+1) = (NOC_ACTIVE_BIT | (size << NOC_PTR_WIDTH) | read_ptr);
    
    
    return 1;
}

// Check if a NoC transfer has finished
#ifdef TRAP
int noc_done(unsigned dma_id) {
    int one = 1;
    asm volatile("mov $r2 = %0;"
                 "mov $r3 = %1;"
                   : : "r" (one), "r" (dma_id)
                   : "$r2", "$r3");
    trap(8);

    int ret = 0;
    asm volatile("mov %0 = $r1;"
                  : "=r" (ret));

    return ret;
}
#else
int noc_done(unsigned dma_id) {
    return k_noc_done(dma_id);
}
#endif

// Check if a NoC transfer has finished
int k_noc_done(unsigned dma_id) {
  //WRITE("k_noc_done\n",11);
  unsigned status = *(NOC_DMA_BASE+(dma_id<<1)+1);
  if ((status & NOC_ACTIVE_BIT) != 0) {
      return 0;
  }
  return 1;
}

// Start a NoC configuration transfer
// The addresses and the size are in double-words and relative to the
// communication SPM
int noc_conf(unsigned dma_id,
            unsigned short write_ptr,
            unsigned short read_ptr,
            unsigned short size) {

    // Only send if previous transfer is done
    if (!noc_done(dma_id)) {
      return 0;
    }

    // Read pointer and write pointer in the dma table
    *(NOC_DMA_BASE+(dma_id<<1)) = (CONFIG_PKT_TYPE << NOC_PTR_WIDTH) | write_ptr;
    // DWord count and valid bit, set active bit
    *(NOC_DMA_BASE+(dma_id<<1)+1) = (NOC_ACTIVE_BIT | (size << NOC_PTR_WIDTH) | read_ptr);

    

    return 1;
}

// Start a NoC interrupt
// The addresses and the size are in double-words and relative to the
// communication SPM
int noc_irq(unsigned dma_id,
            unsigned short write_ptr,
            unsigned short read_ptr) {

    // Only send if previous transfer is done
    if (!noc_done(dma_id)) {
      return 0;
    }

    // Read pointer and write pointer in the dma table
    *(NOC_DMA_BASE+(dma_id<<1)) = (IRQ_PKT_TYPE << NOC_PTR_WIDTH) | write_ptr;
    // DWord count and valid bit, set active bit
    *(NOC_DMA_BASE+(dma_id<<1)+1) = (NOC_ACTIVE_BIT | (1 << NOC_PTR_WIDTH) | read_ptr) ;
    
    

    return 1;
}

// Convert from byte address or size to double-word address or size
#define DW(X) (((X)+7)/8)

// Attempt to transfer data via the NoC
// The addresses and the size are in bytes
int noc_nbsend(unsigned dma_id, volatile void _SPM *dst,
               volatile void _SPM *src, size_t size) {

  unsigned wp = (char *)dst - (char *)NOC_SPM_BASE;
  unsigned rp = (char *)src - (char *)NOC_SPM_BASE;
  int ret = noc_dma(dma_id, DW(wp), DW(rp), DW(size));
  return ret;
}

// Transfer data via the NoC
// The addresses and the size are in bytes
void noc_send(unsigned dma_id, volatile void _SPM *dst,
              volatile void _SPM *src, size_t size) {
  _Pragma("loopbound min 1 max 1")
  while(!noc_nbsend(dma_id, dst, src, size));
}

// Multicast transfer of data via the NoC
// The addresses and the size are in bytes
void noc_multisend(unsigned cnt, unsigned dma_id [], volatile void _SPM *dst [],
              volatile void _SPM *src, size_t size) {

  int done;
  coreset_t sent;
  coreset_clearall(&sent);
  do {
    done = 1;
    for (unsigned i = 0; i < cnt; i++) {
      if (!coreset_contains(dma_id[i], &sent)) {
        if (noc_nbsend(dma_id[i], dst[i], src, size)) {
          coreset_add(dma_id[i], &sent);
        } else {
          done = 0;
        }
      }
    }
  } while(!done);
}

// Multicast transfer of data via the NoC
// The addresses and the size are in bytes
// The receivers are defined in a coreset
// the coreset must not contain the calling core.
void noc_multisend_cs(coreset_t *receivers, volatile void _SPM *dst[],
                      unsigned offset, volatile void _SPM *src, size_t size) {
  int index = 0;
  unsigned cpuid = get_cpuid();
//  for (unsigned i = 0; i < CORESET_SIZE; ++i) {
  for (unsigned i = 0; i < NOC_CORES; ++i) {
    if (coreset_contains(i,receivers)){
      if (i != cpuid) {
        noc_send(i, (volatile void _SPM *)((unsigned)dst[index]+offset), src, size);
      }
      //DEBUGGER("Transmission address: %x+%x at core %i\n",(unsigned)dst[index],offset,i);
      index++;
    }
  }
}
//void noc_multisend_cs(coreset_t receivers, volatile void _SPM *dst[],
//                                unsigned offset, volatile void _SPM *src, size_t size) {
//  int index;
//  int done;
//  coreset_t sent;
//  coreset_clearall(&sent);
//  do {
//    done = 1;
//    index = 0;
//    for(unsigned i = 0; i < CORESET_SIZE; i++) {
//      if(coreset_contains(i,&receivers) != 0) {
//        if(i != get_cpuid() && coreset_contains(i,&sent) == 0) {
//          if(noc_nbsend(i, (volatile void _SPM *)((unsigned)dst[index]+offset), src, size)) {
//            coreset_add(i,&sent);
//            DEBUGGER("Transmission address: %x+%x at core %i\n",(unsigned)dst[index],offset,i);
//          }
//        }
//        index++;
//      }
//    }
//  } while(!done);
//}

void noc_wait_dma(coreset_t receivers) {
  int index = 0;
  for (unsigned i = 0; i < NOC_CORES; ++i) {
    if (coreset_contains(i,&receivers)){
      if (i != get_cpuid()) {
        while(!noc_done(i));
      }
    }
  } 
}
