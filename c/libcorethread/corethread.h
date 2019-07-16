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

/** \addtogroup libcorethread
 *  @{
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
#include <machine/rtc.h>
#include <machine/boot.h>


/// \brief Resource unavailable.
#define EAGAIN 1
/// \brief Invalid argument.
#define EINVAL 2
/// \brief Operation not permitted.
#define EPERM 3
/// \brief No such resource.
#define ESRCH 4
/// \brief Resource deadlock avoided.
#define EDEADLK 5
/// \brief Resource is already in use.
#define EBUSY 6

/// \brief The master core, which governs booting and startup synchronization.
extern const int NOC_MASTER;

////////////////////////////////////////////////////////////////////////////
// Data structures for storing state information
// of the corethreads
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// Functions for initializing the corethreads
////////////////////////////////////////////////////////////////////////////
/// \cond PRIVATE
/// \brief A contructor that makes the slave cores wait for a corethread to
/// be created on that core and the master core continue to execute #main().
//void corethread_worker(void) __attribute__ ((constructor(110),used));

//#ifdef CORETHREAD_INIT
// Pull in initializer, even if nothing else from the library is used
//static const void * volatile __corethread_include __attribute__((used)) = &corethread_worker;
//#endif
/// \endcond

////////////////////////////////////////////////////////////////////////////
// Functions for creating and destroying corethreads
////////////////////////////////////////////////////////////////////////////

/// \brief Creates a corethread on the core with the COREID equal to the
/// id specified by thread
///
/// \param thread A pointer to the corethread_t specifying the thread to
/// start
/// \param start_routine A function pointer to the function that the created
/// thread should start executing
/// \param arg A pointer to the argument to pass to the start_routine
/// function
///
/// \retval 0 The thread was created
/// \retval EAGAIN The corethread is already allocated 
/// \retval EINVAL The attribute value is invalid 
/// \retval EPERM The caller does not have appropriate permissions the set
/// the required scheduling parameters or scheduling policy
int corethread_create(int core_id, void (*start_routine)(void*), void *arg);

/// \brief The last function to be called by a terminating thread
/// 
/// The running corethread terminates with the given returnvalue
/// \param retval A pointer to the return value that the calling
/// thread shall return.
void corethread_exit(void *retval);

/// \brief The caller waits for the corethread, specified by thread, to
/// write a return value and terminate exection.
/// \param thread The thread struct belonging to the terminating corethread.
/// \param retval A pointer to the pointer that is to point to the return
/// value of the terminating thread.
///
/// \retval 0 The specified thread was joined.
/// \retval EINVAL The given corethread can not be joined.
/// \retval ESRCH  No corethread exist with the specified corethread ID.
/// \retval EDEADLK A deadlock was detected or the specified corethread is
/// the calling thread
int corethread_join(int core_id, void **retval);



/*
 * Implementation of pthread mutex. Should be moved to newlib.
 * 
 * Author: Torur Biskopsto Strom (torur.strom@gmail.com)
 *
 */

typedef struct {
  volatile int owner;
  volatile int type;
  volatile int count;
} pthread_mutex_t;

typedef struct {
  volatile int type;
} pthread_mutexattr_t;

#define _PTHREAD_MUTEX_NOOWNER  (-1)
#define PTHREAD_MUTEX_NORMAL 1
#define PTHREAD_MUTEX_ERRORCHECK 2
#define PTHREAD_MUTEX_RECURSIVE 3
#define PTHREAD_MUTEX_DEFAULT 4

/* This is used to statically initialize a pthread_mutex_t. Example:
  
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
 */

#define PTHREAD_MUTEX_INITIALIZER  ((pthread_mutex_t) {_PTHREAD_MUTEX_NOOWNER, PTHREAD_MUTEX_RECURSIVE, 0})

int pthread_mutexattr_init(pthread_mutexattr_t *attr);
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);
int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type);
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

#endif /* _CORETHREAD_H_ */

/** @}*/
