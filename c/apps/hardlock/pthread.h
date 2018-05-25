#ifndef __PTHREAD_h
#define __PTHREAD_h

          
/* Mutex Initialization Attributes, P1003.1c/Draft 10, p. 81 */

int pthread_mutexattr_init(pthread_mutexattr_t *__attr);
int pthread_mutexattr_destroy(pthread_mutexattr_t *__attr);

/* Initializing and Destroying a Mutex, P1003.1c/Draft 10, p. 87 */

int pthread_mutex_init(pthread_mutex_t *__mutex, _CONST pthread_mutexattr_t *__attr);
int pthread_mutex_destroy(pthread_mutex_t *__mutex);

/* This is used to statically initialize a pthread_mutex_t. Example:
  
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
 */

#define PTHREAD_MUTEX_INITIALIZER  ((pthread_mutex_t) 0xFFFFFFFF)

/*  Locking and Unlocking a Mutex, P1003.1c/Draft 10, p. 93
    NOTE: P1003.4b/D8 adds pthread_mutex_timedlock(), p. 29 */

int pthread_mutex_lock(pthread_mutex_t *__mutex);
int pthread_mutex_trylock(pthread_mutex_t *__mutex);
int pthread_mutex_unlock(pthread_mutex_t *__mutex);

#endif
/* end of include file */
