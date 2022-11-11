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

#define CACHECTRL 0xF0010014
#define CACHECTRL_LOCAL_MODE_BIT_SET 0x80000000
#define local_mode() asm volatile ( \
	"li 			$r1 = %[addr];" \
	"lwl 			$r2 = [$r1];" \
	"li 			$r3 = %[new_val];" \
	"cmplt 			$p1 = $r2, 0;" \
	"swl	(!$p1)	[$r1] = $r3;" \
	:  \
	: [addr] "i" (CACHECTRL), [new_val] "i" (CACHECTRL_LOCAL_MODE_BIT_SET) \
	: "r1", "r2", "r3", "p1" \
  )
#define global_mode() asm volatile ( \
	"li 			$r1 = %[addr];" \
	"lwl 			$r2 = [$r1];" \
	"li 			$r3 = %[new_val];" \
	"cmplt 			$p1 = $r2, 0;" \
	"swl	( $p1)	[$r1] = $r3;" \
	:  \
	: [addr] "i" (CACHECTRL), [new_val] "i" (CACHECTRL_LOCAL_MODE_BIT_SET) \
	: "r1", "r2", "r3", "p1" \
  )

extern int _stack_cache_base, _shadow_stack_base;
int main(void);
void _start(void) __attribute__((naked,used));

void _start(void) {
  // setup stack frame and stack cache.
  asm volatile (
	"mov $r31 = %0;" // initialize shadow stack pointer"
	"mts $ss  = %1;" // initialize the stack cache's spill pointer"
	"mts $st  = %1;" // initialize the stack cache's top pointer"				
	: : "r" (&_shadow_stack_base),
	  "r" (&_stack_cache_base)
  );

  // enable local mode
  local_mode();

#ifdef _NOC_H_
  // configure network interface
  asm volatile(
	"li $r1 = %[addr];"
	"callnd $r1;" 
	:
	: [addr] "i" (noc_init)
	: "r1"
  ); 
#endif /* _NOC_H_ */

  // call main()
  asm volatile(
	"li $r1 = %[addr];"
	"callnd $r1;" 
	
	// freeze
	"brnd 0"
	:
	: [addr] "i" (main)
	: "r1"
  ); 
}
#endif

#endif /* _BOOTABLE_H_ */
