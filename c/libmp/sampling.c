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

#include "mp.h"
#include "mp_internal.h"
////////////////////////////////////////////////////////////////////////////
// Functions for sampling point-to-point transmission of data
////////////////////////////////////////////////////////////////////////////

void mp_write(mpd_t* mpd_ptr) {
  // Sampling of the write pointer to get rid of transient hazards.
  // We sample the write poiniter twice, if the two samples are
  // different, the write_pointer just changed, and therefore it is
  // safe to sample it once more and rely on the newest sample,
  // because it cannot change that often
  int write_ptr = *(mpd_ptr->write_ptr);
  int write_ptr_tmp = *(mpd_ptr->write_ptr);
  if (write_ptr != write_ptr_tmp) {
    write_ptr = *(mpd_ptr->write_ptr);
  }
  // Calculate the address of the remote receiving buffer
  int rmt_addr_offset = (mpd_ptr->buf_size + FLAG_SIZE) * write_ptr;
  volatile void _SPM * calc_rmt_addr = &mpd_ptr->recv_addr[rmt_addr_offset];
  *(volatile int _SPM *)((char*)mpd_ptr->write_buf + mpd_ptr->buf_size) = FLAG_VALID;

  // Send the new sample to the remote receiving buffer
  noc_send(mpd_ptr->recv_id,calc_rmt_addr,mpd_ptr->write_buf,mpd_ptr->buf_size + FLAG_SIZE);

  // Swap write_buf and shadow_write_buf
  volatile void _SPM * tmp = mpd_ptr->write_buf;
  mpd_ptr->write_buf = mpd_ptr->shadow_write_buf;
  mpd_ptr->shadow_write_buf = tmp;

  return;
}

void mp_read(mpd_t* mpd_ptr) {

  // Update the write pointer to point to the other buffer.
  unsigned int read_buf = mpd_ptr->write_ptr;
  mpd_ptr->write_ptr = (read_buf + 1) % NUM_WRITE_BUF;
  // Send the update write pointer to the sender.
  noc_send(mpd_ptr->send_id,mpd_ptr->,mpd_ptr->write_ptr,sizeof(mpd_ptr->write_ptr));
  // Wait fo the write pointer to propergate to the sender
  // and a possible sample to propergate from the sender
  // to the receiver
  // Calculate the address of the local receiving buffer
  int locl_addr_offset = (mpd_ptr->buf_size + FLAG_SIZE) * mpd_ptr->recv_ptr;
  volatile void _SPM * calc_locl_addr = &mpd_ptr->recv_addr[locl_addr_offset];

  volatile int _SPM * recv_flag = (volatile int _SPM *)((char*)calc_locl_addr + mpd_ptr->buf_size);

  if (*recv_flag == FLAG_INVALID) {
    DEBUGGER("mp_nbrecv(): Recv flag %x\n",*recv_flag);
    return 0;
  }

  // Move the receive pointer
  if (mpd_ptr->recv_ptr == mpd_ptr->num_buf - 1) {
    mpd_ptr->recv_ptr = 0;
  } else {
    mpd_ptr->recv_ptr++;
  }

  // Set the reception flag of the received message to FLAG_INVALID
  *recv_flag = FLAG_INVALID;

  // Set the new read buffer pointer
  mpd_ptr->read_buf = calc_locl_addr;

  return 1;
}

int mp_read_updated(mpd_t* mpd_ptr) {
  
}
