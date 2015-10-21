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
//#define TRACE_LEVEL WARNING
//#define DEBUG_ENABLE
#include "include/debug.h"

#define SINGLE_NOC              0
#define SINGLE_SHM              1
#define MULTI_NOC               2
#define MULTI_NOC_NONBLOCKING   3
#define MULTI_NOC_MP            4

#define IMPL MULTI_NOC_MP

spd_t * mp_create_sport(const unsigned int chan_id, const direction_t direction_type,
              const coreid_t remote, const size_t sample_size) {
  if (chan_id >= MAX_CHANNELS || remote >= get_cpucnt()) {
    TRACE(FAILURE,TRUE,"Channel id or remote id is out of range: chan_id %d, remote: %d\n",chan_id,remote);
    return NULL;
  }

  #if IMPL != MULTI_NOC_MP
    
    spd_t * spd_ptr = mp_alloc(sizeof(spd_t));
    if (spd_ptr == NULL) {
      TRACE(FAILURE,TRUE,"Sampling port descriptor could not be allocated, SPM out of memory.\n");
      return NULL;
    }

    spd_ptr->direction_type = direction_type;
    spd_ptr->remote = remote;
    // Align the buffer size to double words and add the flag size
    spd_ptr->sample_size = DWALIGN(sample_size);

    spd_ptr->lock = initialize_lock(remote);
    TRACE(INFO,TRUE,"Initializing lock : %#08x\n",(unsigned int)spd_ptr->lock);

    if (spd_ptr->lock == NULL) {
      TRACE(FAILURE,TRUE,"Lock initialization failed\n");
      return NULL;
    }

    chan_info[chan_id].port_type = SAMPLING;
    if (direction_type == SOURCE) {
      #if IMPL == MULTI_NOC
        spd_ptr->reading = -1;
        spd_ptr->next = 0;
      #elif IMPL == MULTI_NOC_NONBLOCKING
        spd_ptr->reading = 0;
        spd_ptr->next = 0;
      #endif
      // src_desc_ptr must be set first inorder for
      // core 0 to see which cores are absent in debug mode
      chan_info[chan_id].src_spd_ptr = spd_ptr;
      if (chan_info[chan_id].src_spd_ptr == NULL) {
        TRACE(ERROR,TRUE,"src_spd_ptr written incorrectly\n");
        return NULL;
      }
      chan_info[chan_id].src_lock = spd_ptr->lock;

      #if IMPL == SINGLE_SHM
        // For shared memory buffer
        spd_ptr->read_shm_buf = malloc(DWALIGN(sample_size));
        chan_info[chan_id].src_addr = (volatile void _SPM *)spd_ptr->read_shm_buf;
        TRACE(ERROR,chan_info[chan_id].src_addr == NULL,"src_addr written incorrectly\n");
      #endif

      chan_info[chan_id].src_id = (char) get_cpuid();    
      TRACE(INFO,TRUE,"Initialization at sender done.\n");

    } else if (direction_type == SINK) {
      #if IMPL == MULTI_NOC
        spd_ptr->read_bufs = mp_alloc(DWALIGN(sample_size)*3);
        spd_ptr->newest = -1;
      #elif IMPL == MULTI_NOC_NONBLOCKING
        spd_ptr->read_bufs = mp_alloc(DWALIGN(sample_size)*3);
        spd_ptr->newest = -1;
        spd_ptr->next_reading = 0;
      #else
        spd_ptr->read_bufs = mp_alloc(DWALIGN(sample_size));
      #endif
      TRACE(INFO,TRUE,"Initialising SINK port buf_addr: %#08x\n",(unsigned int)spd_ptr->read_bufs);
      // sink_desc_ptr must be set first inorder for
      // core 0 to see which cores are absent in debug mode
      TRACE(INFO,TRUE,"SINK spd ptr: %#08x\n",(unsigned int)spd_ptr);
      chan_info[chan_id].sink_spd_ptr = spd_ptr;
      if (chan_info[chan_id].sink_spd_ptr == NULL) {
        TRACE(ERROR,TRUE,"src_spd_ptr written incorrectly\n");
        return NULL;
      }
      chan_info[chan_id].sink_lock = spd_ptr->lock;
      chan_info[chan_id].sink_addr = (volatile void _SPM *)spd_ptr->read_bufs;
      

      TRACE(ERROR,chan_info[chan_id].sink_addr == NULL,"sink_addr written incorrectly\n");
      if (spd_ptr->read_bufs == NULL) {
        TRACE(FAILURE,TRUE,"SPM allocation failed at SINK\n");
        return NULL;
      }
      chan_info[chan_id].sink_id = (char)get_cpuid();
      TRACE(INFO,TRUE,"Initialization at receiver done.\n");

    }
    return spd_ptr;

  #elif IMPL == MULTI_NOC_MP
    size_t num_buf = 3;
    mpd_t * spd_ptr = mp_create_qport(chan_id,direction_type,remote,sample_size,num_buf);
    if (spd_ptr == NULL) {
      TRACE(FAILURE,TRUE,"Sampling port descriptor could not be allocated, SPM out of memory.\n");
      return NULL;
    }

    return (spd_t *)spd_ptr;
  #endif
}

