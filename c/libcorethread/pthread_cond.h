/** \addtogroup pthread_cond
 *  @{
 */

/**
 * \file pthread_cond.h Definitions for pthread_cond.
 * 
 * \author Torur Biskopsto Strom <torur.strom@gmail.com>
 *
 * \brief POSIX conditional variables for the T-CREST platform
 */

#ifndef _PTHREAD_COND_H_
#define _PTHREAD_COND_H_

#include <time.h>

typedef struct
{
  char head;
  char tail;
  long long signals;
  // We use an array to avoid malloc in pthread_cond_wait. Max 64 threads/cores
  char waiting[64];
} pthread_cond_t;

typedef struct
{
  int __dummy;
} pthread_condattr_t;

#define _PTHREAD_COND_NULL  (-1)

/* This is used to statically initialize a pthread_cond_t. Example:
  
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
 */

#define PTHREAD_COND_INITIALIZER  ((pthread_cond_t) {_PTHREAD_COND_NULL, _PTHREAD_COND_NULL, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}})

int pthread_cond_init(pthread_cond_t *cond,  const pthread_condattr_t *cond_attr);
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec * abstime);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);
int pthread_condattr_init(pthread_condattr_t *attr);
int pthread_condattr_destroy(pthread_condattr_t *attr);

#endif /* _PTHREAD_COND_H_ */

/** @}*/
