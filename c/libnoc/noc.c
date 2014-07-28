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
 *
 */

#include <stdio.h>

#include <machine/patmos.h>
#include <machine/spm.h>

#include "cmpboot.h"
#include "noc.h"

// Structure to model the network interface
static struct network_interface
{
    volatile int _IODEV *dma;
    volatile int _IODEV *dma_p;
    volatile int _IODEV *st;
} noc_interface = {
  NOC_DMA_BASE, NOC_DMA_P_BASE, NOC_ST_BASE
};

// Configure network interface according to initialization information
void noc_configure(void) {
  int row_size = NOC_TIMESLOTS > NOC_DMAS ? NOC_TIMESLOTS : NOC_DMAS;
  int core_idx = get_cpuid() * NOC_TABLES * row_size;
  for (unsigned i = 0; i < NOC_TIMESLOTS; ++i) {
    *(noc_interface.st+i) = noc_init_array[core_idx + i];
  }
  for (unsigned i = 0; i < NOC_DMAS; ++i) {
    *(noc_interface.dma_p+i) = noc_init_array[core_idx + row_size + i];
  }
}

// Synchronize start-up
static void noc_sync(void) {

  if (get_cpuid() == NOC_MASTER) {
    // Wait until all slaves have configured their network interface
    int done = 0;
    if (boot_info->master.status == STATUS_INITDONE) {
      puts("master.status = STATUS_INITDONE");
    }
    do {
      done = 1;
      for (unsigned i = 0; i < MAX_CORES; i++) {
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
}

// Start a NoC transfer
// The addresses and the size are in double-words and relative to the
// communication SPM
int noc_dma(unsigned rcv_id,
             unsigned short write_ptr,
             unsigned short read_ptr,
             unsigned short size) {

    // Ony send if previous transfer is done
    unsigned status = *(noc_interface.dma+(rcv_id<<1));
    if ((status & NOC_VALID_BIT) != 0 && (status & NOC_DONE_BIT) == 0) {
        return 0;
    }

    // Read pointer and write pointer in the dma table
    *(noc_interface.dma+(rcv_id<<1)+1) = (read_ptr << 16) | write_ptr;
    // DWord count and valid bit, done bit cleared
    *(noc_interface.dma+(rcv_id<<1)) = (size | NOC_VALID_BIT) & ~NOC_DONE_BIT;

    return 1;
}

// Convert from byte address or size to double-word address or size
#define DW(X) (((X)+7)/8)

// Attempt to transfer data via the NoC
// The addresses and the size are in bytes
int noc_nbsend(int dst_id, volatile void _SPM *dst,
               volatile void _SPM *src, size_t len) {

  unsigned wp = (char *)dst - (char *)NOC_SPM_BASE;
  unsigned rp = (char *)src - (char *)NOC_SPM_BASE;
  return noc_dma(dst_id, DW(wp), DW(rp), DW(len));
}

// Transfer data via the NoC
// The addresses and the size are in bytes
void noc_send(int dst_id, volatile void _SPM *dst,
              volatile void _SPM *src, size_t len) {

  while(!noc_nbsend(dst_id, dst, src, len));
}


