/** \addtogroup pthread_mutex
 *  @{
 */

/**
 * \file pthread_mutex.h Definitions for pthread_mutex.
 * 
 * \author Torur Biskopsto Strom <torur.strom@gmail.com>
 *
 * \brief POSIX mutexes for the T-CREST platform
 */

#ifndef _PTHREAD_MUTEX_H_
#define _PTHREAD_MUTEX_H_

typedef struct {
  volatile int owner;
  volatile int type;
  volatile int count;
  volatile int ticket_req;
  volatile int ticket_cur;
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

#define PTHREAD_MUTEX_INITIALIZER  ((pthread_mutex_t) {_PTHREAD_MUTEX_NOOWNER, PTHREAD_MUTEX_RECURSIVE, 0, 0, 0})

int pthread_mutexattr_init(pthread_mutexattr_t *attr);
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);
int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type);
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

#endif /* _PTHREAD_MUTEX_H_ */

/** @}*/