#if IMPL == SINGLE_SHM

void mp_read_cr(spd_t * sport, volatile void _SPM * sample) {
  // Since sample_size is in bytes and we want to copy 32 bit at the time we divide sample_size by 4
  unsigned itteration_count = (sport->sample_size + 4 - 1) / 4; // equal to ceil(sport->sample_size/4)
  inval_dcache();
  for (int i = 0; i < itteration_count; ++i) {
    ((int _SPM *)sample)[i] = ((volatile int *)sport->read_shm_buf)[i];
  }
}

int mp_read(spd_t * sport, volatile void _SPM * sample) {
  acquire_lock(sport->lock);
  mp_read_cr(sport,sample);
  release_lock(sport->lock);

  return 1;

} 

void mp_write_cr(spd_t * sport, volatile void _SPM * sample) {
  // Since sample_size is in bytes and we want to copy 32 bit at the time we divide sample_size by 4
  unsigned itteration_count = (sport->sample_size + 4 - 1) / 4; // equal to ceil(sport->sample_size/4)
  for (int i = 0; i < itteration_count; ++i) {
    ((volatile int *)sport->read_shm_buf)[i] = ((int _SPM *)sample)[i];
  }
  
}

int mp_write(spd_t * sport, volatile void _SPM * sample) {
  acquire_lock(sport->lock);
  mp_write_cr(sport,sample);
  release_lock(sport->lock);
  return 1;
} 

#elif IMPL == SINGLE_NOC


void mp_read_cr(spd_t * sport, volatile void _SPM * sample) {
  // Since sample_size is in bytes and we want to copy 32 bit at the time we divide sample_size by 4
  unsigned itteration_count = (sport->sample_size + 4 - 1) / 4; // equal to ceil(sport->sample_size/4)
  for (int i = 0; i < itteration_count; ++i) {
    ((int _SPM *)sample)[i] = ((volatile int _SPM *)sport->read_bufs)[i];
  }
}

int mp_read(spd_t * sport, volatile void _SPM * sample) {
  acquire_lock(sport->lock);
  mp_read_cr(sport,sample);
  release_lock(sport->lock);

  return 1;

} 


void mp_write_cr(spd_t * sport, volatile void _SPM * sample) {
  noc_send(sport->remote,sport->read_bufs,sample,sport->sample_size);
  while(!noc_done(sport->remote));
}


int mp_write(spd_t * sport, volatile void _SPM * sample) {
  acquire_lock(sport->lock);
  mp_write_cr(sport,sample);
  release_lock(sport->lock);

  return 1;
} 

#elif IMPL == MULTI_NOC

void mp_read_cr(spd_t * sport, volatile void _SPM * sample, int * newest) {
  // Read newest
  *newest = (int)sport->newest;
  // Update reading
  if (newest >= 0) {
    noc_send( sport->remote,
              (void _SPM *)(((int)&(sport->remote_spd->reading)) ),
              (void _SPM *)&sport->newest,
              sizeof(sport->newest));
    while(!noc_done(sport->remote));
  }
}

int mp_read(spd_t * sport, volatile void _SPM * sample) {
  int newest = 0;
  acquire_lock(sport->lock);
  mp_read_cr(sport, sample, &newest);
  release_lock(sport->lock);
  DEBUGD(newest);

  if (newest < 0) {
    // No sample value has been written yet.
    return 0;
  }
  // Since sample_size is in bytes and we want to copy 32 bit at the time we divide sample_size by 4
  unsigned itteration_count = (sport->sample_size + 4 - 1) / 4; // equal to ceil(sport->sample_size/4)
  for (int i = 0; i < itteration_count; ++i) {
    ((int _SPM *)sample)[i] = ((volatile int _SPM *)(sport->read_bufs+
                              newest*sport->sample_size))[i];
  }

  return 1;

} 

void mp_write_cr(spd_t * sport, volatile void _SPM * sample, unsigned int * reading) {
    // Update newest
  noc_send( sport->remote,
            (void _SPM *)&(sport->remote_spd->newest),
            (void _SPM *)&sport->next,
            sizeof(sport->next));
  while(!noc_done(sport->remote));
  // update next based on the reading variable
  *reading = (unsigned int)sport->reading;
  
}

