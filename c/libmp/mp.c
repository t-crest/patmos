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
#include "include/debug.h"

////////////////////////////////////////////////////////////////////////////
// Functions for library initialization and memory management
////////////////////////////////////////////////////////////////////////////

/// These two arrays are initialized by #mp_init() and only modified by
/// #mp_alloc(), they should not be cached.
static volatile unsigned int * _UNCACHED spm_alloc_array[MAX_CORES];
static volatile unsigned int _UNCACHED spm_size_array[MAX_CORES];

#define MAX_CHANNELS  32
#define SOURCE_SINK   2
#define SOURCE        0
#define SINK          1
static volatile void * _UNCACHED chan_ptr_array[SOURCE_SINK][MAX_CHANNELS];

static int test_mem_size_spm(){
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
      // We found the address where the mapping of the SPM wrapps
      // Restore the state of the memory as is was when the function was called
      *(addr+i) = tmp;
      *(addr) = init;
      // Remember to multiply by 4 for the byte address
      return i << 2;
    }
    i = i << 1;
    if (*(addr) != 0xFFEEDDCC){
      // Memory failure happend
      *(addr+i) = tmp;
      *(addr) = init;
      return -2;
    }
    *(addr+i) = tmp;
  }
  *(addr) = init;
  return -1;
}
  

void mp_init() {
  // Get cpu ID
  int cpuid = get_cpuid();

  // Find the size of the local communication SPM
  int spm_size = test_mem_size_spm();
  // Check the return value of the test
  if(spm_size == -1) {
    // TODO: Cause disaster
    // test_mem_size_spm() did not find the size of the spm
    DEBUGS("Did not find the size of the memory");
    abort();
  } else if (spm_size == -2) {
    // TODO: Cause disaster
    // while executing test_mem_size_spm() a memory failure happend
    DEBUGS("Memory failure found");
    abort();
  }
  // Initialize the size of the SPM
  spm_size_array[cpuid] = spm_size;

  // Initializing the array of pointers to the beginning of the SPMs
  spm_alloc_array[cpuid] = (volatile unsigned int * _UNCACHED) NOC_SPM_BASE;

  // Allocate a zero value for remote resetting of values through the NOC
  barrier_t _SPM * spm_zero = (barrier_t _SPM *) mp_alloc(BARRIER_SIZE);
  *spm_zero = BARRIER_INITIALIZED;
  return;
}

static size_t mp_send_alloc_size(mpd_t* mpd_ptr) {
  size_t send_size = (mpd_ptr->buf_size + FLAG_SIZE) * NUM_WRITE_BUF
                                  + DWALIGN(sizeof(*(mpd_ptr->send_recv_count)));
  return send_size;
}

static size_t mp_recv_alloc_size(mpd_t* mpd_ptr) {
  size_t recv_size = (mpd_ptr->buf_size + FLAG_SIZE) * mpd_ptr->num_buf
                                  + DWALIGN(sizeof(*(mpd_ptr->recv_count)));
  return recv_size;
}

void _SPM * mp_alloc(size_t size) {
  // Get cpu ID
  int cpuid = get_cpuid();
  // Align size to double words, this is minimum addressable
  // amount of data from the noc
  size_t dw_size = DWALIGN(size);
  // Read the new pointer from the array of addresses
  unsigned int mem_ptr = (unsigned int)spm_alloc_array[cpuid];

  unsigned int new_addr = mem_ptr + dw_size;
  // Check if the allocated memory is there
  if (new_addr > (unsigned int)(spm_size_array[cpuid] + NOC_SPM_BASE)) {
    // TODO: Cause disaster (Kernel panic)
    DEBUGS("SPM Alloc failed. No more memory in SPM");
    return NULL;
  }
  spm_alloc_array[cpuid] = (volatile unsigned int * _UNCACHED)new_addr;
  TRACE(INFO,TRUE,"Core id %u, dw size %u, allocated addr %x\n",cpuid,dw_size,mem_ptr);
  return (void _SPM *)mem_ptr;
}



////////////////////////////////////////////////////////////////////////////
// Functions for initializing the message passing API
////////////////////////////////////////////////////////////////////////////

