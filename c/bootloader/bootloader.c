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
 * Combined master and slave boot loader.
 * 
 * Author: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *         Torur Biskopsto Strom (torur.strom@gmail.com)
 *
 */

#include "boot.h"
#include <machine/boot.h>
#include "include/patio.h"
#include "include/bootable.h"

#define DELAY 1000*1

//#define DEBUG
//#define HEAVY_DEBUG

//#define data ((_UNCACHED int *)0x00000080)

int main(void)
{

  // wait a little bit in case of the TU/e memory controller not being ready
  int val = TIMER_US_LOW+DELAY;

  while (TIMER_US_LOW-val < 0)
    ;

  // overwrite potential leftovers from previous runs
  boot_info->master.status = STATUS_NULL;
  boot_info->master.entrypoint = NULL;
  if(get_cpuid() == 0) {
    for (unsigned i = 0; i < get_cpucnt(); i++) {
      boot_info->slave[i].status = STATUS_NULL;
      boot_info->slave[i].return_val = -1;
      boot_info->slave[i].param = NULL;
      boot_info->slave[i].funcpoint = NULL;
    }

    // give the slaves some time to boot
    for (unsigned i = 0; i < 0x10; i++) {
      boot_info->master.status = STATUS_BOOT;
    }

    // download application
    boot_info->master.entrypoint = download();
  }
  else {
    boot_info->slave[get_cpuid()].status = STATUS_NULL;
    boot_info->slave[get_cpuid()].return_val = -1;
    boot_info->slave[get_cpuid()].param = NULL;
    boot_info->slave[get_cpuid()].funcpoint = NULL;

    do {
      // make sure the own status is visible
      boot_info->slave[get_cpuid()].status = STATUS_BOOT;
      // until master has booted
    } while (boot_info->master.status != STATUS_BOOT);

    // wait until master has downloaded
    while (boot_info->master.status != STATUS_INIT) {
    /* spin */
    }
  }
  // initialize the content of the I-SPM from the main memory
  // copy words not bytes
  for (int i = 0; i < get_ispm_size()/4; ++i) {
    // starting at 64 K (1 << 16) word address (/4)
    *(SPM+(1<<16)/4+i) = *(MEM+(1<<16)/4+i);
  }


  if(get_cpuid() == 0) {

    // notify slaves that they can call _start()
    boot_info->master.status = STATUS_INIT;

    // wait for slaves to start
    for (unsigned i = 1; i < get_cpucnt(); i++) {
      while(boot_info->slave[i].status != STATUS_INIT){
        /* spin */
      }
    }
  }
  else {
    // acknowledge reception of start status
    boot_info->slave[get_cpuid()].status = STATUS_INIT;
  }

  // call the application's _start()
  int retval = -1;

  if (boot_info->master.entrypoint != NULL) {

    // enable global mode
    global_mode();

    retval = (*boot_info->master.entrypoint)();

    // Return may be "unclean" and leave registers clobbered.
    asm volatile ("" : :
                  : "r2", "r3", "r4", "r5",
                    "r6", "r7", "r8", "r9",
                    "r10", "r11", "r12", "r13",
                    "r14", "r15", "r16", "r17",
                    "r18", "r19", "r20", "r21",
                    "r22", "r23", "r24", "r25",
                    "r26", "r27", "r28", "r29",
                    "r30", "r31");

    // enable local mode again
    local_mode();

    boot_info->slave[get_cpuid()].return_val = retval;
  }

  if(get_cpuid() == 0) {

    // wait for slaves to finish
    for (unsigned i = 1; i < get_cpucnt(); i++) {
      if (boot_info->slave[i].status != STATUS_NULL) {
        while(boot_info->slave[i].status != STATUS_RETURN){
          /* spin */
        }
      }
    }

    // Print exit magic and return code
    WRITECHAR('\0');
    WRITECHAR('x');
    WRITECHAR(retval & 0xff);
    // notify slaves that they can loop back
    boot_info->master.status = STATUS_RETURN;

    // Wait for slaves to finish
    for (unsigned i = 1; i < get_cpucnt(); i++) {
      while(boot_info->slave[i].status == STATUS_RETURN){
        /* spin */
      }
    }
  }
  else {
    
    // wait until master application has returned
    do {
      // notify master that application has returned
      boot_info->slave[get_cpuid()].status = STATUS_RETURN;
    } while (boot_info->master.status != STATUS_RETURN); 
    
    boot_info->slave[get_cpuid()].status = STATUS_NULL;
  }
  
  
  // clear caches and loop back
  inval_dcache();
  inval_mcache();
  _start();

  return 0;
}
