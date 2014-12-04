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


/**
 * \file mp.h Definitions for libmp.
 * 
 * \author Rasmus Bo Soerensen <rasmus@rbscloud.dk>
 *
 * \brief Message passing library for the T-CREST platform
 *
 * It is up to the programmer to allocate buffering space in the communication
 * scratch pads. The allocation is specified in the mp_send_init() and
 * mp_recv_init() functions.
 *
 * The size of the message passing buffer structure in the commuincation
 * scratch pads are:
 *
 * Sender side:
 *       2 * (buf_size + FLAG_SIZE) + sizeof(recv_count)(Aligned to DW)
 *
 * Receiver side:
 *       num_buf * (buf_size + FLAG_SIZE) + sizeof(remote_recv_count)
 *                                                       (Aligned to DW)
 *
 * The local and remote addresses set in mp_send_init() and mp_recv_init()
 * have to be choosen such that they do not overlap.
 *
 */

#ifndef _MP_H_
#define _MP_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <machine/patmos.h>
#include <machine/spm.h>
#include "libnoc/noc.h"
#include "libnoc/coreset.h"
#include "bootloader/cmpboot.h"

/*! \def DWALIGN
 * Alignes X to double word size
 */

#define DWALIGN(X) dw_align(X)

static inline unsigned dw_align(unsigned x){
  if ((x & 7) != 0) {
    x = ((x>>3)+1)<<3;
  }
  return x;
}


/*! \def FLAG_SIZE
 * The size of the flag used to detect completion of a received message.
 * This flag is placed at the end of the message to be send.
 * The flag size is aligned to double words.
 */
#define FLAG_SIZE DWALIGN(8)

// Possible Flag types
#define FLAG_VALID 0xFFFFFFFF
#define FLAG_INVALID 0x00000000

// Possible Barrier states
#define BARRIER_T unsigned long long
#define BARRIER_INITIALIZED (BARRIER_T)0x0000000000000000
#define BARRIER_PHASE_0 (BARRIER_T)0x0000000000000000
#define BARRIER_PHASE_1 (BARRIER_T)0x00000000FFFFFFFF
#define BARRIER_PHASE_2 (BARRIER_T)0xFFFFFFFFFFFFFFFF
#define BARRIER_PHASE_3 (BARRIER_T)0xFFFFFFFF00000000

#define BARRIER_REACHED 0xFFFFFFFF


 /*! \def BARRIER_SIZE
 * The size of the barrier flag for each core.
 */
#define BARRIER_SIZE DWALIGN(sizeof(BARRIER_T))

/*! \def NUM_WRITE_BUF
 * DO NOT CHANGE! The number of write pointers is not 
 * defined in a way that is can be changed
 */
#define NUM_WRITE_BUF 2

#define BARRIER_CORES 9

typedef unsigned coreid_t;

////////////////////////////////////////////////////////////////////////////
// Data structures for storing state information
// of the message passing channels
////////////////////////////////////////////////////////////////////////////

/// \struct mpd_t
/// \brief Message passing descriptor.
///
/// The struct is used to store the data describing the massage passing channel.
/// This struct is used to describe both the sending and receiving ends of a
/// communication channel.
typedef struct {
  /*-- Shared variables --*/
  /** The ID of the sender */
  coreid_t send_id;
  /** The address of the sender buffer structure */
  volatile void _SPM * send_addr;
  /** The ID of the receiver */
  coreid_t recv_id;
  /** The address of the receiver buffer structure */
  volatile void _SPM * recv_addr; 
  /** The size of a buffer in bytes */
  unsigned buf_size;
  /** The number of buffers at the receiver */
  unsigned num_buf;

  /*-- Sender/receiver specific variables --*/
  /** The number of messages received by the receiver */
  volatile unsigned _SPM * recv_count;
  /** The address of the recv_count at the sender */
  volatile unsigned _SPM * send_recv_count;
  /** The number of messages sent by the sender */
  unsigned send_count;
  /** A pointer to the tail of the receiving queue */
  unsigned send_ptr;
  /** A pointer to the head of the receiving queue */
  unsigned recv_ptr;
  /** A pointer to the free write buffer */
  volatile void _SPM * write_buf;
  /** A pointer to the used write buffer */
  volatile void _SPM * shadow_write_buf;
  /** A pointer to the currently free read buffer */
  volatile void _SPM * read_buf;

} mpd_t __attribute__((aligned(16)));

/// \struct communicator_t
/// \brief Describes at set of communicating processors.
///
/// The struct is used to store all necessary information on the set of
/// communicating processors.
typedef struct {
  coreset_t barrier_set;
  unsigned count;
  unsigned msg_size;
  volatile void _SPM ** addr;
} communicator_t __attribute__((aligned(16)));


// This array is only used by mp_alloc, it should not be cached.
static volatile unsigned* _UNCACHED spm_alloc_array[MAX_CORES];

////////////////////////////////////////////////////////////////////////////
// Functions for memory management in the communication SPM
////////////////////////////////////////////////////////////////////////////

static void mp_init(void) __attribute__((constructor(120),used));

/// \brief A function for returning the amount of data that the channel is
/// alocating in the sending spm.
//size_t mp_send_alloc_size(mpd_t* mpd_ptr);

