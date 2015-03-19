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
// Function for initializing collective behavior
////////////////////////////////////////////////////////////////////////////

int mp_communicator_init(communicator_t* comm, unsigned count,
              const coreid_t member_ids [], unsigned msg_size) {
  return 0;
}

//int mp_communicator_init(communicator_t* comm, unsigned count,
//              const coreid_t member_ids [], unsigned msg_size) {
//  if (get_cpuid() != NOC_MASTER) {
//    return 0;
//  }
//  comm->count = count;
//  comm->msg_size = msg_size;
//  comm->addr = (volatile void _SPM **) malloc(sizeof(void*)*count);
//
//  TRACE(INFO,TRUE,"mp_communicator_init(): malloc size %lu\n",sizeof(void*)*count);
//  
//  coreset_clearall(&comm->barrier_set);
//  for (int i = 0; i < count; ++i) {
//    coreset_add(member_ids[i],&comm->barrier_set);
//    comm->addr[i] = (volatile void _SPM *) mp_alloc(member_ids[i],count*BARRIER_SIZE+msg_size+FLAG_SIZE);
//
//
//    if (get_cpuid() == member_ids[i]) {
//      for (int j = 0; j < count; ++j) {
//        *((volatile unsigned long long _SPM *)((unsigned)comm->addr[i] + (j*BARRIER_SIZE))) = BARRIER_INITIALIZED;
//      }
//      *((volatile unsigned long long _SPM *)((unsigned)comm->addr[i] + (count*BARRIER_SIZE) + msg_size)) = FLAG_INVALID;
//    } else {      
//      for (int j = 0; j < count; ++j) {
//        noc_send(member_ids[i],
//                (volatile int _SPM *)((unsigned)comm->addr[i] + (j*BARRIER_SIZE)),
//                NOC_SPM_BASE,
//                BARRIER_SIZE);
//      }
//      noc_send(member_ids[i],
//              (volatile int _SPM *)((unsigned)comm->addr[i] + (count*BARRIER_SIZE) + msg_size),
//              NOC_SPM_BASE,
//              BARRIER_SIZE);
//    }  
//  }
//  
//  return 1;
//}

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
  TRACE(INFO,TRUE,"\tBarrier phase\t%016llx\n\tPhase\t%d\n\tLocal val\t%08x@%08x\n",
            *addr,
            phase,
            *(volatile unsigned _SPM *)local_val,
            local_val);
  noc_multisend_cs(&barrier_set,addr_arr,index*BARRIER_SIZE,addr,BARRIER_SIZE);
  
  void * addr_a = (void*)local_addr;
  _Pragma("loopbound min 0 max 9")
  for(unsigned cpu_index = 0; cpu_index < count; cpu_index++) {
    unsigned remote_val = (unsigned)addr_a + cpu_index*BARRIER_SIZE+ (phase & 1)*sizeof(unsigned);
    TRACE(INFO,TRUE,"\t--------\n\tCPUIDX\t%u\n\tRemote val\t%08x@%08x\n",
              cpu_index,
              *(volatile unsigned _SPM *)remote_val,
              remote_val);
    unsigned comp = *(volatile unsigned _SPM *)local_val;

    _Pragma("loopbound min 1 max 1")
    while (!(*(volatile unsigned _SPM *)remote_val == comp)) {
    }
    TRACE(INFO,TRUE,"\tBarrier reached\n");
    
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