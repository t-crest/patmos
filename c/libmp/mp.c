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

/// These two arrays are initialized by #mp_init() and only modified by
/// #mp_alloc(), they should not be cached.
static volatile unsigned int * _UNCACHED spm_alloc_array[MAX_CORES];
static volatile unsigned int _UNCACHED spm_size_array[MAX_CORES];

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
    DEBUGGER("test_mem_size_spm(): Did not find the size of the memory");
    return;
  } else if (spm_size == -2) {
    // TODO: Cause disaster
    // while executing test_mem_size_spm() a memory failure happend
    DEBUGGER("test_mem_size_spm(): Memory failure found");
    return;
  }
  // Initialize the size of the SPM
  spm_size_array[cpuid] = spm_size;

  // Initializing the array of pointers to the beginning of the SPMs
  spm_alloc_array[cpuid] = (volatile unsigned* _UNCACHED) NOC_SPM_BASE;

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

//void _SPM * mp_alloc(coreid_t id, unsigned size) {
//  if (get_cpuid() != NOC_MASTER) {
//    return NULL;
//  }
//  unsigned dw_size = DWALIGN(size);
//  void _SPM * mem_ptr = (void _SPM *) spm_alloc_array[id];
//  unsigned int new_addr = spm_alloc_array[id] + dw_size;
//  if (new_addr > spm_size_array[get_cpuid()] + NOC_SPM_BASE) {
//    // TODO: Cause disaster (Kernel panic)
//    return NULL;
//  }
//  spm_alloc_array[id] = new_addr;
//  DEBUGGER("mp_alloc(): core id %u, dw size %u, allocated addr %x\n",id,dw_size,(int)mem_ptr);
//  return mem_ptr;
//}

void _SPM * mp_alloc(size_t size) {
  // Get cpu ID
  int cpuid = get_cpuid();
  // Align size to double words, this is minimum addressable
  // amount of data from the noc
  size_t dw_size = DWALIGN(size);
  // Read the new pointer from the array of addresses
  unsigned int mem_ptr = spm_alloc_array[cpuid];

  unsigned int new_addr = mem_ptr + dw_size;
  // Check if the allocated memory is there
  if (new_addr > spm_size_array[cpuid] + NOC_SPM_BASE) {
    // TODO: Cause disaster (Kernel panic)
    DEBUGGER("mp_alloc(): Alloc failed. No more memory in SPM");
    return NULL;
  }
  spm_alloc_array[cpuid] = new_addr;
  DEBUGGER("mp_alloc(): core id %u, dw size %u, allocated addr %x\n",cpuid,dw_size,(int)mem_ptr);
  return (void _SPM *)mem_ptr;
}

void mp_mem_init(coreid_t id, void _SPM* spm_addr) {
  int cpuid = get_cpuid();
  if (cpuid != NOC_MASTER) {
    return;
  }
  if (cpuid == id) {
    *((int _SPM*)spm_addr) = 0;
  } else {
    noc_send(id,spm_addr,NOC_SPM_BASE,8);
  }
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
  
  coreset_clearall(&comm->barrier_set);
  for (int i = 0; i < count; ++i) {
    coreset_add(member_ids[i],&comm->barrier_set);
    comm->addr[i] = (volatile void _SPM *) mp_alloc(member_ids[i],count*BARRIER_SIZE+msg_size+FLAG_SIZE);


    if (get_cpuid() == member_ids[i]) {
      for (int j = 0; j < count; ++j) {
        *((volatile unsigned long long _SPM *)((unsigned)comm->addr[i] + (j*BARRIER_SIZE))) = BARRIER_INITIALIZED;
      }
      *((volatile unsigned long long _SPM *)((unsigned)comm->addr[i] + (count*BARRIER_SIZE) + msg_size)) = FLAG_INVALID;
    } else {      
      for (int j = 0; j < count; ++j) {
        noc_send(member_ids[i],
                (volatile int _SPM *)((unsigned)comm->addr[i] + (j*BARRIER_SIZE)),
                NOC_SPM_BASE,
                BARRIER_SIZE);
      }
      noc_send(member_ids[i],
              (volatile int _SPM *)((unsigned)comm->addr[i] + (count*BARRIER_SIZE) + msg_size),
              NOC_SPM_BASE,
              BARRIER_SIZE);
    }  
  }
  
  return 1;
}

////////////////////////////////////////////////////////////////////////////
// Functions for point-to-point transmission of data
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
  _Pragma("loopbound min 1 max 1")
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
  // Check previous acknowledgement transfer before updating counter in SPM
  if (!noc_done(mpd_ptr->send_id)) { return 0; }
  // Increment the receive counter
  (*mpd_ptr->recv_count)++;
  // Update the remote receive count
  int success = noc_nbsend(mpd_ptr->send_id,mpd_ptr->send_recv_count,mpd_ptr->recv_count,8);
  if (!success) {
    (*mpd_ptr->recv_count)--;
  }
  return success;
}

void mp_ack(mpd_t* mpd_ptr){
  // Await previous acknowledgement transfer before updating counter in SPM
  while (!noc_done(mpd_ptr->send_id)) { /* spin */ }
  // Increment the receive counter
  (*mpd_ptr->recv_count)++;
  // Update the remote receive count
  noc_send(mpd_ptr->send_id,mpd_ptr->send_recv_count,mpd_ptr->recv_count,8);
  return;
}


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


