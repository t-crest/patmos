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
 * Definitions for boot loaders.
 * 
 * Authors: Tórur Biskopstø Strøm (torur.strom@gmail.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 */

#ifndef _BOOT_H_
#define _BOOT_H_

#include <machine/patmos.h>
#include <machine/spm.h>

#define UART_STATUS *((volatile _IODEV int *) 0xF0000800)
#define UART_DATA   *((volatile _IODEV int *) 0xF0000804)
#define LEDS        *((volatile _IODEV int *) 0xF0000900)

#define MEM         ((volatile _UNCACHED int *) 0x0)
#define SPM         ((volatile _SPM int *) 0x0)

#define XDIGIT(c) ((c) <= 9 ? '0' + (c) : 'a' + (c) - 10)

#define WRITE(data,len) do { \
  unsigned i; \
  for (i = 0; i < (len); i++) {		   \
    while ((UART_STATUS & 0x01) == 0); \
    UART_DATA = (data)[i];			   \
  } \
} while(0)

int main(void) __attribute__((naked,used));
extern int _stack_cache_base, _shadow_stack_base;

typedef volatile int (*entrypoint_t)(void);
entrypoint_t download(void) __attribute__((noinline));

#endif /* _BOOT_H_ */
