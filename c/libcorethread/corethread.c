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

////////////////////////////////////////////////////////////////////////////
// Functions for initializing the workers
////////////////////////////////////////////////////////////////////////////

void corethread_worker(void) {
   if (get_cpuid() != 0) {
      unsigned long long time;
      boot_info->slave[get_cpuid()].status = STATUS_RETURN;
      
      // Wait for corethread_create request or application exit
      while(boot_info->master.status != STATUS_RETURN) {
         // As long as the master is still executing, wait for a corethread to
         // be created and then execute it.
         if (boot_info->slave[get_cpuid()].funcpoint != NULL) {
            funcpoint_t funcpoint = boot_info->slave[get_cpuid()].funcpoint;
            boot_info->slave[get_cpuid()].return_val = -1;
            boot_info->slave[get_cpuid()].status = STATUS_INITDONE;
            (*funcpoint)((void*)boot_info->slave[get_cpuid()].param);
            boot_info->slave[get_cpuid()].status = STATUS_RETURN;
            while(boot_info->slave[get_cpuid()].funcpoint != NULL) {

            }
         }
         //time = get_cpu_usecs();
         //while(get_cpu_usecs() < time+1) {
         //
         //}
      }
      boot_info->slave[get_cpuid()].status = STATUS_RETURN;
      exit(0);
   }
   return;
}

////////////////////////////////////////////////////////////////////////////
// Functions for creating and destroying corethreads
////////////////////////////////////////////////////////////////////////////

int corethread_create(corethread_t *thread, const corethread_attr_t *attr,
                                    void (*start_routine)(void*), void *arg) {
   if(boot_info->slave[*thread].status == STATUS_RETURN &&
      boot_info->slave[*thread].funcpoint == NULL ) {
      boot_info->slave[*thread].param = arg;
      boot_info->slave[*thread].funcpoint = (funcpoint_t) start_routine;
      while(boot_info->slave[*thread].status != STATUS_INITDONE) {

      }
      return 0;
   } else {
      return EAGAIN;
   }
   // TODO: use attribute
}

void corethread_exit(void *retval) {
   boot_info->slave[get_cpuid()].return_val = (int) retval;
   //boot_info->slave[get_cpuid()].funcpoint = NULL;
   boot_info->slave[get_cpuid()].status = STATUS_RETURN;
   return;
}

int corethread_join(corethread_t thread, void **retval) {
   //inval_dcache();
   unsigned long long time;
   while(boot_info->slave[thread].status != STATUS_RETURN) {
      //inval_dcache();
   //   if (get_cpuid() == 0) {
   //      printf("Status of thread %zu is %i funcpoint: %x\n",thread,boot_info->slave[thread].status,boot_info->slave[thread].funcpoint);
   //      printf("Status of master is %i\n",boot_info->master.status);
   //   }
      //inval_dcache();
      //time = get_cpu_usecs();
      //while(get_cpu_usecs() < time+10) {
      //
      //}
   }
   *retval = (void *) boot_info->slave[thread].return_val;
   boot_info->slave[thread].funcpoint = NULL;
   return 0;
}

////////////////////////////////////////////////////////////////////////////
// Help functions 
////////////////////////////////////////////////////////////////////////////

