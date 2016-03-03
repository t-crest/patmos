/*
   Copyright 2015 Technical University of Denmark, DTU Compute. 
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
 * Message passing API
 * 
 * Author: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
 *
 */

#include "mp.h"
#include "mp_internal.h"
#define TRACE_LEVEL INFO
#define DEBUG_ENABLE
#include "include/debug.h"

LOCK_T * initialize_lock(unsigned remote) {
    LOCK_T * lock = (LOCK_T *)mp_alloc(sizeof(LOCK_T));
    lock->remote_entering = 0;
    lock->remote_number = 0;
    lock->local_entering = 0;
    lock->local_number = 0;  
    lock->remote_cpuid = remote;
    return lock;
}

void acquire_lock(LOCK_T * lock){
    /* Write Entering true */
    /* Write Number */
    unsigned remote = lock->remote_cpuid;
    unsigned id = get_cpuid();
    lock->local_entering = 1;
    
    noc_send(remote,
              (void _SPM *)&(lock->remote_ptr->remote_entering),
              (void _SPM *)&lock->local_entering,
              sizeof(lock->local_entering));

    //#pragma loopbound min 1 max 2
    #pragma loopbound min PKT_TRANS_WAIT max PKT_TRANS_WAIT
    while(!noc_done(remote));
    unsigned n = (unsigned)lock->remote_number + 1;
    lock->local_number = n;
    /* Enforce memory barrier */
    noc_send(remote,
              (void _SPM *)&(lock->remote_ptr->remote_number),
              (void _SPM *)&lock->local_number,
              sizeof(lock->local_number));

//    /* Enforce memory barrier */
    #pragma loopbound min PKT_TRANS_WAIT max PKT_TRANS_WAIT
    while(!noc_done(remote)); // noc_send() also waits for the dma to be
                                // free, so no need to do it here as well

    /* Write Entering false */
    lock->local_entering = 0;
    noc_send(remote,
              (void _SPM *)&(lock->remote_ptr->remote_entering),
              (void _SPM *)&lock->local_entering,
              sizeof(lock->local_entering));

    /* Wait for remote core not to change number */
    #pragma loopbound min 1 max 2
    while(lock->remote_entering == 1);
    /* Wait to be the first in line to the bakery queue */
    unsigned m = lock->remote_number;
    #pragma loopbound min 1 max 2
    while( (m != 0) &&
            ( (m < n) || ((m == n) && ( remote < id)))) {
      m = lock->remote_number;
    }
    /* Lock is grabbed */  
    return;
}

void release_lock(LOCK_T * lock) {
    /* Write Number */
    lock->local_number = 0;
    noc_send(lock->remote_cpuid,
              (void _SPM *)&(lock->remote_ptr->remote_number),
              (void _SPM *)&lock->local_number,
              sizeof(lock->local_number));
    /* Enforce memory barrier */
    #pragma loopbound min PKT_TRANS_WAIT max PKT_TRANS_WAIT
    while(!noc_done(lock->remote_cpuid));
    /* Lock is freed */  
    return;
}

void close_lock(LOCK_T * lock) {

}
