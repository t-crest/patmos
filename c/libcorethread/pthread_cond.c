/*
 * Implementation of pthread_cond. Should be moved to newlib.
 * 
 * Author: Torur Biskopsto Strom (torur.strom@gmail.com)
 *
 */

#include "pthread_cond.h"

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *cond_attr)
{
  cond->head = _PTHREAD_COND_NULL;
  cond->tail = _PTHREAD_COND_NULL;
  return 0;
}

int pthread_cond_destroy(pthread_cond_t *cond)
{
  _UNCACHED pthread_cond_t *_cond = (_UNCACHED pthread_cond_t *)cond;
  if (_cond->head != _PTHREAD_COND_NULL) return EBUSY;
  return 0;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
  /* mutex should be locked by this thread. Unless multiple mutexes
     are associated with this condition variable, we can safely modify
     the queue. We can also do cached reads before releasing the mutex. */
  
  int id = get_cpuid();
  
  cond->waiting[id] = _PTHREAD_COND_NULL;
  
  cond->signals &= ~(1 << id); // Unset id's bit
  
  if(cond->tail == _PTHREAD_COND_NULL) {
    cond->head = id;
    cond->tail = id;
  } else {
    cond->waiting[cond->tail] = id;
    cond->tail = id;
  }
  
  pthread_mutex_unlock(mutex);
  
  _UNCACHED pthread_cond_t *_cond = (_UNCACHED pthread_cond_t *)cond;
  
  // Waiting to be signalled
  while(!((_cond->signals >> id) & 1)) {asm("");}
  
  pthread_mutex_lock(mutex);
  
  return 0;
}

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec * abstime)
{
  // Not implemented yet
  return -1;
}

int pthread_cond_signal(pthread_cond_t *cond)
{
  /* mutex should be locked by this thread. Unless multiple mutexes
     are associated with this condition variable, we can safely modify
     the queue. We can also do cached reads before releasing the mutex. */
  
  if(cond->head == _PTHREAD_COND_NULL)
    return 0;
  
  if(cond->head != _PTHREAD_COND_NULL) {
    cond->signals |= 1 << cond->head; // Set head's bit
    
    if(cond->head != cond->tail)
      cond->head = cond->waiting[cond->head];
    else {
      cond->head = _PTHREAD_COND_NULL;
      cond->tail = _PTHREAD_COND_NULL;
    }
  }
  return 0;
}

int pthread_cond_broadcast(pthread_cond_t *cond)
{
  if(cond->head == _PTHREAD_COND_NULL)
    return 0;
  
  while(cond->head != _PTHREAD_COND_NULL) {
    cond->signals |= 1 << cond->head; // Set head's bit
    
    if(cond->head != cond->tail)
      cond->head = cond->waiting[cond->head];
    else {
      cond->head = _PTHREAD_COND_NULL;
      cond->tail = _PTHREAD_COND_NULL;
    }
  }
  return 0;
}

int pthread_condattr_init(pthread_condattr_t *attr)
{
  return 0;
}

int pthread_condattr_destroy(pthread_condattr_t *attr)
{
  return 0;
}
