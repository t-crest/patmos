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

#include <stdio.h>

////////////////////////////////////////////////////////////////////////////
// Functions for library initialization and memory management
////////////////////////////////////////////////////////////////////////////

void mp_init() {
  // Initializing the array of pointers to the beginning of the SPMs
  for (int i = 0; i < MAX_CORES; ++i) {
    spm_alloc_array[i] = (volatile char* _UNCACHED) NOC_SPM_BASE;
  }
  int _SPM * spm_zero = (int _SPM *) mp_alloc(NOC_MASTER,sizeof(int _SPM *));
  *spm_zero = 0;
  return;
}

size_t mp_send_alloc_size(mpd_t* mpd_ptr) {
  size_t send_size = (mpd_ptr->buf_size + FLAG_SIZE) * NUM_WRITE_BUF
                                  + DWALIGN(sizeof(*(mpd_ptr->send_recv_count)));
  return send_size;
}

size_t mp_recv_alloc_size(mpd_t* mpd_ptr) {
  size_t recv_size = (mpd_ptr->buf_size + FLAG_SIZE) * mpd_ptr->num_buf
                                  + DWALIGN(sizeof(*(mpd_ptr->recv_count)));
  return recv_size;
}

void _SPM * mp_alloc(coreid_t id, unsigned size) {
  if (get_cpuid() != NOC_MASTER) {
    return NULL;
  }
  unsigned dw_size = DWALIGN(size);
  void _SPM * mem_ptr = (void _SPM *) spm_alloc_array[id];
  spm_alloc_array[id] = spm_alloc_array[id] + dw_size;
  DEBUGGER("mp_alloc(): core id %u, dw size %u, allocated addr %x\n",id,dw_size,(int)mem_ptr);
  // TODO: Check if SPM size is larger than new pointer value
  return mem_ptr;
}


////////////////////////////////////////////////////////////////////////////
// Functions for initializing the message passing API
////////////////////////////////////////////////////////////////////////////

