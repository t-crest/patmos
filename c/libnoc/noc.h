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
 * Definitions for the NoC.
 * 
 * Author: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

#ifndef _NOC_H_
#define _NOC_H

#include <machine/patmos.h>
#include <machine/spm.h>

// Definitions used for initialization of network interface
extern const int NOC_CORES;
extern const int NOC_MASTER;
extern const int NOC_TABLES;
extern const int NOC_TIMESLOTS;
extern const int NOC_DMAS;
extern const int noc_init_array [];

// Configure network interface according to initialization information
void noc_configure(void);

// Initialize notwork-on-chip and synchronize
void noc_init(void) __attribute__((constructor,used));

#ifdef NOC_INIT
// Pull in initializer, even if nothing else from the library is used
static const void * volatile __noc_include __attribute__((used)) = &noc_init;
#endif

// Start a NoC transfer
// The addresses and the size are in double-words and relative to the
// communication SPM
int noc_send(unsigned rcv_id, unsigned short write_ptr,
             unsigned short read_ptr, unsigned short size);

// Transfer data via the NoC
// The addresses and the size are in bytes
void noc_dma(int dst_id, volatile void _SPM *dst,
             volatile void _SPM *src, size_t len);

// Definitions for setting up a transfer
#define NOC_VALID_BIT 0x08000
#define NOC_DONE_BIT 0x04000

// Definitions for address mapping
#define NOC_DMA_BASE    ((volatile int _IODEV *)0xE0000000)
#define NOC_DMA_P_BASE  ((volatile int _IODEV *)0xE1000000)
#define NOC_ST_BASE     ((volatile int _IODEV *)0xE2000000)
#define NOC_SPM_BASE    ((volatile int _SPM   *)0xE8000000)

#endif /* _NOC_H_ */
