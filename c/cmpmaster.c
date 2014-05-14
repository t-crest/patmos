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
 * Master for CMP boot loader.
 * 
 * Author: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

#include "boot.h"
#include "cmpboot.h"
#include "patio.h"

#include "bootable.h"

#define TIM TIMER_USLOW
#define DELAY 1000*1

// #define DEBUG

int main(void)
{

#ifdef DEBUG
  WRITE("BOOT\n", 5);
#endif

  // wait a little bit in case of the TU/e memory controller not being ready
  int val = TIM+DELAY;
  while (TIM-val < 0)
    ;

  // overwrite potential leftovers from previous runs
  boot_info->master.status = STATUS_NULL;
  boot_info->master.entrypoint = NULL;
  for (unsigned i = 0; i < MAX_CORES; i++) {
    boot_info->slave[i].status = STATUS_NULL;
  }

  // give the slaves some time to boot
  for (unsigned i = 0; i < 0x10; i++) {
    boot_info->master.status = STATUS_BOOT;
  }

#ifdef DEBUG
  WRITE("DOWN\n", 5);
#endif

  // download application
  boot_info->master.entrypoint = download();
#ifdef DEBUG
  // force some valid address for debugging
  if (boot_info->master.entrypoint == NULL) {
    boot_info->master.entrypoint = 0x20004;
  }
#endif

  // notify slaves that they can call _start()
  boot_info->master.status = STATUS_INIT;

  static char msg[10];

#ifdef DEBUG
  WRITE("START ", 6);
    
  msg[0] = XDIGIT(((int)boot_info->master.entrypoint >> 28) & 0xf);
  msg[1] = XDIGIT(((int)boot_info->master.entrypoint >> 24) & 0xf);
  msg[2] = XDIGIT(((int)boot_info->master.entrypoint >> 20) & 0xf);
  msg[3] = XDIGIT(((int)boot_info->master.entrypoint >> 16) & 0xf);
  msg[4] = XDIGIT(((int)boot_info->master.entrypoint >> 12) & 0xf);
  msg[5] = XDIGIT(((int)boot_info->master.entrypoint >>  8) & 0xf);
  msg[6] = XDIGIT(((int)boot_info->master.entrypoint >>  4) & 0xf);
  msg[7] = XDIGIT(((int)boot_info->master.entrypoint >>  0) & 0xf);
  msg[8] = '\n';
  WRITE(msg, 9);
#endif

  // call the application's _start()
  int retval = -1;
  if (boot_info->master.entrypoint != NULL) {
    retval = (*boot_info->master.entrypoint)();

    // Return may be "unclean" and leave registers clobbered.
    asm volatile ("" : :
                  : "$r2", "$r3", "$r4", "$r5",
                    "$r6", "$r7", "$r8", "$r9",
                    "$r10", "$r11", "$r12", "$r13",
                    "$r14", "$r15", "$r16", "$r17",
                    "$r18", "$r19", "$r20", "$r21",
                    "$r22", "$r23", "$r24", "$r25",
                    "$r26", "$r27", "$r28", "$r29",
                    "$r30", "$r31");
  }

  // TODO: wait for slaves to finish

#ifdef DEBUG
  WRITE("EXIT\n", 5);
#endif

  // Print exit magic and return code
  msg[0] = '\0';
  msg[1] = 'x';
  msg[2] = retval & 0xff;
  WRITE(msg, 3);

  // loop back, TODO: replace with a real reset
  main();

  return 0;
}
