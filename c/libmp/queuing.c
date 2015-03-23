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
 #define TRACE_LEVEL INFO
 #define DEBUG_ENABLE
 #include "include/debug.h"

////////////////////////////////////////////////////////////////////////////
// Function for creating a queuing port 
////////////////////////////////////////////////////////////////////////////
mpd_t _SPM * mp_create_qport(const unsigned int chan_id, const port_t port_type,
              const coreid_t remote, const size_t msg_size, const size_t num_buf) {
  if (chan_id >= MAX_CHANNELS || remote >= get_cpucnt()) {
    TRACE(FAILURE,TRUE,"Channel id or remote id is out of range: chan_id %d, remote: %d",chan_id,remote);
    return NULL;
  }

  // Allocate descriptor in SPM
  mpd_t _SPM * mpd_ptr = mp_alloc(sizeof(mpd_t));
  if (mpd_ptr == NULL) {
    TRACE(FAILURE,TRUE,"Message passing descriptor could not be allocated, SPM out of memory.");
    return NULL;
  }

  mpd_ptr->port_type = port_type;
  mpd_ptr->remote = remote;
  // Align the buffer size to double words and add the flag size
  mpd_ptr->buf_size = DWALIGN(msg_size);
  mpd_ptr->num_buf = num_buf;

  if (port_type == SOURCE) {

    unsigned int _SPM * send_addr = (unsigned int _SPM *)mp_alloc(mp_send_alloc_size(mpd_ptr));
    
    if (send_addr == NULL) {
      TRACE(FAILURE,TRUE,"SPM allocation failed at SOURCE");
      return NULL;
    }

    int send_recv_count_offset = (mpd_ptr->buf_size + FLAG_SIZE) * NUM_WRITE_BUF;
    mpd_ptr->send_recv_count = (volatile unsigned int _SPM *)((char*)send_addr + send_recv_count_offset);

    // src_desc_ptr must be set first inorder for
    // core 0 to see which cores are absent in debug mode
    chan_info[chan_id].src_desc_ptr = mpd_ptr;
    chan_info[chan_id].src_id = (char) get_cpuid();
    // Write the send_recv_count address to the main memory for coordination
    chan_info[chan_id].src_addr = (volatile void _SPM * )mpd_ptr->send_recv_count;

    // Initializing sender_recv_count
    *(mpd_ptr->send_recv_count) = 0;
    
    TRACE(INFO,TRUE,"Initialization at sender done.\n");
    // Initialize send count to 0 and recv count to 0.
    mpd_ptr->send_count = 0;
    mpd_ptr->send_ptr = 0;
    
    mpd_ptr->write_buf = (volatile void _SPM *)send_addr;
    mpd_ptr->shadow_write_buf = (volatile void _SPM *)((char*)send_addr + (mpd_ptr->buf_size + FLAG_SIZE));


  } else if (port_type == SINK) {

    mpd_ptr->recv_addr = mp_alloc(mp_recv_alloc_size(mpd_ptr));
    // sink_desc_ptr must be set first inorder for
    // core 0 to see which cores are absent in debug mode
    chan_info[chan_id].sink_desc_ptr = mpd_ptr;
    chan_info[chan_id].sink_id = (char)get_cpuid();
    chan_info[chan_id].sink_addr = (volatile void _SPM *)mpd_ptr->recv_addr;

    if (mpd_ptr->recv_addr == NULL) {
      TRACE(FAILURE,TRUE,"SPM allocation failed at SINK");
      return NULL;
    }

    mpd_ptr->read_buf = mpd_ptr->recv_addr;
    mpd_ptr->recv_ptr = 0;

    int recv_count_offset = (mpd_ptr->buf_size + FLAG_SIZE) * num_buf;
    mpd_ptr->recv_count = (volatile unsigned _SPM *)((char*)mpd_ptr->recv_addr + recv_count_offset);
    
    // Initializing recv_count
    *(mpd_ptr->recv_count) = 0;
        
    // Initialize last word in each buffer to FLAG_INVALID
    for (int i = 0; i < mpd_ptr->num_buf; i++) {
      // Calculate the address of the local receiving buffer
      int locl_addr_offset = (mpd_ptr->buf_size + FLAG_SIZE) * i;
      volatile void _SPM * calc_locl_addr = &mpd_ptr->recv_addr[locl_addr_offset];

      volatile int _SPM * flag_addr = (volatile int _SPM *)((char*)calc_locl_addr + mpd_ptr->buf_size);
      *(flag_addr) = 0;
      
    }

    TRACE(INFO,TRUE,"Initialization at receiver done.\n");


  }

  // Return the created queuing port
  return mpd_ptr;
}


////////////////////////////////////////////////////////////////////////////
// Functions for point-to-point transmission of data
////////////////////////////////////////////////////////////////////////////

