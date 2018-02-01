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
/*
void corethread_worker(void) {
  unsigned id = get_cpuid();
  if (id != NOC_MASTER) { // Core zero should proceed to execute main
    unsigned long long time;
    boot_info->slave[id].status = STATUS_RETURN;
      
    // Wait for corethread_create request or application exit
    while(boot_info->master.status != STATUS_RETURN) {
      // As long as the master is still executing, wait for a corethread to
      // be created and then execute it.
      if (boot_info->slave[id].funcpoint != NULL) {
        funcpoint_t funcpoint = boot_info->slave[id].funcpoint;
        boot_info->slave[id].return_val = -1;
        boot_info->slave[id].status = STATUS_INITDONE;
        (*funcpoint)((void*)boot_info->slave[id].param);
        boot_info->slave[id].status = STATUS_RETURN;
        while(boot_info->slave[id].funcpoint != NULL) {

        }
      }
      time = get_cpu_usecs();
      while(get_cpu_usecs() < time+10) {
        // Wait for 10 micro seconds before checking again
      }
    }
    boot_info->slave[id].status = STATUS_RETURN;
    exit(0);
  }
  return;
}
*/
////////////////////////////////////////////////////////////////////////////
// Functions for creating and destroying corethreads
////////////////////////////////////////////////////////////////////////////

int corethread_create(int core_id, void (*start_routine)(void*),
                                                                    void *arg) {
  if(boot_info->slave[core_id].status != STATUS_INITDONE &&
                                boot_info->slave[core_id].funcpoint == NULL ) {
    boot_info->slave[core_id].param = arg;
    boot_info->slave[core_id].funcpoint = (funcpoint_t) start_routine;
    while(boot_info->slave[core_id].status != STATUS_INITDONE) {
      // Wait for corethread to respond
    }
    return 0;
  } else {
    // Corethread is not available
    return EAGAIN;
  }
}

void corethread_exit(void *retval) {
  unsigned id = get_cpuid();
  boot_info->slave[id].return_val = (int) retval;
  boot_info->slave[id].status = STATUS_RETURN;
  return;
}

int corethread_join(int core_id, void **retval) {
  unsigned long long time;
  while(boot_info->slave[core_id].status != STATUS_RETURN) {
    time = get_cpu_usecs();
    while(get_cpu_usecs() < time+10) {
      // Wait for 10 microseconds before checking again
    }
  }
  *retval = (void *) boot_info->slave[core_id].return_val;
  boot_info->slave[core_id].funcpoint = NULL;
  return 0;
}