////////////////////////////////////////////////////////////////////////////
// Functions for collective behaviour
////////////////////////////////////////////////////////////////////////////

static void mp_barrier_int(communicator_t* comm, unsigned index){
  coreset_t barrier_set = comm->barrier_set;
  volatile void _SPM **addr_arr = (volatile void _SPM **)&comm->addr[0];
  unsigned local_addr = (unsigned)addr_arr[index];
  unsigned count = comm->count;
  volatile barrier_t _SPM * addr = (volatile barrier_t _SPM *)(local_addr +
                               index*BARRIER_SIZE);
  unsigned phase;
  switch(*addr){
    case BARRIER_PHASE_0:
      *addr = BARRIER_PHASE_1;
      phase = 1;
      break;
    case BARRIER_PHASE_1:
      *addr = BARRIER_PHASE_2;
      phase = 2;
      break;
    case BARRIER_PHASE_2:
      *addr = BARRIER_PHASE_3;
      phase = 3;
      break;
    case BARRIER_PHASE_3:
      *addr = BARRIER_PHASE_0;
      phase = 0;
      break;
  }
  unsigned local_val = (unsigned)addr + (phase & 1)*sizeof(unsigned);
  DEBUGGER("\tBarrier phase\t%016llx\n\tPhase\t%d\n\tLocal val\t%08x@%08x\n",
            *addr,
            phase,
            *(volatile unsigned _SPM *)local_val,
            local_val);
  noc_multisend_cs(&barrier_set,addr_arr,index*BARRIER_SIZE,addr,BARRIER_SIZE);
  
  void * addr_a = (void*)local_addr;
  _Pragma("loopbound min 0 max 9")
  for(unsigned cpu_index = 0; cpu_index < count; cpu_index++) {
    unsigned remote_val = (unsigned)addr_a + cpu_index*BARRIER_SIZE+ (phase & 1)*sizeof(unsigned);
    DEBUGGER("\t--------\n\tCPUIDX\t%u\n\tRemote val\t%08x@%08x\n",
              cpu_index,
              *(volatile unsigned _SPM *)remote_val,
              remote_val);
    unsigned comp = *(volatile unsigned _SPM *)local_val;

    _Pragma("loopbound min 1 max 1")
    while (!(*(volatile unsigned _SPM *)remote_val == comp)) {
    }
    DEBUGGER("\tBarrier reached\n");
    
  }
  return;
}

void mp_barrier(communicator_t* comm){
  //if(coreset_contains(get_cpuid(),&comm->barrier_set) == 0) {
  //  if(get_cpuid() == 0) {
  //    printf("mp_barrier(): Bad barrier call!!");
  //  }
  //}
  //DEBUG_CORECHECK(coreset_contains(get_cpuid(),&comm->barrier_set) != 0);
  // Something bad happens if mp_barrier() is called by a core
  // that is not in the communicator.
  unsigned index = 0;
  unsigned cpuid = get_cpuid();
  coreset_t barrier_set = comm->barrier_set;
  for (unsigned i = 0; i < get_cpucnt(); ++i) {
    if(coreset_contains(i,&barrier_set) != 0 && i < cpuid) {
      index++;
    }
  }
  DEBUGGER("mp_barrier():\n\tIndex\t%d\n",index);
  mp_barrier_int(comm,index);
  return;
}

void mp_broadcast(communicator_t* comm, coreid_t root) {
  unsigned index = 0;
  for (unsigned i = 0; i < get_cpucnt(); ++i) {
    if(coreset_contains(i,&comm->barrier_set) != 0 && i < get_cpuid()) {
      index++;
    }
  }
  mp_barrier_int(comm,index);
  

  volatile unsigned _SPM * flag_addr = (volatile unsigned _SPM*)(
                                  (unsigned)comm->addr[index]
                                           +comm->count*BARRIER_SIZE
                                           +comm->msg_size);

  DEBUGGER("mp_broadcast():\n\tindex\t%d\n",index);
  DEBUGGER("\tflag_addr\t%x\n",(unsigned)flag_addr);
  
  if (get_cpuid() == root) {
    *flag_addr = FLAG_VALID;
    volatile unsigned _SPM * src_addr = (volatile unsigned _SPM*)(
                                  (unsigned)comm->addr[index]
                                           +comm->count*BARRIER_SIZE);
    DEBUGGER("\tsrc addr\t%08x\n\tflag val\t%08x\n",(unsigned)src_addr,(unsigned)(*flag_addr));
    noc_multisend_cs(&comm->barrier_set,
                     (volatile void _SPM **)comm->addr,
                     comm->count*BARRIER_SIZE,
                     (volatile void _SPM *)src_addr,
                     comm->msg_size+BARRIER_SIZE);
    noc_wait_dma(comm->barrier_set);
  } else {

    _Pragma("loopbound min 1 max 1")
    while(!(*flag_addr == FLAG_VALID)) {
      /* Spin */
      DEBUGGER("\tflag value %08x\n",(unsigned)*flag_addr);
    }
  }
  *flag_addr = FLAG_INVALID;
  return;
}