int mp_nbsend(mpd_t _SPM * mpd_ptr) {

  // Calculate the address of the remote receiving buffer
  int rmt_addr_offset = (mpd_ptr->buf_size + FLAG_SIZE) * mpd_ptr->send_ptr;
  volatile void _SPM * calc_rmt_addr = &mpd_ptr->recv_addr[rmt_addr_offset];
  *(volatile int _SPM *)((char*)mpd_ptr->write_buf + mpd_ptr->buf_size) = FLAG_VALID;

  if ((mpd_ptr->send_count) - *(mpd_ptr->send_recv_count) == mpd_ptr->num_buf) {
    TRACE(INFO,TRUE,"NO room in queue\n");
    return 0;
  }
  if (!noc_nbsend(mpd_ptr->remote,calc_rmt_addr,mpd_ptr->write_buf,mpd_ptr->buf_size + FLAG_SIZE)) {
    TRACE(INFO,TRUE,"NO DMA free\n");
    return 0;
  }

  // Increment the send counter
  mpd_ptr->send_count++;

  // Move the send pointer
  if (mpd_ptr->send_ptr == mpd_ptr->num_buf-1) {
    mpd_ptr->send_ptr = 0;
  } else {
    mpd_ptr->send_ptr++;  
  }

  // Swap write_buf and shadow_write_buf
  volatile void _SPM * tmp = mpd_ptr->write_buf;
  mpd_ptr->write_buf = mpd_ptr->shadow_write_buf;
  mpd_ptr->shadow_write_buf = tmp;

  return 1;
}

int mp_send(mpd_t _SPM * mpd_ptr, const unsigned int time_usecs) {
  unsigned long long int timeout = get_cpu_usecs() + time_usecs;
  int retval = 0;
  _Pragma("loopbound min 1 max 1")
  // while message not sent and ( timeout infinite or now is before timeout)
  while(retval == 0 && ( time_usecs == 0 || get_cpu_usecs() < timeout ) ) {
    retval = mp_nbsend(mpd_ptr);
  }
  return retval;
}

int mp_nbrecv(mpd_t _SPM * mpd_ptr) {

  // Calculate the address of the local receiving buffer
  int locl_addr_offset = (mpd_ptr->buf_size + FLAG_SIZE) * mpd_ptr->recv_ptr;
  volatile void _SPM * calc_locl_addr = &mpd_ptr->recv_addr[locl_addr_offset];

  volatile int _SPM * recv_flag = (volatile int _SPM *)((char*)calc_locl_addr + mpd_ptr->buf_size);

  if (*recv_flag == FLAG_INVALID) {
    TRACE(INFO,TRUE,"Recv flag %x\n",*recv_flag);
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

int mp_recv(mpd_t _SPM * mpd_ptr, const unsigned int time_usecs) {
  unsigned long long int timeout = get_cpu_usecs() + time_usecs;
  int retval = 0;
  _Pragma("loopbound min 1 max 1")
  // while message not received and ( timeout infinite or now is before timeout)
  while(retval == 0 && ( time_usecs == 0 || get_cpu_usecs() < timeout ) ) {
    retval = mp_nbrecv(mpd_ptr);
  }
  return retval;
}

int mp_nback(mpd_t _SPM * mpd_ptr){
  // Check previous acknowledgement transfer before updating counter in SPM
  if (!noc_done(mpd_ptr->remote)) { return 0; }
  // Increment the receive counter
  (*mpd_ptr->recv_count)++;
  // Update the remote receive count
  int success = noc_nbsend(mpd_ptr->remote,mpd_ptr->send_recv_count,mpd_ptr->recv_count,8);
  if (!success) {
    (*mpd_ptr->recv_count)--;
  }
  return success;
}

int mp_ack(mpd_t _SPM * mpd_ptr, const unsigned int time_usecs){
  unsigned long long int timeout = get_cpu_usecs() + time_usecs;
  int retval = 0;
  // Await previous acknowledgement transfer before updating counter in SPM
  _Pragma("loopbound min 1 max 1")
  // while DMA is not free and ( timeout infinite or now is before timeout)
  while(retval == 0 && ( time_usecs == 0 || get_cpu_usecs() < timeout ) ) {
    retval = noc_done(mpd_ptr->remote);
  }
  if (retval == 0) {
    // Return if timed out
    return retval;
  } else {
    // Reset the return val to reuse in next while loop
    retval = 0;
  }
  // Increment the receive counter
  (*mpd_ptr->recv_count)++;
  // Update the remote receive count
  _Pragma("loopbound min 1 max 1")
  // while message not sent and ( timeout infinite or now is before timeout)
  while(retval == 0 && ( time_usecs == 0 || get_cpu_usecs() < timeout ) ) {
    retval = noc_nbsend(mpd_ptr->remote,mpd_ptr->send_recv_count,
                        mpd_ptr->recv_count,8);
  }
  if (retval == 0) {
    (*mpd_ptr->recv_count)--;
  }
  return retval;
}
