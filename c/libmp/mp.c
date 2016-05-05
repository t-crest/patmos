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
#define DEBUG_ENABLE
#include "include/debug.h"

//#define DEBUG

////////////////////////////////////////////////////////////////////////////
// Functions for library initialization and memory management
////////////////////////////////////////////////////////////////////////////

/// These two arrays are initialized by #mp_init() and only modified by
/// #mp_alloc(), they should not be cached.
static volatile unsigned int * _UNCACHED spm_alloc_array[MAX_CORES];
static volatile unsigned int _UNCACHED spm_size_array[MAX_CORES];

volatile _UNCACHED chan_info_t chan_info[MAX_CHANNELS];

void mp_init() {
  // Get cpu ID
  int cpuid = get_cpuid();

  if (cpuid == 0) {
    for (int i = 0; i < MAX_CHANNELS; ++i) {
      chan_info[i].src_id = -1;
      chan_info[i].sink_id = -1;
      chan_info[i].src_addr = NULL;
      chan_info[i].sink_addr = NULL;
      chan_info[i].src_mpd_ptr = NULL;
      chan_info[i].sink_mpd_ptr = NULL;
      chan_info[i].src_spd_ptr = NULL;
      chan_info[i].sink_spd_ptr = NULL;
    }
  }

  // Find the size of the local communication SPM
  int spm_size = test_spm_size();
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

void _SPM * mp_alloc(const size_t size) {
  // Get cpu ID
  int cpuid = get_cpuid();
  // Align size to double words, this is minimum addressable
  // amount of data from the noc
  size_t w_size = WALIGN(size);
  // Read the new pointer from the array of addresses
  unsigned int mem_ptr = (unsigned int)spm_alloc_array[cpuid];

  unsigned int new_addr = (unsigned int)((char*)mem_ptr + w_size);
  // Check if the allocated memory is there
  if (new_addr < (unsigned int)NOC_SPM_BASE) {
    // TODO: Cause disaster (Kernel panic)
    DEBUGS("SPM Alloc failed. SPM not initialized");
    return NULL;
  }
  if (new_addr > (unsigned int)(spm_size_array[cpuid] + NOC_SPM_BASE)) {
    // TODO: Cause disaster (Kernel panic)
    DEBUGS("SPM Alloc failed. No more memory in SPM");
    return NULL;
  }
  spm_alloc_array[cpuid] = (volatile unsigned int * _UNCACHED)new_addr;
  TRACE(INFO,TRUE,"Core id %u, dw size %lu, allocated addr %x\n",cpuid,w_size,mem_ptr);
  return (void _SPM *)mem_ptr;
}

////////////////////////////////////////////////////////////////////////////
// Functions for initializing the message passing API
////////////////////////////////////////////////////////////////////////////

int mp_init_chans() {
  int retval = 1;
  int cpuid = get_cpuid();
  // For all channels check if port type is sampling or queuing and
  // check if calling core is either source or sink in the channel
  for (int chan_id = 0; chan_id < MAX_CHANNELS; ++chan_id) {
    TRACE(INFO,TRUE,"Initializing channel %d\n",chan_id);

    if(chan_info[chan_id].src_id == cpuid) {
      // If calling core is source, wait for the sink address, and then
      // copy into the source message passing descriptor.
      while (chan_info[chan_id].sink_id == -1);
      TRACE(INFO,TRUE,"Source port found sink_addr : %#08x, src_addr : %#08x\n",
                                    (unsigned int)chan_info[chan_id].sink_addr,
                                    (unsigned int)chan_info[chan_id].src_addr);
      if (chan_info[chan_id].port_type == QUEUING) {
        chan_info[chan_id].src_mpd_ptr->recv_addr = chan_info[chan_id].sink_addr;
      } else if (chan_info[chan_id].port_type == SAMPLING) {
        chan_info[chan_id].src_spd_ptr->lock->remote_ptr = (LOCK_T *)chan_info[chan_id].sink_lock;
        chan_info[chan_id].src_spd_ptr->read_bufs = chan_info[chan_id].sink_addr;
        chan_info[chan_id].src_spd_ptr->remote_spd = chan_info[chan_id].sink_spd_ptr;
        TRACE(INFO,TRUE,"SRC spd ptr: %#08x\n",(int)chan_info[chan_id].src_spd_ptr);
        TRACE(INFO,TRUE,"SINK spd ptr: %#08x\n",(int)chan_info[chan_id].sink_spd_ptr);
      }
      TRACE(INFO,TRUE,"Source port initialized\n");
    } else if (chan_info[chan_id].sink_id == cpuid) {
      // If calling core is sink, wait for the source address, and then
      // copy into the sink message passing descriptor.
      while (chan_info[chan_id].src_id == -1);
      TRACE(INFO,TRUE,"Sink port found src_addr : %#08x, sink_addr : %#08x\n",
                                    (unsigned int)chan_info[chan_id].src_addr,
                                    (unsigned int)chan_info[chan_id].sink_addr);
      if (chan_info[chan_id].port_type == QUEUING) {
        chan_info[chan_id].sink_mpd_ptr->send_recv_count = chan_info[chan_id].src_addr;
      } else if (chan_info[chan_id].port_type == SAMPLING) {
        chan_info[chan_id].sink_spd_ptr->lock->remote_ptr = (LOCK_T *)chan_info[chan_id].src_lock;
        chan_info[chan_id].sink_spd_ptr->read_shm_buf = (volatile void * _SPM)chan_info[chan_id].src_addr;
        chan_info[chan_id].sink_spd_ptr->remote_spd = chan_info[chan_id].src_spd_ptr;
        TRACE(INFO,TRUE,"SINK spd ptr: %#08x\n",(int)chan_info[chan_id].sink_spd_ptr);
        TRACE(INFO,TRUE,"SRC spd ptr: %#08x\n",(int)chan_info[chan_id].src_spd_ptr);
      }
      TRACE(INFO,TRUE,"Sink port initialized\n");
    }

  }

#ifdef DEBUG
  if (get_cpuid() == NOC_MASTER) {
    wait(1000000);
    // TODO: Check that all channels have been initialized
    // and print out which channels that was not initialized
    // within a timeout
    for (int chan_id = 0; chan_id < MAX_CHANNELS; ++chan_id) {
      if(chan_info[chan_id].src_id != -1 && chan_info[chan_id].src_mpd_ptr != NULL) {
        // If the source of the channel has been written and the
        // descriptor address is not NULL, maybe a deadlock happend
        TRACE(FAILURE,TRUE,"The channel %d was not initialized at source\n",chan_id);
      } else if (chan_info[chan_id].sink_id != -1 && chan_info[chan_id].sink_mpd_ptr != NULL) {
        // If the sink of the channel has been written and the
        // descriptor address is not NULL, maybe a deadlock happend
        TRACE(FAILURE,TRUE,"The channel %d was not initialized at sink\n",chan_id);
      }

    }
  }
#endif
  return retval;
}




