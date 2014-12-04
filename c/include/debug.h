/*
 * Debug macros for the patmos multi-processor
 *
 * Author: Rasmus Bo Sorensen (rasmus@rbscloud.dk)
 * Copyright: DTU, BSD License
 *
 */
#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdio.h>
#include "bootloader/cmpboot.h"

// SEVERITY
#define INFO    0
#define WARNING 1
#define FAULT   2
#define ERROR   3
#define FAILURE 4


// TRACE LEVEL
// Any trace with equal or higher severity than the trace level is printed
#define TRACE_LEVEL INFO

#define TRACE_NAME(x) ( x == INFO ? "INFO" : \
                      ( x == WARNING ? "WARNING" : \
                      ( x == FAULT ? "FAULT" : \
                      ( x == ERROR ? "ERROR" : \
                      ( x == FAILURE ? "FAILURE" : "" )))))



#define ENSURE(pred, x) { \
  if (!(pred)) { \
    DPRINTF("ENSURE:\t%s [%s:%d]\n",x,__FILE__,__LINE__); \
    abort(); \
  }}
#define TRACE(severity, pred, x) { \
  if ((severity>=TRACE_LEVEL) && (pred))  { \
    DPRINTF("%s:\t%s [%s:%d]\n",TRACE_NAME(severity),x,__FILE__,__LINE__); \
  }}

#ifdef DEBUG_VERBOSE
#define debugf(x)     {std::cout << __FILE__ << ":" << __LINE__ << ":\t " #x " = '" << (x) << "'" << std::endl;}
#define debugs(x)     {std::cout << __FILE__ << ":" << __LINE__ << ":\n " << x << "'" << std::endl;}
#else
#define debugf(x)
#define debugs(x)
#endif


#define DPRINTF(...)  if(get_cpuid() == NOC_MASTER) { \
                      printf(__VA_ARGS__); \
                   }


#define wait(microseconds) { \
  unsigned long long hidden_time; \
  hidden_time = get_cpu_usecs(); \
  while(get_cpu_usecs() < hidden_time + microseconds); \
} \

////////////////////////////////////////////////////////////////////////////////
// Intercore queues
////////////////////////////////////////////////////////////////////////////////

// OVERRIDE determines whether the circular buffer should save the head or
// the tail of the queue
// If OVERRIDE is 0 then the head of the queues are not overwritten, in the
// case of a full queue the msg to be enqueued is dropped.
// If OVERRIDE is 1 then the head of the queue is overwritten, in the case of
// a full queue the msg to be enqueued overwrites the head of the queue.
#define OVERWRITE 0
// MAX_MSG_SIZE  should be a power of two to make the modulo operations fast
#define MAX_MSG_SIZE 256

static char msg_arr[MAX_CORES][MAX_MSG_SIZE]; // The message size plus
                                                // two control words
static int count_arr[MAX_CORES];

static char* local_addr[MAX_CORES];
static int* local_ctrl[MAX_CORES];

#define INIT_QUEUES { \
  int cpuid = get_cpuid(); \
  for (int i = 0; i < MAX_CORES; ++i) { \
    count_arr[i] = 0; \
    local_addr[i] = (void*)mp_alloc(i,MAX_MSG_SIZE); \
    local_ctrl[i] = (int*)mp_alloc(i,8); \
    if (cpuid != NOC_MASTER) { \
      mp_mem_init(i,(void _SPM *)((unsigned)local_ctrl[i])); \
    } else { \
      ((int _SPM *)local_ctrl[i])[0] = 0; \
      ((int _SPM *)local_ctrl[i])[1] = 0; \
    } \
  } \
}

#define ENQUEUE_MSG(msg) { \
  int cpuid = get_cpuid(); \
  char _SPM * msg_queue = (char _SPM *)local_addr[cpuid]; \
  int head = ((int _SPM *)local_ctrl[cpuid])[0]; \
  int count = ((int _SPM *)local_ctrl[cpuid])[1]; \
  int len = strlen(msg); \
  int cplen; \
  if(OVERWRITE) { \
    cplen = len; \
    ((int _SPM *)local_ctrl[cpuid])[0] = (count + len <= MAX_MSG_SIZE) ? head : (head+count+len)%MAX_MSG_SIZE; \
  } else { \
    cplen = (len <= MAX_MSG_SIZE-count) ? len : MAX_MSG_SIZE-count; \
  } \
  for (int i = 0; i < cplen; i++){ \
    msg_queue[(head+count+i)%MAX_MSG_SIZE] = msg[i]; \
  } \
  ((int _SPM *)local_ctrl[cpuid])[1] = (count + len <= MAX_MSG_SIZE) ? count+len : MAX_MSG_SIZE; \
} \

// TODO: Copy 4 bytes at the time instead of 1
#define MOVE_TO_SHMEM { \
  int cpuid = get_cpuid(); \
  while (count_arr[cpuid] != 0) { \
  } \
  char _SPM * msg_queue = (char _SPM *)local_addr[cpuid]; \
  int head = ((int _SPM *)local_ctrl[cpuid])[0]; \
  int count = ((int _SPM *)local_ctrl[cpuid])[1]; \
  for (int i = 0; i < count; i++){ \
    msg_arr[cpuid][i] = (char)msg_queue[(head+i)%MAX_MSG_SIZE]; \
  } \
  count_arr[cpuid] = count; \
  ((int _SPM *)local_ctrl[cpuid])[0] = 0; \
  ((int _SPM *)local_ctrl[cpuid])[1] = 0; \
}

// TODO: Find out why "const int pcore = core;" is needed to stop core from
// looping together with i in the for loop.
#define DEQUEUE_MSG(core) { \
  const int pcore = core; \
  int count = count_arr[core]; \
  if (count != 0) { \
    printf("Core %d: ",core); \
    for (int i = 0; i < count; i++) { \
      putc(msg_arr[pcore][i],stdout); \
    } \
    putc('\n',stdout); \
    count_arr[core] = 0; \
  } \
} \

//printf("DEQUEUE_MSG\tcore %d,\tcount %d\n",core,count); \
//DPRINTF("%c@%d\n",msg_arr[pcore][i],i); \
//printf("core %d,\ti %d\n",core,i); \

#define PRINT_MSG { \
  puts("PRINT_MSG"); \
  int done = 0; \
  while(!done){ \
    for (int i = 0; i < MAX_CORES; ++i) { \
      DEQUEUE_MSG(i); \
    } \
  } \
} \

#endif  /* _DEBUG_H_ */
