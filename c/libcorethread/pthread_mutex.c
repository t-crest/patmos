/*
 * Implementation of pthread_mutex. Should be moved to newlib.
 * 
 * Author: Torur Biskopsto Strom (torur.strom@gmail.com)
 *
 */

#include "pthread_mutex.h"

#include <sys/types.h>
#include <machine/patmos.h>

// Assumes that the Hardlock is connected with at least 1 lock, used as a global lock
#ifndef _HARDLOCK_LOCK
#define _HARDLOCK_LOCK() do {asm volatile ("" : : : "memory"); *((_iodev_ptr_t) PATMOS_IO_HARDLOCK) = 1; asm volatile ("" : : : "memory");} while(0)
#endif
#ifndef _HARDLOCK_UNLOCK
#define _HARDLOCK_UNLOCK() do {asm volatile ("" : : : "memory"); *((_iodev_ptr_t) PATMOS_IO_HARDLOCK) = 0; asm volatile ("" : : : "memory");} while(0)
#endif
          
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
  _mutex->ticket_req = 0;
  _mutex->ticket_cur = 0;
  
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
      _mutex->count = count+1;
      return 0;
    }
    break;
  default:
    return EINVAL;
  }
  
  _HARDLOCK_LOCK();
  int ticket = _mutex->ticket_req++;
  _HARDLOCK_UNLOCK();

  while(1)
    if(ticket == _mutex->ticket_cur)
      break;

  _mutex->owner = id;
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
      _mutex->count = count+1;
      return 0;
    }
    break;
  default:
    return EINVAL;
  }
  
  _HARDLOCK_LOCK();
  int ticket = _mutex->ticket_req;
  if(ticket != _mutex->ticket_cur) {
    // Already owned
    _HARDLOCK_UNLOCK();
    return EBUSY;
  }

  _mutex->ticket_req = ticket+1;
  _mutex->owner = id;

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
  _mutex->ticket_cur++;
  return 0;
}
