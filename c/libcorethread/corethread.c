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
 * Management of corethreads
 * 
 * Author: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
 *
 */

#include "corethread.h"

#include <stdio.h>

////////////////////////////////////////////////////////////////////////////
// Functions for initializing the workers
////////////////////////////////////////////////////////////////////////////
/*
void corethread_worker(void) {
  unsigned id = get_cpuid();
  if (id != NOC_MASTER) { // Core zero should proceed to execute main
    unsigned long long time;
    boot_info->slave[id].status = STATUS_RETURN;
      
    // Wait for corethread_create request or application exit
    while(boot_info->master.status != STATUS_RETURN) {
      // As long as the master is still executing, wait for a corethread to
      // be created and then execute it.
      if (boot_info->slave[id].funcpoint != NULL) {
        funcpoint_t funcpoint = boot_info->slave[id].funcpoint;
        boot_info->slave[id].return_val = -1;
        boot_info->slave[id].status = STATUS_INITDONE;
        (*funcpoint)((void*)boot_info->slave[id].param);
        boot_info->slave[id].status = STATUS_RETURN;
        while(boot_info->slave[id].funcpoint != NULL) {

        }
      }
      time = get_cpu_usecs();
      while(get_cpu_usecs() < time+10) {
        // Wait for 10 micro seconds before checking again
      }
    }
    boot_info->slave[id].status = STATUS_RETURN;
    exit(0);
  }
  return;
}
*/
////////////////////////////////////////////////////////////////////////////
// Functions for creating and destroying corethreads
////////////////////////////////////////////////////////////////////////////

int corethread_create(int core_id, void (*start_routine)(void*),
                                                                    void *arg) {
  if(boot_info->slave[core_id].status != STATUS_INITDONE &&
                                boot_info->slave[core_id].funcpoint == NULL ) {
    boot_info->slave[core_id].param = arg;
    boot_info->slave[core_id].funcpoint = (funcpoint_t) start_routine;
    while(boot_info->slave[core_id].status != STATUS_INITDONE) {
      // Wait for corethread to respond
    }
    return 0;
  } else {
    // Corethread is not available
    return EAGAIN;
  }
}

void corethread_exit(void *retval) {
  unsigned id = get_cpuid();
  boot_info->slave[id].return_val = (int) retval;
  boot_info->slave[id].status = STATUS_RETURN;
  return;
}

int corethread_join(int core_id, void **retval) {
  unsigned long long time;
  while(boot_info->slave[core_id].status != STATUS_RETURN) {
    time = get_cpu_usecs();
    while(get_cpu_usecs() < time+10) {
      // Wait for 10 microseconds before checking again
    }
  }
  *retval = (void *) boot_info->slave[core_id].return_val;
  boot_info->slave[core_id].funcpoint = NULL;
  return 0;
}

/*
 * Implementation of pthread mutex. Should be moved to newlib.
 * 
 * Author: Torur Biskopsto Strom (torur.strom@gmail.com)
 *
 */

#include <sys/types.h>
#include <machine/patmos.h>

// Assumes that the Hardlock is connected with at least 1 lock, used as a global lock
#define _HARDLOCK_LOCK() do {asm volatile ("" : : : "memory"); *((_iodev_ptr_t) PATMOS_IO_HARDLOCK) = 1; asm volatile ("" : : : "memory");} while(0)
#define _HARDLOCK_UNLOCK() do {asm volatile ("" : : : "memory"); *((_iodev_ptr_t) PATMOS_IO_HARDLOCK) = 0; asm volatile ("" : : : "memory");} while(0)
          
/* Mutex Initialization Attributes, P1003.1c/Draft 10, p. 81 */

int pthread_mutexattr_init(pthread_mutexattr_t *attr) {
  _UNCACHED pthread_mutexattr_t * _attr = (_UNCACHED pthread_mutexattr_t *)attr;
  _attr->type = PTHREAD_MUTEX_RECURSIVE;
  return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr) {
  return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type) {
  _UNCACHED pthread_mutexattr_t * _attr = (_UNCACHED pthread_mutexattr_t *)attr;
  switch(type) {
  case PTHREAD_MUTEX_NORMAL:
  case PTHREAD_MUTEX_ERRORCHECK:
  case PTHREAD_MUTEX_RECURSIVE:
  case PTHREAD_MUTEX_DEFAULT:
    break;
  default:
    return EINVAL;
  }
  
  _attr->type = type;
  return 0;
}