int mp_chan_init(mpd_t* mpd_ptr, coreid_t sender, coreid_t receiver,
          unsigned buf_size, unsigned num_buf) {

  if (get_cpuid() != NOC_MASTER) {
    TRACE(INFO,TRUE,"called by non-master");
    return 0;
  }
  
  /* COMMON INITIALIZATION */

  // Align the buffer size to double words and add the flag size
  mpd_ptr->buf_size = DWALIGN(buf_size);
  mpd_ptr->num_buf = num_buf;
  
  mpd_ptr->recv_id = receiver;
  mpd_ptr->send_id = sender;

  mpd_ptr->send_addr = mp_alloc(mp_send_alloc_size(mpd_ptr)); // sender,
  mpd_ptr->recv_addr = mp_alloc(mp_recv_alloc_size(mpd_ptr)); // receiver,

  if (mpd_ptr->send_addr == NULL || mpd_ptr->recv_addr == NULL) {
    TRACE(INFO,TRUE,"SPM allocation failed");
    return 0;
  }


  /* SENDER INITIALIZATION */


  int send_recv_count_offset = (mpd_ptr->buf_size + FLAG_SIZE) * NUM_WRITE_BUF;
  mpd_ptr->send_recv_count = (volatile unsigned _SPM *)((char*)mpd_ptr->send_addr + send_recv_count_offset);
  // TODO: sender_recv_count must be initialized through the NoC
  if (get_cpuid() == mpd_ptr->send_id) {
    *(mpd_ptr->send_recv_count) = 0;
  } else {
    noc_send(mpd_ptr->send_id,mpd_ptr->send_recv_count,NOC_SPM_BASE,4);
  }
  TRACE(INFO,TRUE,"Initialization at sender done.\n");
  // Initialize send count to 0 and recv count to 0.
  mpd_ptr->send_count = 0;
  mpd_ptr->send_ptr = 0;
  
  mpd_ptr->write_buf = mpd_ptr->send_addr;
  mpd_ptr->shadow_write_buf = (volatile void _SPM *)((char*)mpd_ptr->send_addr + (mpd_ptr->buf_size + FLAG_SIZE));


  /* RECEIVER INITIALIZATION */

  mpd_ptr->read_buf = mpd_ptr->recv_addr;
  mpd_ptr->recv_ptr = 0;

  int recv_count_offset = (mpd_ptr->buf_size + FLAG_SIZE) * num_buf;
  mpd_ptr->recv_count = (volatile unsigned _SPM *)((char*)mpd_ptr->recv_addr + recv_count_offset);
  
  // TODO: must be initialized through the NoC
  if (get_cpuid() == mpd_ptr->recv_id) {
    *(mpd_ptr->recv_count) = 0;
  } else {
    noc_send(mpd_ptr->recv_id,mpd_ptr->recv_count,NOC_SPM_BASE,4);
  }
  
  
  
  // Initialize last word in each buffer to FLAG_INVALID
  for (int i = 0; i < mpd_ptr->num_buf; i++) {
    // TODO: calculate address and do write with a noc_send()
    // Calculate the address of the local receiving buffer
    int locl_addr_offset = (mpd_ptr->buf_size + FLAG_SIZE) * i;
    volatile void _SPM * calc_locl_addr = &mpd_ptr->recv_addr[locl_addr_offset];

    volatile int _SPM * flag_addr = (volatile int _SPM *)((char*)calc_locl_addr + mpd_ptr->buf_size);

    if (get_cpuid() == mpd_ptr->recv_id) {
      *(flag_addr) = 0;
    } else {
      noc_send(mpd_ptr->recv_id,flag_addr,NOC_SPM_BASE,4);
    }
    
  }

  TRACE(INFO,TRUE,"Initialization at receiver done.\n");

  return 1;
}

int mp_init_chans() {
  int retval = 1;
#ifdef DEBUG
  if (get_cpuid() == NOC_MASTER) {
    // TODO: Check that all channels have been initialized
    // and print out which channels that was not initialized
    // with in a timeout
  }
#endif
  return retval;
}



