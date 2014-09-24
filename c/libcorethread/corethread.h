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
 * \file corethread.h Definitions for libcorethread.
 * 
 * \author Rasmus Bo Soerensen <rasmus@rbscloud.dk>
 *
 * \brief Corethread library for the T-CREST platform
 */

#ifndef _CORETHREAD_H_
#define _CORETHREAD_H_

#include <machine/patmos.h>
#include <machine/spm.h>
#include <stdlib.h>
#include "libmp/mp.h"
#include "bootloader/cmpboot.h"
#include "libnoc/noc.h"
#include "libnoc/coreset.h"

#define EAGAIN 1
#define EINVAL 2
#define EPERM 3
#define ESRCH 4
#define EDEADLK 5

////////////////////////////////////////////////////////////////////////////
// Data structures for storing state information
// of the corethreads
////////////////////////////////////////////////////////////////////////////

typedef enum {
   POLLING,
   SLEEPING
} corethread_attr_t;

typedef struct {
   void (* start)(void*);
   void* arg;
   corethread_attr_t attr;
   volatile void _SPM *ret_addr;
} corethread_t;


////////////////////////////////////////////////////////////////////////////
// Functions for initializing the corethreads
////////////////////////////////////////////////////////////////////////////

/// \brief Make the slave cores wait for a corethread to be started on that
/// core.
// void corethread_worker(void) __attribute__((constructor(10000)));
// Leave out constructor for initial testing
void corethread_worker(void);

/// \brief Initialize the communication channel from the master to the
/// corethread. The pointers in the receiver are set statically.
int corethread_init(volatile corethread_t _SPM *corethread_ptr, int recv_id,
                                          volatile void _SPM *ret_addr);

////////////////////////////////////////////////////////////////////////////
// Functions for creating and destroying corethreads
////////////////////////////////////////////////////////////////////////////

/// \brief corethread creation
///
/// \param mpd_ptr
/// \param attr The attributes of the corethread to be created
/// \param start_routine
/// \param arg
///
/// \retval 0 the function call was successful
/// \retval EAGAIN The corethread is already allocated 
/// \retval EINVAL The attribute value is invalid 
/// \retval EPERM The caller does not have appropriate permissions the set
/// the required scheduling parameters or scheduling policy
int corethread_create(volatile corethread_t _SPM *corethread_ptr,
      const corethread_attr_t *attr, void *(*start_routine)(void*), void *arg);

/// \brief corethread termination
/// 
/// The running corethread terminates with the given returnvalue
/// \param retval_ptr
void corethread_exit(void *retval_ptr);

/// \brief wait for corethread termination
/// \param mpd_ptr
/// \param retcal_ptr
///
/// \retval EINVAL The given corethread can not be joined.
/// \retval ESRCH  No corethread exist with the specified corethread ID.
/// \retval EDEADLK A deadlock was detected or the specified corethread is
/// the calling thread
int corethread_join(corethread_t* corethread_ptr, void **retval_ptr);

#endif /* _CORETHREAD_H_ */
