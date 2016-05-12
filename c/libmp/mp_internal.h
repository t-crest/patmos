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

/** \addtogroup libmp
 *  @{
 */

/**
 * \file mp_internal.h Definitions for libmp.
 * 
 * \author Rasmus Bo Soerensen <rasmus@rbscloud.dk>
 *
 * \brief Internal header for message passing library
 *
 */

#ifndef _MP_INTERNAL_H_
#define _MP_INTERNAL_H_

/// \brief Aligns X to word size
#define WALIGN(X) (((X)+0x3) & ~0x3)

#define SINGLE_NOC              0
#define SINGLE_SHM              1
#define MULTI_NOC               2
#define MULTI_NOC_NONBLOCKING   3
#define MULTI_NOC_MP            4

#ifndef IMPL
#define IMPL SINGLE_NOC
#endif

/*! \def FLAG_SIZE
 * \brief The size of the flag used to detect completion of a received message.
 *
 * This flag is placed at the end of the message to be send.
 * The flag size is aligned to words.
 */
#define FLAG_SIZE WALIGN(sizeof(unsigned int))

/// \cond PRIVATE
// Possible Flag types
#define FLAG_VALID   0xFFFFFFFF
#define FLAG_INVALID 0x00000000
/// \endcond

/// \brief The type of the synchronization flag of a barrier.
typedef unsigned int barrier_t;

/// \cond PRIVATE
// Possible Barrier states
#define BARRIER_INITIALIZED (barrier_t)0x00000000
#define BARRIER_PHASE_0 (barrier_t)0x00000000
#define BARRIER_PHASE_1 (barrier_t)0x0000FFFF
#define BARRIER_PHASE_2 (barrier_t)0xFFFFFFFF
#define BARRIER_PHASE_3 (barrier_t)0xFFFF0000

#define BARRIER_REACHED 0xFFFFFFFF
/// \endcond

/// \brief The size of the barrier flag for each core.
#define BARRIER_SIZE WALIGN(sizeof(barrier_t))

/// \cond PRIVATE
/*! \def NUM_WRITE_BUF
 * DO NOT CHANGE! The number of write pointers is not 
 * defined in a way that is can be changed
 */
#define NUM_WRITE_BUF 2
/// \endcond

////////////////////////////////////////////////////////////////////////////
// Data structures for storing state information
// of the message passing channels
////////////////////////////////////////////////////////////////////////////

typedef enum {SAMPLING, QUEUING} port_t;

/// \struct chan_info_t
/// \brief Struct for exchanging initialization information
/// between the two ends of a channel.
///
/// The structure contains the pointers to the source and the sink
/// ports that are connected.
struct _chan_info_t;
typedef struct _chan_info_t chan_info_t;
struct _chan_info_t {
  port_t port_type;
  int src_id;
  int sink_id;
  volatile void _SPM * src_addr;
  volatile void _SPM * sink_addr;
  volatile LOCK_T * src_lock;
  volatile LOCK_T * sink_lock;
  /** The following  */
  union {
    /** Queuing port specific fields */
    struct {
      qpd_t * src_qpd_ptr;
      qpd_t * sink_qpd_ptr;
    };
    /** sampling port specific fields */
    struct {
      spd_t * src_spd_ptr;
      spd_t * sink_spd_ptr;
    };
  };

} ;

extern volatile _UNCACHED chan_info_t chan_info[MAX_CHANNELS];

size_t mp_send_alloc_size(qpd_t * qpd_ptr);

size_t mp_recv_alloc_size(qpd_t * qpd_ptr);

int test_spm_size();

#endif /* _MP_INTERNAL_H_ */

/** @}*/
