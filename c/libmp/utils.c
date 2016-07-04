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
 * Utility functions for the Message passing library
 * 
 * Author: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
 *
 */

#include "mp.h"
#include "mp_internal.h"
#include "mp_loopbound.h"


size_t mp_send_alloc_size(qpd_t * qpd_ptr) {
  size_t send_size = (qpd_ptr->buf_size + FLAG_SIZE) * NUM_WRITE_BUF
                                  + WALIGN(sizeof(*(qpd_ptr->send_recv_count)));
  return send_size;
}

size_t mp_recv_alloc_size(qpd_t * qpd_ptr) {
  size_t recv_size = (qpd_ptr->buf_size + FLAG_SIZE) * qpd_ptr->num_buf
                                  + WALIGN(sizeof(*(qpd_ptr->recv_count)));
  return recv_size;
}

int test_spm_size(){
  volatile unsigned int _SPM * addr = NOC_SPM_BASE;
  int init = *(addr);
  int tmp;
  *(addr) = 0xFFEEDDCC;
  int i = 2;
  int j = 0;
  for(j = 0; j < 28; j++) {
    tmp = *(addr+i);
    *(addr+i) = 0;
    if (*(addr) == 0) {
      // We found the address where the mapping of the SPM wraps
      // Restore the state of the memory as is was when the function was called
      *(addr+i) = tmp;
      *(addr) = init;
      // Remember to multiply by 4 for the byte address
      return i << 2;
    }
    i = i << 1;
    if (*(addr) != 0xFFEEDDCC){
      // Memory failure happened
      *(addr+i) = tmp;
      *(addr) = init;
      return -2;
    }
    *(addr+i) = tmp;
  }
  *(addr) = init;
  return -1;
}

static inline void mem_copy(int _SPM * to, int _SPM * from, int bytes){
  // Since we want to copy 32 bit at the time we divide bytes by 4
  unsigned itteration_count = (bytes + 4 - 1) / 4; // equal to ceil(bytes/4)
  #pragma loopbound min MSG_SIZE_WORDS max MSG_SIZE_WORDS
  for (int i = 0; i < itteration_count; ++i) {
    to[i] = from[i];
  }
}



