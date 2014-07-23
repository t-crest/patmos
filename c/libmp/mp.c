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
 * Message passing API
 * 
 * Author: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
 *
 */

#include <stdio.h>

#include <machine/patmos.h>
#include <machine/spm.h>

#include "cmpboot.h"
#include "mp.h"

////////////////////////////////////////////////////////////////////////////
// Functions for initializing the message passing API
////////////////////////////////////////////////////////////////////////////

void mp_send_init(mp_t* mp_ptr, int rcv_id, volatile void _SPM *remote_addr,
          volatile void _SPM *local_addr, size_t buf_size, size_t num_buf){
  mp_ptr->remote_addr = remote_addr;
  mp_ptr->local_addr = local_addr;
  mp_ptr->buf_size = buf_size + 8; // 2 words in bytes for the message complete flag
  mp_ptr->num_buf = num_buf;
  mp_ptr->rcv_count = (volatile void _SPM *)((char*)local_addr + buf_size + 8);
  mp_ptr->rcv_id = rcv_id;
  // Initialize send count to 0 and rcv count to 0.
  mp_ptr->sent_count = 0;
  *(volatile int _SPM *)(mp_ptr->rcv_count) = 0;
  // Initialize last word in buffer to -1.
  *(volatile int _SPM *)((char*)mp_ptr->local_addr + buf_size) = -1;
  return;  
}

void mp_rcv_init(mp_t* mp_ptr, int send_id, volatile void _SPM *remote_addr, volatile void _SPM *local_addr, size_t buf_size, size_t num_buf){
  mp_ptr->remote_addr = remote_addr;
  mp_ptr->local_addr = local_addr;
  mp_ptr->buf_size = buf_size + 8; // 2 words in bytes for the message complete flag
  mp_ptr->num_buf = num_buf;
  mp_ptr->rcv_count = (volatile void _SPM *)((char*)local_addr + (buf_size + 8)*num_buf);
  mp_ptr->send_id = send_id;
  mp_ptr->remote_rcv_count = (volatile void _SPM *)((char*)remote_addr + buf_size + 8);
  // Initialize rcv count to 0.
  *(volatile int _SPM *)(mp_ptr->rcv_count) = 0;
  // Initialize last word in each buffer to 0
  for (int i = 1; i <= num_buf; i++) {
    *(volatile int _SPM *)((char*)mp_ptr->local_addr + buf_size*i) = -1;
  }
  return;
}

////////////////////////////////////////////////////////////////////////////
// Functions for transmitting data
////////////////////////////////////////////////////////////////////////////

void mp_send(mp_t* mp_ptr){
  // Calc addresses based on the status registers
  volatile void _SPM * calc_rmt_addr = &mp_ptr->remote_addr[mp_ptr->buf_size*mp_ptr->sent_count];
  while((mp_ptr->sent_count) - *(mp_ptr->rcv_count) == mp_ptr->num_buf);
  /* spin until there is room in receiving buffer*/
  noc_send(mp_ptr->rcv_id,calc_rmt_addr,mp_ptr->local_addr,mp_ptr->buf_size); // The size is including the last word which is -1
  if (mp_ptr->sent_count == mp_ptr->num_buf-1)
  {
    mp_ptr->sent_count = 0;
  } else {
    mp_ptr->sent_count++;  
  }
  return;
}

void mp_rcv(mp_t* mp_ptr){
  volatile void _SPM * calc_locl_addr = &mp_ptr->local_addr[mp_ptr->buf_size*(*mp_ptr->rcv_count)];
  while(*((volatile int _SPM *)((char*)calc_locl_addr + mp_ptr->buf_size - 8)) != -1){ // Spin until message is received
    /* spin */
  }
  if ((*mp_ptr->rcv_count) == mp_ptr->num_buf-1)
  {
    (*mp_ptr->rcv_count) = 0;
  } else {
    (*mp_ptr->rcv_count)++;  
  }
  *((volatile int _SPM *)((char*)calc_locl_addr + mp_ptr->buf_size - 8)) = 0; // Set the reception flag to 0
  return;
}


void mp_ack(mp_t* mp_ptr){
  noc_send(mp_ptr->send_id,mp_ptr->remote_rcv_count,mp_ptr->rcv_count,8);
  return;
}

