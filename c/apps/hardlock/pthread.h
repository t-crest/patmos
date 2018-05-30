#ifndef __PTHREAD_h
#define __PTHREAD_h

#include <sys/types.h>


typedef unsigned int pthread_mutex_t;      /* identify a mutex */

typedef struct {
  int   is_initialized;
#if defined(_POSIX_THREAD_PROCESS_SHARED)
  int   process_shared;  /* allow mutex to be shared amongst processes */
#endif
#if defined(_POSIX_THREAD_PRIO_PROTECT)
  int   prio_ceiling;
  int   protocol;
#endif
#if defined(_UNIX98_THREAD_MUTEX_ATTRIBUTES)
  int type;
#endif
  int   recursive;
} pthread_mutexattr_t;




#ifndef _HARDLOCK_
#ifndef _SSPM_
#ifndef _ASYNCLOCK_
#ifndef _CASPM_
#define _HARDLOCK_
#endif
#endif
#endif
#endif

#ifdef _HARDLOCK_
#include "hardlock.h"
#endif

#ifdef _ASYNCLOCK_
#include "asynclock.h"
#endif

#ifdef _CASPM_
#include "caspm.h"
#endif


#define _MAX_HARDWARE_LOCKS_ 16

_UNCACHED int _hardware_locks_ [_MAX_HARDWARE_LOCKS_];
          
/* Mutex Initialization Attributes, P1003.1c/Draft 10, p. 81 */

int pthread_mutexattr_init(pthread_mutexattr_t *__attr) {
  return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *__attr) {
  return 0;
}

/* Initializing and Destroying a Mutex, P1003.1c/Draft 10, p. 87 */

int pthread_mutex_init(pthread_mutex_t *__mutex, _CONST pthread_mutexattr_t *__attr) {
  lock(0);
  for(int i = 1; i < _MAX_HARDWARE_LOCKS_; i++) {
    if(_hardware_locks_[i] == 0) {
      _hardware_locks_[i] = 1;
      *__mutex = i;
      unlock(0);
      return 0;
    }
  }
  unlock(0);
  return -1;
}

int pthread_mutex_destroy(pthread_mutex_t *__mutex) {
  lock(0);
  _hardware_locks_[*__mutex] = 0;
  unlock(0);
  return 0;
}

/* This is used to statically initialize a pthread_mutex_t. Example:
  
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
 */

#define PTHREAD_MUTEX_INITIALIZER  ((pthread_mutex_t) 0xFFFFFFFF)

/*  Locking and Unlocking a Mutex, P1003.1c/Draft 10, p. 93
    NOTE: P1003.4b/D8 adds pthread_mutex_timedlock(), p. 29 */

int pthread_mutex_lock(pthread_mutex_t *__mutex) {
  lock(*__mutex);
  return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *__mutex) {
  // not implemented for now
  return -1;
}

int pthread_mutex_unlock(pthread_mutex_t *__mutex) {
  unlock(*__mutex);
  return 0;
}






















#endif
