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
 * IO functions of ethlib (ethernet library)
 * 
 * Authors: Luca Pezzarossa (lpez@dtu.dk)
 *          Jakob Kenn Toft
 *          Jesper Lønbæk
 *          Russell Barnes
 */

#ifndef _ETH_PATMOS_IO_H_
#define _ETH_PATMOS_IO_H_

#include <machine/patmos.h>

// Pointers to the base addresses, all the addressing (addr as arguments)
// in the library are an offset on these addresses
#define ETH_DEV_BASE    0xF00D0000

#define ETH_BASE  ((volatile _IODEV unsigned *) (ETH_DEV_BASE + 0xF000))
#define BUFF_BASE ((volatile _IODEV unsigned *) (ETH_DEV_BASE + 0x0000))

// Base addresses of second Ethernet controller (when present)
#define ETH1_DEV_BASE    0xF00E0000
#define ETH1_BASE  ((volatile _IODEV unsigned *) (ETH1_DEV_BASE + 0xF000))
#define BUFF1_BASE ((volatile _IODEV unsigned *) (ETH1_DEV_BASE + 0x0000))

// Write to ethernet controller
void eth_iowr(unsigned addr,unsigned data);

// Read ethernet controller
unsigned eth_iord(unsigned addr);

// Write rx-tx buffer
void mem_iowr(unsigned addr, unsigned data);

// Write a byte in rx-tx buffer
void mem_iowr_byte(unsigned addr, unsigned data) __attribute__((noinline));

// Read rx-tx buffer
unsigned mem_iord(int addr);

// Write a byte in rx-tx buffer
unsigned mem_iord_byte(unsigned addr) __attribute__((noinline));

// Write to ethernet controller
void eth_iowr1(unsigned addr,unsigned data);

// Read ethernet controller
unsigned eth_iord1(unsigned addr);

// Write rx-tx buffer
void mem_iowr1(unsigned addr, unsigned data);

// Write a byte in rx-tx buffer
void mem_iowr_byte1(unsigned addr, unsigned data) __attribute__((noinline));

// Read rx-tx buffer
unsigned mem_iord1(int addr);

// Write a byte in rx-tx buffer
unsigned mem_iord_byte1(unsigned addr) __attribute__((noinline));

#endif
