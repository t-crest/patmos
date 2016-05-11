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

#include "noc.h"

//#define TRAP
  
// Find the size in bytes
int find_mem_size(volatile unsigned int _SPM * mem_addr){
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
int k_noc_sched_load(void) {
  unsigned int schedule_table_size = find_mem_size(NOC_SCHED_BASE);
  unsigned int noc_conf_base_ptr = 0;
  for (int j = 0; j < NOC_CONFS; ++j) {
    unsigned mode_idx = j * NOC_CORES * NOC_TABLES * NOC_SCHEDULE_ENTRIES;
    unsigned core_idx = get_cpuid() * NOC_TABLES * NOC_SCHEDULE_ENTRIES;
    unsigned short schedule_entries = noc_init_array[mode_idx+core_idx];
    if (noc_conf_base_ptr+schedule_entries >= schedule_table_size) {
      //WRITE("Configuration schedules do not fit in schedule table\n",53);
      //puts("Configuration schedules do not fit in schedule table");
      return 0;
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
  return 1;
    
}

void k_noc_sched_set(unsigned char config) {
  if (get_cpuid() == NOC_MASTER) {
    *(NOC_MC_BASE+0) = config; // Set the config in the network
    while(*(NOC_MC_BASE+0) != config);
  }
}

void k_noc_state_set(unsigned char state) {
  if (get_cpuid() == NOC_MASTER) {
    *(NOC_TDM_BASE+4) = state; // Set the run mode in the network
    while(*(NOC_TDM_BASE+4) != state);
  }
}

int k_noc_fifo_read(unsigned char irq_type){
  return *(NOC_IRQ_BASE+irq_type);
}

// Configure network interface according to initialization information
int k_noc_configure(void) {
  int ret;
  k_noc_state_set(0); // Make sure that the network is disabled
  ret = k_noc_sched_load();
  
  exc_register(18,&__data_recv_handler);
  exc_register(19,&__remote_irq_handler);
#ifdef TRAP
  exc_register(8,&__noc_trap_handler);
#endif
  return ret;
}


// Check if a NoC transfer has finished
int k_noc_dma_done(unsigned dma_id) {
  //WRITE("k_noc_done\n",11);
  unsigned status = *(NOC_DMA_BASE+(dma_id<<1)+1);
  if ((status & NOC_ACTIVE_BIT) != 0) {
      return 0;
  }
  return 1;
}


// Stops a NoC transfer and clear the DMA entry
void k_noc_dma_clear(unsigned dma_id) {
  // DWord count and valid bit, set active bit
  *(NOC_DMA_BASE+(dma_id<<1)+1) = 0;
  // Read pointer and write pointer in the dma table
  *(NOC_DMA_BASE+(dma_id<<1)) = 0;
  return;
}


int k_noc_dma_write(unsigned dma_id,
            unsigned short write_ptr,
            unsigned short read_ptr,
            unsigned short size,
            unsigned char pkt_type) {
    //DEBUGGER("k_noc_dma\n");
    // Only send if previous transfer is done
    if (!k_noc_dma_done(dma_id)) {
      return 0;
    }

    if (pkt_type > 3) {
      return 0;
    }
    // Read pointer and write pointer in the dma table
    *(NOC_DMA_BASE+(dma_id<<1)) = (pkt_type << NOC_PTR_WIDTH) | write_ptr;  
    // Word count and valid bit, set active bit
    *(NOC_DMA_BASE+(dma_id<<1)+1) = (NOC_ACTIVE_BIT | (size << NOC_PTR_WIDTH) | read_ptr);  
    
    return 1;
}

#ifndef TRAP

int noc_dma_write(unsigned dma_id,
            unsigned short write_ptr,
            unsigned short read_ptr,
            unsigned short size,
            unsigned char pkt_type) {
    return k_noc_dma_write(dma_id,write_ptr,read_ptr,size,pkt_type);
}

int noc_dma_done(unsigned dma_id) {
    return k_noc_dma_done(dma_id);
}

// Stops a NoC transfer and clear the DMA entry
void noc_dma_clear(unsigned dma_id) {
  k_noc_dma_clear(dma_id);
}

void noc_sched_set(unsigned char config) {
  k_noc_sched_set(config);
}

void noc_state_set(unsigned char state) {
  k_noc_state_set(state);
}

// Configure network interface according to initialization information
int noc_configure(void) {
  return k_noc_configure();
}

int noc_fifo_irq_read(){
  return k_noc_fifo_read(0);
}

int noc_fifo_data_read(){
  return k_noc_fifo_read(1);
}

#else

int noc_dma_write(unsigned dma_id,
            unsigned short write_ptr,
            unsigned short read_ptr,
            unsigned short size,
            unsigned char pkt_type) {
  unsigned int zero = 0;
  unsigned int ret_base, ret_offset;
  asm volatile("mov $r3 = %2;"
               "mov $r4 = %3;"
               "mov $r5 = %4;"
               "mov $r6 = %5;"
               "mov $r7 = %6;"
               "mov $r8 = %7;"
               "mfs %0 = $srb;"
               "mfs %1 = $sro;"
                 : "=r" (ret_base), "=r" (ret_offset)
                 : "r" (zero), "r" (dma_id), "r" (write_ptr), "r" (read_ptr), "r" (size), "r" (pkt_type)
                 : "$r3", "$r4", "$r5", "$r6", "$r7", "$r8" );
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

// Check if a NoC transfer has finished
int noc_dma_done(unsigned dma_id) {
  unsigned int one = 1;
  unsigned int ret_base, ret_offset;
  asm volatile("mov $r3 = %2;"
               "mov $r4 = %3;"
               "mfs %0 = $srb;"
               "mfs %1 = $sro;"
                 : "=r" (ret_base), "=r" (ret_offset)
                 : "r" (one), "r" (dma_id)
                 : "$r3", "$r4");
  trap(8);

  int ret;
  asm volatile("mov %0 = $r1;"
               "mts $srb = %1;"
               "mts $sro = %2;"
                : "=r" (ret)
                : "r" (ret_base), "r" (ret_offset)
                : "$r1");

  return ret;
}

// Stops a NoC transfer and clear the DMA entry
void noc_dma_clear(unsigned dma_id) {
  unsigned int two = 2;
  unsigned int ret_base, ret_offset;
  asm volatile("mov $r3 = %2;"
               "mov $r4 = %3;"
               "mfs %0 = $srb;"
               "mfs %1 = $sro;"
                 : "=r" (ret_base), "=r" (ret_offset)
                 : "r" (two), "r" (dma_id)
                 : "$r3", "$r4");
  trap(8);
  int ret;
  asm volatile("mts $srb = %0;"
               "mts $sro = %1;"
                : : "r" (ret_base), "r" (ret_offset));

  return;
}

void noc_sched_set(unsigned char config) {
  int three = 3;
  unsigned int ret_base, ret_offset;
  asm volatile("mov $r3 = %2;"
               "mov $r4 = %3;"
               "mfs %0 = $srb;"
               "mfs %1 = $sro;"
                 : "=r" (ret_base), "=r" (ret_offset)
                 : "r" (three), "r" (config)
                 : "$r3", "$r4");
  trap(8);

  asm volatile("mts $srb = %0;"
               "mts $sro = %1;"
                : : "r" (ret_base), "r" (ret_offset));

  return;
}

void noc_state_set(unsigned char state) {
  unsigned int four = 4;
  unsigned int ret_base, ret_offset;
  asm volatile("mov $r3 = %2;"
               "mov $r4 = %3;"
               "mfs %0 = $srb;"
               "mfs %1 = $sro;"
                 : "=r" (ret_base), "=r" (ret_offset)
                 : "r" (four), "r" (state)
                 : "$r3", "$r4");
  trap(8);

  asm volatile("mts $srb = %0;"
               "mts $sro = %1;"
                : : "r" (ret_base), "r" (ret_offset));

  return;
}

// Configure network interface according to initialization information
int noc_configure(void) {
  // int five = 5;
  // asm volatile("mov $r3 = %0;"
  //                : : "r" (five)
  //                : "$r3");
  // trap(8);

  // int ret = 0;
  // asm volatile("mov %0 = $r1;"
  //               : "=r" (ret));

  // return ret;

  return k_noc_configure();
}

#endif

void noc_enable(void) {
  noc_state_set(1);
}

void noc_disable(void) {
  noc_state_set(0);
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

// Convert from byte address or size to word address or size
#define W(X) (((X)+3)/4)

// Start a NoC configuration transfer
// The addresses and the size are in words and relative to the
// communication SPM
int noc_conf(unsigned dma_id, volatile void _SPM *dst,
               volatile void _SPM *src, size_t size) { 
  unsigned wp = (char *)dst - (char *)NOC_SPM_BASE;
  unsigned rp = (char *)src - (char *)NOC_SPM_BASE;   
  return noc_dma_write(dma_id, W(wp), W(rp), W(size), CONFIG_PKT_TYPE);
}


// Start a NoC interrupt
// The addresses and the size are in words and relative to the
// communication SPM
int noc_irq(unsigned dma_id, volatile void _SPM *dst,
               volatile void _SPM *src) {
  unsigned wp = (char *)dst - (char *)NOC_SPM_BASE;
  unsigned rp = (char *)src - (char *)NOC_SPM_BASE;
  return noc_dma_write(dma_id, W(wp), W(rp), 1, IRQ_PKT_TYPE);
}

// Attempt to transfer data via the NoC
// The addresses and the size are in bytes
int noc_nbwrite(unsigned dma_id, volatile void _SPM *dst,
               volatile void _SPM *src, size_t size, unsigned irq_enable) {
  unsigned int pkt_type;
  if (irq_enable == 1) {
    // Enable an interrupt at the end of the DMA transfer
    pkt_type = DATA_IRQ_PKT_TYPE;  
  } else if (irq_enable == 0) {
    // Disable an interrupt at the end of the DMA transfer
    pkt_type = DATA_PKT_TYPE;  
  } else {
    // Wrong parameter
    return 0;
  }
  unsigned wp = (char *)dst - (char *)NOC_SPM_BASE;
  unsigned rp = (char *)src - (char *)NOC_SPM_BASE;
  int ret = noc_dma_write(dma_id, W(wp), W(rp), W(size), pkt_type);
  return ret;
}

// Transfer data via the NoC
// The addresses and the size are in bytes
void noc_write(unsigned dma_id, volatile void _SPM *dst,
              volatile void _SPM *src, size_t size, unsigned irq_enable) {
  _Pragma("loopbound min 1 max 1")
  while(!noc_nbwrite(dma_id, dst, src, size, irq_enable));
}

// Multicast transfer of data via the NoC
// The addresses and the size are in bytes
void noc_multisend(unsigned cnt, unsigned dma_id [], volatile void _SPM *dst [],
              volatile void _SPM *src, size_t size, unsigned irq_enable) {

  int done;
  coreset_t sent;
  coreset_clearall(&sent);
  do {
    done = 1;
    for (unsigned i = 0; i < cnt; i++) {
      if (!coreset_contains(dma_id[i], &sent)) {
        if (noc_nbwrite(dma_id[i], dst[i], src, size, irq_enable)) {
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
                      unsigned offset, volatile void _SPM *src, size_t size,
                                                      unsigned irq_enable) {
  int index = 0;
  unsigned cpuid = get_cpuid();
//  for (unsigned i = 0; i < CORESET_SIZE; ++i) {
  for (unsigned i = 0; i < NOC_CORES; ++i) {
    if (coreset_contains(i,receivers)){
      if (i != cpuid) {
        noc_write(i, (volatile void _SPM *)((unsigned)dst[index]+offset), src,
                                                            size, irq_enable);
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
//          if(noc_nbwrite(i, (volatile void _SPM *)((unsigned)dst[index]+offset), src, size)) {
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
        while(!noc_dma_done(i));
      }
    }
  } 
}

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

int _noc_trap_handler(unsigned int op,
                       unsigned int p1,
                       unsigned int p2,
                       unsigned int p3,
                       unsigned int p4,
                       unsigned int p5) {
  intr_enable();
  //DEBUGGER("TRAP, op: %d\n",op);
  int ret = 1;
  switch (op) {
    case 0:
      DEBUGGER("noc_dma_wite p1: %d, p2: %d, p3: %d, p4: %d, p5: %d\n", p1,p2,p3,p4,p5);
      ret = k_noc_dma_write(p1,(unsigned short)p2,(unsigned short)p3,(unsigned short)p4,(unsigned char)p5);
      break;
    case 1:
      //DEBUGGER("noc_dma_done\n");
      ret = k_noc_dma_done(p1);
      break;
    case 2:
      //DEBUGGER("noc_dma_clear\n");
      k_noc_dma_clear(p1);
      break;
    case 3:
      //DEBUGGER("noc_sched_set\n");
      k_noc_sched_set((unsigned char)p1);
      break;
    case 4:
      //DEBUGGER("noc_state_set\n");
      k_noc_state_set((unsigned char)p1);
      break;
    case 5:
      //DEBUGGER("noc_configure\n");
      ret = k_noc_configure();
      break;
    default:
      break;
      //DEBUGGER("Illegal op\n");
  }
  intr_disable();
  //DEBUGGER("Returning from trap\n");
  return ret;
}


void __noc_trap_handler(void)  __attribute__((naked));
void __noc_trap_handler(void) {
  
  unsigned int op,p1,p2,p3,p4,p5;
  unsigned int ret_base,ret_offset;
  unsigned int pret;
  asm volatile("mov %0 = $r3;"
               "mov %1 = $r4;"
               "mov %2 = $r5;"
               "mov %3 = $r6;"
               "mov %4 = $r7;"
               "mov %5 = $r8;"
               "mfs %6 = $sxb;"
               "mfs %7 = $sxo;"
               "mfs %8 = $s0;"
                 : "=r" (op), "=r" (p1), "=r" (p2), "=r" (p3), "=r" (p4), "=r" (p5), "=r" (ret_base), "=r" (ret_offset), "=r" (pret)
                 : : "$r3", "$r4", "$r5", "$r6", "$r7", "$r8");

  int ret = _noc_trap_handler(op,p1,p2,p3,p4,p5);

  asm volatile("mts $sxb = %0;"
               "mts $sxo = %1;"
               "mts $s0 = %2;"
               "mov $r1 = %3;"
               "xretnd;"
                : : "r" (ret_base), "r" (ret_offset), "r" (pret), "r" (ret)
                : "$r1");
}


#endif