int mp_chan_init(mpd_t* mpd_ptr, coreid_t sender, coreid_t receiver,
          unsigned buf_size, unsigned num_buf) {

  if (get_cpuid() != NOC_MASTER) {
    DEBUGGER("mp_chan_init(): called by non-master");
    return 0;
  }
  
  /* COMMON INITIALIZATION */

  // Align the buffer size to double words and add the flag size
  mpd_ptr->buf_size = DWALIGN(buf_size);
  mpd_ptr->num_buf = num_buf;
  
  mpd_ptr->recv_id = receiver;
  mpd_ptr->send_id = sender;

  mpd_ptr->send_addr = mp_alloc(sender,mp_send_alloc_size(mpd_ptr));
  mpd_ptr->recv_addr = mp_alloc(receiver,mp_recv_alloc_size(mpd_ptr));

  if (mpd_ptr->send_addr == NULL || mpd_ptr->recv_addr == NULL) {
    DEBUGGER("mp_chan_init(): SPM allocation failed");
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
  DEBUGGER("mp_chan_init(): Initialization at sender done.\n");
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

  DEBUGGER("mp_chan_init(): Initialization at receiver done.\n");

  return 1;
}

int mp_communicator_init(communicator_t* comm, unsigned count,
              const coreid_t member_ids [], unsigned msg_size) {
  if (get_cpuid() != NOC_MASTER) {
    return 0;
  }
  comm->count = count;
  comm->msg_size = msg_size;
  comm->addr = (volatile void _SPM **) malloc(sizeof(void*)*count);

  DEBUGGER("mp_communicator_init(): malloc size %lu\n",sizeof(void*)*count);
  
  for (int i = 0; i < count; ++i) {
    coreset_add(member_ids[i],&comm->barrier_set);
    comm->addr[i] = (volatile void _SPM *) mp_alloc(member_ids[i],count*BARRIER_SIZE+msg_size+FLAG_SIZE);


    if (get_cpuid() == member_ids[i]) {
      for (int j = 0; j < count; ++j) {
        *((volatile int _SPM *)((unsigned)comm->addr[i] + (j*BARRIER_SIZE))) = BARRIER_INITIALIZED;
      }
      *((volatile int _SPM *)((unsigned)comm->addr[i] + (count*BARRIER_SIZE) + msg_size)) = BARRIER_INITIALIZED;
    } else {      
      for (int j = 0; j < count; ++j) {
        noc_send(member_ids[i],
                (volatile int _SPM *)((unsigned)comm->addr[i] + (j*BARRIER_SIZE)),
                NOC_SPM_BASE,
                4);
      }
      noc_send(member_ids[i],
              (volatile int _SPM *)((unsigned)comm->addr[i] + (count*BARRIER_SIZE) + msg_size),
              NOC_SPM_BASE,
              4);
    }  
  }
  
  return 1;
}

////////////////////////////////////////////////////////////////////////////
// Functions for transmitting data
////////////////////////////////////////////////////////////////////////////

int mp_nbsend(mpd_t* mpd_ptr) {

  // Calculate the address of the remote receiving buffer
  int rmt_addr_offset = (mpd_ptr->buf_size + FLAG_SIZE) * mpd_ptr->send_ptr;
  volatile void _SPM * calc_rmt_addr = &mpd_ptr->recv_addr[rmt_addr_offset];
  *(volatile int _SPM *)((char*)mpd_ptr->write_buf + mpd_ptr->buf_size) = FLAG_VALID;

  if ((mpd_ptr->send_count) - *(mpd_ptr->send_recv_count) == mpd_ptr->num_buf) {
    DEBUGGER("mp_nbsend(): NO room in queue\n");
    return 0;
  }
  if (!noc_nbsend(mpd_ptr->recv_id,calc_rmt_addr,mpd_ptr->write_buf,mpd_ptr->buf_size + FLAG_SIZE)) {
    DEBUGGER("mp_nbsend(): NO DMA free\n");
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

void mp_send(mpd_t* mpd_ptr) {
  while(!mp_nbsend(mpd_ptr));
}

int mp_nbrecv(mpd_t* mpd_ptr) {

  // Calculate the address of the local receiving buffer
  int locl_addr_offset = (mpd_ptr->buf_size + FLAG_SIZE) * mpd_ptr->recv_ptr;
  volatile void _SPM * calc_locl_addr = &mpd_ptr->recv_addr[locl_addr_offset];

  volatile int _SPM * recv_flag = (volatile int _SPM *)((char*)calc_locl_addr + mpd_ptr->buf_size);

  if (*recv_flag == FLAG_INVALID) {
    DEBUGGER("mp_nbrecv(): Recv flag %x\n",*recv_flag);
    return 0;
  }

  // Increment the receive counter
  (*mpd_ptr->recv_count)++;

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

void mp_recv(mpd_t* mpd_ptr) {
  while(!mp_nbrecv(mpd_ptr));
}


int mp_nback(mpd_t* mpd_ptr){
  // Update the remote receive count
  return noc_nbsend(mpd_ptr->send_id,mpd_ptr->send_recv_count,mpd_ptr->recv_count,8);
}

void mp_ack(mpd_t* mpd_ptr){
  // Update the remote receive count
  while(!mp_nback(mpd_ptr));
  return;
}


void mp_barrier(communicator_t* comm){
  DEBUG_CORECHECK(!coreset_contains(get_cpuid(),&comm->barrier_set));
  // Something bad happens if mp_barrier() is called by a core
  // that is not in the communicator.
  unsigned index = 0;
  for (unsigned i = 0; i < CORESET_SIZE; ++i) {
    if(coreset_contains(i,&comm->barrier_set) && i < get_cpuid()) {
      index++;
    }
  }
  DEBUGGER("mp_barrier(): index %d\n",index);
  volatile int _SPM * addr = (volatile int _SPM *)((unsigned)comm->addr[index] +
                               index*BARRIER_SIZE);

  *addr = BARRIER_REACHED;
  noc_multisend_cs(comm->barrier_set,comm->addr,index*BARRIER_SIZE,addr,BARRIER_SIZE);
  int done;
  coreset_t recv;
  coreset_clearall(&recv);
  do {
    done = 1;
    unsigned cpu_index = 0;
    for (unsigned cpu_id = 0; cpu_id < CORESET_SIZE; ++cpu_id) {
      if (coreset_contains(cpu_id,&comm->barrier_set) &&
          !coreset_contains(cpu_id,&recv)) {
        DEBUGGER("mp_barrier(): looking at cpuid %u at address %x\n",cpu_id,(unsigned)comm->addr[index] + cpu_index*BARRIER_SIZE);
        if (*(volatile int _SPM *)((unsigned)comm->addr[index] + cpu_index*BARRIER_SIZE)
              == BARRIER_REACHED) {
          DEBUGGER("mp_barrier(): cpuid %u reached barrier\n",cpu_id);
          coreset_add(cpu_id,&recv);
        } else {
          done = 0;
        }
        cpu_index++;
      }
    }
  } while(!done);
  for (int i = 0; i < comm->count; ++i) {
    *((volatile int _SPM *)((unsigned)comm->addr[index] + (i*BARRIER_SIZE))) = BARRIER_INITIALIZED;
  }
  return;
}


void mp_broadcast(communicator_t* comm, coreid_t root) {
  if (comm->msg_size == 0) {
    DEBUGGER("mp_broadcast(): msg_size is 0;");
    return;
  }
  mp_barrier(comm);
  if (get_cpuid() == root) {
    noc_multisend_cs(comm->barrier_set,
                     comm->addr,
                     comm->count*BARRIER_SIZE,
                     (volatile void _SPM*)((unsigned)comm->addr[root]+comm->count*BARRIER_SIZE),
                     comm->msg_size);
  } else {
    volatile int _SPM * flag_addr = (volatile int _SPM*)(
                                    (unsigned)comm->addr[get_cpuid()]
                                             +comm->count*BARRIER_SIZE
                                             +comm->msg_size);
    while((*flag_addr) != BARRIER_REACHED) {
      /* Spin */
    }
    *flag_addr = BARRIER_INITIALIZED;    
  }
  mp_barrier(comm);

  return;
}