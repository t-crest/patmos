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
 * Create entry point when being compiled for boot ROM. To be included
 * at the beginning of the application. Configures NoC before calling
 * main if included after libnoc/noc.h.
 * 
 * Author: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 */

#ifndef _BOOTABLE_H_
#define _BOOTABLE_H_

#include "include/patio.h"

#ifdef BOOTROM

#define CACHECTRL *((volatile _IODEV int *)0xF0010014)
#define local_mode()  do { if (CACHECTRL >= 0) { CACHECTRL = 0x80000000; } \
    asm volatile("nop; nop;"); } while(0)
#define global_mode() do { if (CACHECTRL < 0)  { CACHECTRL = 0x80000000; } \
    asm volatile("nop; nop;"); } while(0)

extern int _stack_cache_base, _shadow_stack_base;
int main(void);
void _start(void) __attribute__((naked,used));

void _start(void) {
  // setup stack frame and stack cache.
  asm volatile ("mov $r31 = %0;" // initialize shadow stack pointer"
                "mts $ss  = %1;" // initialize the stack cache's spill pointer"
                "mts $st  = %1;" // initialize the stack cache's top pointer"
                : : "r" (&_shadow_stack_base),
                  "r" (&_stack_cache_base));

  // enable local mode
  local_mode();

#ifdef _NOC_H_
  // configure network interface
  noc_init();
#endif /* _NOC_H_ */

  // call main()
  main();
  // freeze
  for(;;);
}
#endif

#endif /* _BOOTABLE_H_ */