/// \brief A function for returning the amount of data that the channel is
/// alocating in the receiving spm.
//size_t mp_recv_alloc_size(mpd_t* mpd_ptr);

/// \brief Static memory allocation on the communication scratchpad.
/// No mp_free function
void _SPM * mp_alloc(coreid_t id, unsigned size);

/// \brief Function for initializing SPM memory in other cores.
void mp_mem_init(coreid_t id, void _SPM* spm_addr);

////////////////////////////////////////////////////////////////////////////
// Functions for initializing the message passing API
////////////////////////////////////////////////////////////////////////////

/// \brief Initialize the state of a communication channel
///
/// \param mpd_ptr A pointer the the message passing descriptor
/// \param sender The core id of the sending processor
/// \param receiver The core id of the receiving processor
/// \param buf_size The size of the message buffer
/// \param num_buf The number of buffers in the receiving scratchpad
///
/// \retval 0 The local or remote addresses were not aligned to double words.
/// \retval 1 The initialization of the send channel succeeded.
int mp_chan_init(mpd_t* mpd_ptr, coreid_t sender, coreid_t receiver,
              unsigned buf_size, unsigned num_buf);

/// \brief Initialize the communicator
///
/// \param comm A pointer to the communicator structure
/// \param member_ids An array of member ids.
/// \param member_addrs An array of COM SPM addresses for the members.
///
/// \retval 0 The address is not aligned  to double words.
/// \retval 1 The initialization of the communicator_t succeeded.
int mp_communicator_init(communicator_t* comm, unsigned count,
              const coreid_t member_ids [], unsigned msg_size);

////////////////////////////////////////////////////////////////////////////
// Functions for transmitting data
////////////////////////////////////////////////////////////////////////////

/// \brief Non-blocking function for passing a message to a remote processor
/// under flow control. The data to be passed by the function should be in the
/// local buffer in the communication scratch pad before the function
/// is called.
///
/// \param mpd_ptr A pointer to the message passing data structure
/// for the given message passing channel.
///
/// \retval 0 The send did not succeed, either there was no space in the
/// receiving buffer or there was no free DMA to start a transfere
/// \retval 1 The send succeeded.
int mp_nbsend(mpd_t* mpd_ptr);

/// \brief A function for passing a message to a remote processor under
/// flow control. The data to be passed by the function should be in the
/// local buffer in the communication scratch pad before the function
/// is called.
///
/// \param mpd_ptr A pointer to the message passing data structure
/// for the given message passing channel.
///
/// \returns The function returns when the send has succeeded.
void mp_send(mpd_t* mpd_ptr);

/// \brief Non-blocking function for receiving a message from a remote processor
/// under flow control. The data that is received is placed in a message buffer
/// in the communication scratch pad, when the received message is no
/// longer used the reception of the message should be acknowledged with
/// the #mp_ack()
///
/// \param mpd_ptr A pointer to the message passing data structure
/// for the given message passing channel.
///
/// \retval 0 No message has been received yet.
/// \retval 1 A message has been received and dequeued. The call has to be
/// followed by a call to #mp_ack() when the data is no longer used.
int mp_nbrecv(mpd_t* mpd_ptr);

/// \brief A function for receiving a message from a remote processor under
/// flow control. The data that is received is placed in a message buffer
/// in the communication scratch pad, when the received message is no
/// longer used the reception of the message should be acknowledged with
/// the #mp_ack()
///
/// \param mpd_ptr A pointer to the message passing data structure
/// for the given message passing channel.
///
/// \returns The function returns when a message is received.
void mp_recv(mpd_t* mpd_ptr);

/// \brief Non-blocking function for acknowledging the reception of a message.
/// This function should be used with extra care, if no acknowledgement is sent
/// the communication channel will be blocked until an acknowledgement is sent.
/// This function shall be called to release space in the receiving
/// buffer when the received data is no longer used.
/// It is not necessary to call #mp_ack() after each #mp_recv() call.
/// It is possible to work on 2 or more incomming messages at the same
/// time with out them being overwritten.
///
/// \param mpd_ptr A pointer to the message passing data structure
/// for the given message passing channel.
///
/// \retval 0 No acknowledgement has been sent.
/// \retval 1 An acknowledgement has been sent.
int mp_nback(mpd_t* mpd_ptr);

/// \brief A function for acknowledging the reception of a message.
/// This function shall be called to release space in the receiving
/// buffer when the received data is no longer used.
/// It is not necessary to call #mp_ack() after each #mp_recv() call.
/// It is possible to work on 2 or more incomming messages at the same
/// time with out them being overwritten.
///
/// \param mpd_ptr A pointer to the message passing data structure
/// for the given message passing channel.
///
/// \returns The function returns when an acknowledgement has been sent.
void mp_ack(mpd_t* mpd_ptr);

/// \brief A function to syncronize the cores described in the communicator
/// to a barrier.
///
/// \param comm A pointer to a communicator structure.
void mp_barrier(communicator_t* comm);
static void mp_barrier_int(communicator_t* comm, unsigned index);

/// \brief A function for broadcasting a message to all members of
/// a communicator
void mp_broadcast(communicator_t* comm, coreid_t root);

#endif /* _MP_H_ */
