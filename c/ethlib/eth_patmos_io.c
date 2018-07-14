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

#include "eth_patmos_io.h"

// Write to ethernet controller
void eth_iowr(unsigned addr,unsigned data) {
    *(ETH_BASE+(addr >> 2)) = data;
    return;
}      

// Read ethernet controller
unsigned eth_iord(unsigned addr) {
    return *(ETH_BASE+(addr>>2));
}      

// Write rx-tx buffer
void mem_iowr(unsigned addr, unsigned data) {
    *(BUFF_BASE+(addr>>2)) = data;
    return;
}

// Write a byte in rx-tx buffer
void mem_iowr_byte(unsigned addr, unsigned data) {
    unsigned previous_data = *(BUFF_BASE+(addr>>2));
    unsigned shift_factor = (24 - (8 * (addr & 0x03)));
    unsigned mask = ~(0x000000FF << shift_factor);
    data = (0x000000FF & data) << shift_factor;
    data = (previous_data & mask) + data;
    *(BUFF_BASE+(addr>>2)) = data;
    return;
}

// Read rx-tx buffer
unsigned mem_iord(int addr) {
    return *(BUFF_BASE+(addr>>2));
}

// Write a byte in rx-tx buffer
unsigned mem_iord_byte(unsigned addr) {
    unsigned full_data = *(BUFF_BASE+(addr>>2));
    unsigned shift_factor = (24 - (8 * (addr & 0x03)));
    unsigned mask = 0x000000FF << shift_factor;
    return (full_data & mask) >> shift_factor;
}

// Similar functions for second ethernet controller (when present)
// Write to ethernet controller
void eth_iowr1(unsigned addr,unsigned data) {
    *(ETH1_BASE+(addr >> 2)) = data;
    return;
}      

// Read ethernet controller
unsigned eth_iord1(unsigned addr) {
    return *(ETH1_BASE+(addr>>2));
}      

// Write rx-tx buffer
void mem_iowr1(unsigned addr, unsigned data) {
    *(BUFF1_BASE+(addr>>2)) = data;
    return;
}

// Write a byte in rx-tx buffer
void mem_iowr1_byte(unsigned addr, unsigned data) {
    unsigned previous_data = *(BUFF1_BASE+(addr>>2));
    unsigned shift_factor = (24 - (8 * (addr & 0x03)));
    unsigned mask = ~(0x000000FF << shift_factor);
    data = (0x000000FF & data) << shift_factor;
    data = (previous_data & mask) + data;
    *(BUFF1_BASE+(addr>>2)) = data;
    return;
}

// Read rx-tx buffer
unsigned mem_iord1(int addr) {
    return *(BUFF1_BASE+(addr>>2));
}

// Write a byte in rx-tx buffer
unsigned mem_iord_byte1(unsigned addr) {
    unsigned full_data = *(BUFF1_BASE+(addr>>2));
    unsigned shift_factor = (24 - (8 * (addr & 0x03)));
    unsigned mask = 0x000000FF << shift_factor;
    return (full_data & mask) >> shift_factor;
}

