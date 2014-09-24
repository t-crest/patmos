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
 * Management of corethreads
 * 
 * Author: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
 *
 */

#include "corethread.h"

#include <stdio.h>
//#include <stdlib.h>
//#include "libmp/mp.h"
//#include "bootloader/cmpboot.h"

////////////////////////////////////////////////////////////////////////////
// Functions for initializing the workers
////////////////////////////////////////////////////////////////////////////

void corethread_worker() {
   if (get_cpuid() == 0) {
      return;
   }
   boot_info->slave[get_cpuid()].status = STATUS_RETURN;
   volatile void _SPM *return_addr;
   // Wait for corethread_create request or application exit
   while(boot_info->master.status != STATUS_RETURN) {
      // As long as the master is still executing wait for at corethread to
      // be created and then execute it.
      if (/* Check recv flag */) {
         // Setup the return channel
         return_addr = (*work_recv.read_buf+12);
         (*work_recv.read_buf)((void*)*(work_recv.read_buf+4)); //(*start_routine)(arg);
      }
      boot_info->slave[get_cpuid()].status = STATUS_RETURN;
   }

   exit(0);
}

int corethread_init(volatile corethread_t _SPM *corethread_ptr, int recv_id,
                                          volatile void _SPM *ret_addr) {

   volatile void _SPM *remote_addr = NOC_SPM_BASE;
}

////////////////////////////////////////////////////////////////////////////
// Functions for creating and destroying corethreads
////////////////////////////////////////////////////////////////////////////

int corethread_create(volatile corethread_t _SPM *corethread_ptr,
      const corethread_attr_t *attr, void *(*start_routine)(void*), void *arg) {
   // Copy the function pointer, arg pointer and attributes to the COM SPM
   // Send the copied 
   //noc_send();
   return 0;
}

////////////////////////////////////////////////////////////////////////////
// Help functions 
////////////////////////////////////////////////////////////////////////////