int mp_write(spd_t * sport, volatile void _SPM * sample) {
  // Send the sample to the next buffer
  noc_send( sport->remote,
            (void _SPM *)( ((unsigned int)sport->read_bufs)+(((unsigned int)sport->next)*sport->sample_size) ),
            sample,
            sport->sample_size);
  while(!noc_done(sport->remote));
  // When the sample is sent take the lock
  unsigned int reading;
  acquire_lock(sport->lock);
  mp_write_cr(sport, sample, &reading);
  release_lock(sport->lock);

  sport->next++;
  if (sport->next >= 3) {
     sport->next = 0;
  }
  if (sport->next == reading) {
    sport->next++;
    if (sport->next >= 3) {
      sport->next = 0;
    }
  }
  

  return 1;
} 

#elif IMPL == MULTI_NOC_NONBLOCKING

int mp_read(spd_t * sport, volatile void _SPM * sample) {
  // Read newest
  int newest = *((volatile int _SPM *)&sport->newest);
  if (newest < 0) {
    // No sample value has been written yet.
    return 0;
  }

  sport->next_reading++;
  if (sport->next_reading >= 3) {
     sport->next_reading = 0;
  }
  unsigned int next_reading = *((volatile unsigned int _SPM *)&sport->next_reading);
  noc_send( sport->remote,
            (void _SPM *)&(sport->remote_spd->reading),
            (void _SPM *)&sport->next_reading,
            sizeof(sport->next_reading));
  while(!noc_done(sport->remote));

  unsigned int next_newest = *((volatile unsigned int _SPM *)&sport->newest);
  while(next_reading != next_newest){
    next_newest = *((volatile unsigned int _SPM *)&sport->newest);
  }
    
  // Since sample_size is in bytes and we want to copy 32 bit at the time we divide sample_size by 4
  unsigned itteration_count = (sport->sample_size + 4 - 1) / 4; // equal to ceil(sport->sample_size/4)
  for (int i = 0; i < itteration_count; ++i) {
    ((int _SPM *)sample)[i] = ((volatile int _SPM *)(sport->read_bufs+
                              newest*sport->sample_size))[i];
  }

  return 1;

} 

int mp_write(spd_t * sport, volatile void _SPM * sample) {
  // Send the sample to the next buffer
  unsigned int reading = *((volatile unsigned int _SPM *)&sport->reading);
  noc_send( sport->remote,
            (void _SPM *)( ((unsigned int)sport->read_bufs)+(reading*sport->sample_size) ),
            sample,
            sport->sample_size);
  sport->next = reading;
  while(!noc_done(sport->remote));

  // Update newest
  noc_send( sport->remote,
            (void _SPM *)&(sport->remote_spd->newest),
            (void _SPM *)&sport->next,
            sizeof(sport->next));
  while(!noc_done(sport->remote));

  return 1;
} 

#elif IMPL == MULTI_NOC_MP

int mp_read(spd_t * sport, volatile void _SPM * sample) {
  int ret = 0;
  int msg_rev = 0;
  for (int i = 0; i < ((mpd_t *)sport)->num_buf; ++i) {
    if(mp_nbrecv((mpd_t *)sport) != 0){
      msg_rev++;
    }
  }
  if (msg_rev == 0) {
    return 0;
  }

  for (int i = 0; i < msg_rev-1; ++i) {
    mp_ack((mpd_t *)sport,0);
  }
  // Since sample_size is in bytes and we want to copy 32 bit at the time we divide sample_size by 4
  unsigned itteration_count = (((mpd_t *)sport)->buf_size + 4 - 1) / 4; // equal to ceil(sport->sample_size/4)
  for (int i = 0; i < itteration_count; ++i) {
    ((volatile int _SPM *)sample)[i] = ((volatile int _SPM *)(((mpd_t *)sport)->read_buf))[i];
  }
  mp_ack((mpd_t *)sport,0);

  return 1;

} 

int mp_write(spd_t * sport, volatile void _SPM * sample) {
  // Since sample_size is in bytes and we want to copy 32 bit at the time we divide sample_size by 4
  unsigned itteration_count = (((mpd_t *)sport)->buf_size + 4 - 1) / 4; // equal to ceil(sport->sample_size/4)
  for (int i = 0; i < itteration_count; ++i) {
    ((volatile int _SPM *)(((mpd_t *)sport)->write_buf))[i]= ((volatile int _SPM *)sample)[i];
  }
  mp_send((mpd_t *)sport,10000);
  return 1;
} 

#endif