int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type) {
  _UNCACHED pthread_mutexattr_t * _attr = (_UNCACHED pthread_mutexattr_t *)attr;
  *type = _attr->type;
  return 0;
}

/* Initializing and Destroying a Mutex, P1003.1c/Draft 10, p. 87 */

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) {
  
  // TODO: Decide whether we should return error codes
  // if the mutex is not uninitialized
  
  _UNCACHED pthread_mutex_t *_mutex = (_UNCACHED pthread_mutex_t *)mutex;
  _UNCACHED pthread_mutexattr_t * _attr = (_UNCACHED pthread_mutexattr_t *)attr;
  
  int type = _attr->type;
  
  switch(type) {
  case PTHREAD_MUTEX_NORMAL:
  case PTHREAD_MUTEX_ERRORCHECK:
  case PTHREAD_MUTEX_RECURSIVE:
  case PTHREAD_MUTEX_DEFAULT:
    break;
  default:
    return EINVAL;
  }
  
  _mutex->owner = _PTHREAD_MUTEX_NOOWNER;
  _mutex->type = type;
  _mutex->count = 0;
  
  return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex) {
  
  // TODO: Decide whether we should return error codes
  // if the mutex is not initialized or if it is in use
  
  return 0;
}

/*  Locking and Unlocking a Mutex, P1003.1c/Draft 10, p. 93
    NOTE: P1003.4b/D8 adds pthread_mutex_timedlock(), p. 29 */

#include <stdio.h>
int pthread_mutex_lock(pthread_mutex_t *mutex) {
  
  // TODO: Decide whether we should return error codes
  // if the mutex is not initialized
  
  _UNCACHED pthread_mutex_t *_mutex = (_UNCACHED pthread_mutex_t *)mutex;

  int id = get_cpuid();
  
  switch(_mutex->type) {
  case PTHREAD_MUTEX_NORMAL:
    break;
  case PTHREAD_MUTEX_ERRORCHECK:
    if(_mutex->owner == id)
      return EDEADLK;
    break;
  case PTHREAD_MUTEX_RECURSIVE:
  case PTHREAD_MUTEX_DEFAULT:
    if(_mutex->owner == id) {
      int count = _mutex->count;
      if(count == -1)
        return EAGAIN;
      _mutex->count++;
      return 0;
    }
    break;
  default:
    return EINVAL;
  }
  
  while(1)
  {
    _HARDLOCK_LOCK();
    if(_mutex->owner == _PTHREAD_MUTEX_NOOWNER) {
      _mutex->owner = id;
      _HARDLOCK_UNLOCK();
      break;
    }
    _HARDLOCK_UNLOCK();
  }
  // invalidate data cache to establish cache coherence
  inval_dcache();
  return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex) {
  // TODO: Decide whether we should return error codes
  // if the mutex is not initialized
  
  _UNCACHED pthread_mutex_t *_mutex = (_UNCACHED pthread_mutex_t *)mutex;
  
  int id = get_cpuid();
  
  switch(_mutex->type) {
  case PTHREAD_MUTEX_NORMAL:
    break;
  case PTHREAD_MUTEX_ERRORCHECK:
    if(_mutex->owner == id)
      return EDEADLK;
    break;
  case PTHREAD_MUTEX_RECURSIVE:
  case PTHREAD_MUTEX_DEFAULT:
    if(_mutex->owner == id) {
      int count = _mutex->count;
      if(count == -1)
        return EAGAIN;
      _mutex->count++;
      return 0;
    }
    break;
  default:
    return EINVAL;
  }
  
  _HARDLOCK_LOCK();
  if(_mutex->owner == _PTHREAD_MUTEX_NOOWNER)
    _mutex->owner = id;
  else {
    _HARDLOCK_UNLOCK();
    return EBUSY;
  }
  _HARDLOCK_UNLOCK();
  
  // invalidate data cache to establish cache coherence
  inval_dcache();
  return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex) {
  // TODO: Decide whether we should return error codes
  // if the mutex is not initialized
  
  _UNCACHED pthread_mutex_t *_mutex = (_UNCACHED pthread_mutex_t *)mutex;
  
  int id = get_cpuid();
  
  if (_mutex->owner != id)
    return EPERM;
  
  if(_mutex->count != 0) {
    _mutex->count--;
    return 0;
  }
  
  _mutex->owner = _PTHREAD_MUTEX_NOOWNER;
  return 0;
}
