/**
* PROGRAM DESCRIPTION:
*
* This is a test program for state based communication through the Argo NoC.
*
*/

/*
	Author: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
	Copyright: DTU, BSD License
*/
const int NOC_MASTER = 0;
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/lock.h>
#include <machine/patmos.h>
#include <limits.h>
#include "libcorethread/corethread.h"
//#include "libmp/mp.h"

#ifndef TT_PERIOD_US
  #define TT_PERIOD_US    1000
#endif

#define TT_SCHED_LENGHT 16
#define TIME_TRIGGERED  0
#define EVENT_DRIVEN    1
#ifndef PARADIGME
  #define PARADIGME     EVENT_DRIVEN
#endif

#define SHM             0
#define SPM             1
#ifndef DATA_PLACEMENT
  #define DATA_PLACEMENT     SPM
#endif

#define SINGLE          0
#define THREE           1
#ifndef BUFFERING
  #define BUFFERING     THREE
#endif

#define SHM_LOCK             0
#define SPM_LOCK             1
#ifndef LOCKING
  #define LOCKING     SPM_LOCK
#endif

#ifndef BUFFER_SIZE
  #define BUFFER_SIZE     128
#endif

#define SLAVE_CORE 2
#define ITERATIONS 1000

typedef unsigned int BUFFER_T;
#if DATA_PLACEMENT == SHM
  BUFFER_T buf[BUFFER_SIZE];
#endif

#if LOCKING == SHM_LOCK
  typedef _LOCK_T LOCK_T;
  LOCK_T lock;
#elif LOCKING == SPM_LOCK
  /* DO NOT change layout of _SPM_LOCK_T  it is carefully laid out
   * to be easily transfered by the NoC. */
  struct _SPM_LOCK_T {
    volatile unsigned long long int remote_entering;
    volatile unsigned long long int remote_number;
    unsigned long long int local_entering;
    unsigned long long int local_number;
    struct _SPM_LOCK_T _SPM * remote_ptr;
    unsigned char remote_cpuid;
  };
  typedef struct _SPM_LOCK_T LOCK_T_S;
  typedef LOCK_T_S _SPM * LOCK_T;

  LOCK_T_S * volatile _UNCACHED master_lock_ptr = NULL;
  LOCK_T_S * volatile _UNCACHED slave_lock_ptr = NULL;
#endif

struct conf_param_t {
  unsigned long long int tt_time;
  BUFFER_T _SPM * read_buf_ptr;
  BUFFER_T _SPM * write_buf_ptr;
};


volatile unsigned short int shared_phase = 0;
volatile int slave_error = 0;

unsigned long long int min_time = ULONG_MAX;
unsigned long long int max_time = 0;
unsigned long long int accum_time = 0;
unsigned long long int cnt_time = 0;

void print_version() {
  #if PARADIGME == TIME_TRIGGERED
    printf("PARADIGME:\tTIME_TRIGGERED\n");
  #elif PARADIGME == EVENT_DRIVEN
    printf("PARADIGME:\tEVENT_DRIVEN\n");
  #else
    printf("PARADIGME:\tUNSUPPORTED\n");
    exit(1);
  #endif
  #if DATA_PLACEMENT == SHM
    printf("DATA_PLACEMENT:\tSHM\n");
  #elif DATA_PLACEMENT == SPM
    printf("DATA_PLACEMENT:\tSPM\n");
  #else
    printf("DATA_PLACEMENT:\tUNSUPPORTED\n");
    exit(1);
  #endif
  #if PARADIGME == EVENT_DRIVEN
    #if LOCKING == SHM_LOCK
      printf("LOCKING:\tSHM_LOCK\n");
    #elif LOCKING == SPM_LOCK
      printf("LOCKING:\tSPM_LOCK\n");
    #else
      printf("LOCKING:\tUNSUPPORTED\n");
      exit(1);
    #endif
    #ifdef EXCLUDE_LOCK
      printf("Locking is excluded from timing measurements\n");
    #else
      printf("Locking is included in timing measurements\n");
    #endif
  #endif
  return;
}

void next_tick(unsigned long long int *tt_time, unsigned short int *tt_slot)  __attribute__((section(".text.spm"))) {
  static int drop_first = 0;
  unsigned long long int now = get_cpu_usecs();
  unsigned long long int nxt_tt_time = *tt_time + TT_PERIOD_US;
  if (nxt_tt_time < now) {
    // abort, time period has been exceeded.
    if (get_cpuid() == 0) {
      puts("TT period time exceeded");
      printf("TT slot: %d\n", *tt_slot);
    } else {
      slave_error++;
    }
    exit(1);
  }
  if (*tt_slot >= TT_SCHED_LENGHT-1) {
    *tt_slot = 0;
  } else {
    (*tt_slot)++;
  }
  *tt_time = nxt_tt_time;
  unsigned long long int left_time = nxt_tt_time - now;
  if (get_cpuid() == 0 && left_time < TT_PERIOD_US-2 && drop_first == 1) {
    min_time = (left_time < min_time) ? left_time : min_time;
    max_time = (left_time > max_time) ? left_time : max_time;
    accum_time += left_time;
    cnt_time++; 
  }
  drop_first = 1;
  while(nxt_tt_time >= get_cpu_usecs());
  return;
}

void read_buffer(volatile unsigned short int * phase, struct conf_param_t * conf_param)  __attribute__((section(".text.spm"))) {
  int error = 0;
  #if DATA_PLACEMENT == SHM
    inval_dcache();
  #endif
  BUFFER_T mem;
  int shm_phase = shared_phase;
  int loc_phase = *phase;
  int i;
  for (i = 0; i < BUFFER_SIZE; ++i) {
    #if DATA_PLACEMENT == SHM
      mem = buf[i];
    #elif DATA_PLACEMENT == SPM
      mem = *((conf_param->read_buf_ptr) + i);
      //printf("mem: %d\tphase: %d\ti: %d\n",mem,loc_phase\,i);
    #endif
    if (loc_phase == 0 && mem != i) {
      error++;
    } else if (loc_phase != 0 && mem != (BUFFER_T)(BUFFER_SIZE-1 - i)) {
      error++;
    }

    if (error != 0) {
      if (get_cpuid() == 0) {\
        inval_dcache();
        puts("Reading wrong values");
        printf("errors: %d\tmem: %d\tphase: %d\tshared phase: %d\ti: %d\n",error,mem,loc_phase,shm_phase,i);
        printf("write_buf_ptr: %#08x\tread_buf_ptr: %#08x\n",conf_param->write_buf_ptr,conf_param->read_buf_ptr);
      } else {
        slave_error++;
      }
      exit(1);
    }
  }

//  if (error != 0) {
//    if (get_cpuid() == 0) {\
//      inval_dcache();
//      puts("Reading wrong values");
//      printf("errors: %d\tmem: %d\tphase: %d\tshared phase: %d\ti: %d\n",error,mem,*phase,shm_phase,i);
//      printf("write_buf_ptr: %#08x\tread_buf_ptr: %#08x\n",conf_param->write_buf_ptr,conf_param->read_buf_ptr);
//    } else {
//      slave_error++;
//    }
//    exit(1);
//  }

  return;
}

void write_buffer(volatile unsigned short int * phase, struct conf_param_t * conf_param)  __attribute__((section(".text.spm"))){
  #if DATA_PLACEMENT == SHM
    if (*phase == 0) {
      for (int i = 0; i < BUFFER_SIZE; ++i) {
        buf[i] = (BUFFER_T)i;
      }
    } else {
      for (int i = 0; i < BUFFER_SIZE; ++i) {
        buf[i] = (BUFFER_T)(BUFFER_SIZE-1 - i);
      }
    }

  #elif DATA_PLACEMENT == SPM
    if (*phase == 0) {
      for (int i = 0; i < BUFFER_SIZE; ++i) {
        *((conf_param->write_buf_ptr) + i) = (BUFFER_T)i;
      }
    } else {
      for (int i = 0; i < BUFFER_SIZE; ++i) {
        *((conf_param->write_buf_ptr) + i) = (BUFFER_T)(BUFFER_SIZE-1 - i);
      }
    }
    noc_send(0,conf_param->read_buf_ptr,conf_param->write_buf_ptr,BUFFER_SIZE*sizeof(BUFFER_T),0);

    while(!noc_done(0));

  #endif
}

void initialize_lock(LOCK_T * lock) {
  #if LOCKING == SHM_LOCK
    if(get_cpuid() == 0) {
      __lock_init(*lock);
    }
  #elif LOCKING == SPM_LOCK
    *lock = (LOCK_T_S _SPM *)mp_alloc(sizeof(LOCK_T_S));
    (*lock)->remote_entering = 0;
    (*lock)->remote_number = 0;
    (*lock)->local_entering = 0;
    (*lock)->local_number = 0;
    if(get_cpuid() == 0) {
      master_lock_ptr = (LOCK_T_S * _UNCACHED )*lock;
      while(slave_lock_ptr == NULL);

      (*lock)->remote_ptr = (LOCK_T_S _SPM *)slave_lock_ptr;
      (*lock)->remote_cpuid = SLAVE_CORE;
    } else {
      slave_lock_ptr = (LOCK_T_S * _UNCACHED )*lock;
      while(master_lock_ptr == NULL);
      
      (*lock)->remote_ptr = (LOCK_T_S _SPM *)master_lock_ptr;
      (*lock)->remote_cpuid = 0;
    }
  #endif
}

void acquire_lock(LOCK_T * lock)  __attribute__((section(".text.spm"))) {
  #if LOCKING == SHM_LOCK
    __lock_acquire(*lock);
  #elif LOCKING == SPM_LOCK
    /* Write Entering true */
    /* Write Number */
    unsigned remote = (*lock)->remote_cpuid;
    unsigned id = get_cpuid();
    (*lock)->local_entering = 1;
    
    noc_send(remote,
              (void _SPM *)&((*lock)->remote_ptr->remote_entering),
              (void _SPM *)&(*lock)->local_entering,
              sizeof((*lock)->local_entering),
              0);


    while(!noc_done(remote));
    unsigned n = (unsigned)(*lock)->remote_number + 1;
    (*lock)->local_number = n;
    /* Enforce memory barrier */
    noc_send(remote,
              (void _SPM *)&((*lock)->remote_ptr->remote_number),
              (void _SPM *)&(*lock)->local_number,
              sizeof((*lock)->local_number),
              0);

  
//    noc_send(remote,
//              (void _SPM *)&((*lock)->remote_ptr->remote_entering),
//              (void _SPM *)&(*lock)->local_entering,
//              sizeof((*lock)->local_entering)+sizeof((*lock)->local_number),
//                0);

//    /* Enforce memory barrier */
    while(!noc_done(remote)); // noc_send() also waits for the dma to be
                                // free, so no need to do it here as well

    /* Write Entering false */
    (*lock)->local_entering = 0;
    noc_send(remote,
              (void _SPM *)&((*lock)->remote_ptr->remote_entering),
              (void _SPM *)&(*lock)->local_entering,
              sizeof((*lock)->local_entering),
              0);

    /* Wait for remote core not to change number */
    while((*lock)->remote_entering == 1);
    /* Wait to be the first in line to the bakery queue */
    unsigned m = (*lock)->remote_number;
    while( (m != 0) &&
            ( (m < n) || ((m == n) && ( remote < id)))) {
      m = (*lock)->remote_number;
    }
    /* Lock is grabbed */  
    return;
  #endif
}

void release_lock(LOCK_T * lock)  __attribute__((section(".text.spm"))) {
  #if LOCKING == SHM_LOCK
    __lock_release(*lock);
  #elif LOCKING == SPM_LOCK
    /* Write Number */
    (*lock)->local_number = 0;
    noc_send((*lock)->remote_cpuid,
              (void _SPM *)&((*lock)->remote_ptr->remote_number),
              (void _SPM *)&(*lock)->local_number,
              sizeof((*lock)->local_number),
              0);
    /* Enforce memory barrier */
    while(!noc_done((*lock)->remote_cpuid));
    /* Lock is freed */  
    return;
  #endif
}

void close_lock(LOCK_T * lock) {
  #if LOCKING == SHM_LOCK
    if (get_cpuid() == 0) {
      __lock_close(*lock);
    }
    
  #elif LOCKING == SPM_LOCK

  #endif
}

void func_worker_1(void* arg) {
  struct conf_param_t conf_param;
  conf_param.tt_time = ((struct conf_param_t*)arg)->tt_time;
  conf_param.read_buf_ptr = ((struct conf_param_t*)arg)->read_buf_ptr;

  unsigned short int tt_slot = TT_SCHED_LENGHT;

  #if DATA_PLACEMENT == SPM
    conf_param.write_buf_ptr = (BUFFER_T _SPM *)mp_alloc(BUFFER_SIZE*sizeof(BUFFER_T));
    ((struct conf_param_t*)arg)->write_buf_ptr = conf_param.write_buf_ptr;
  #endif
  #if LOCKING == SPM_LOCK
    LOCK_T lock;
  #endif

  #if PARADIGME == TIME_TRIGGERED  
    next_tick(&conf_param.tt_time,&tt_slot);       // Wait for initial time tick
    // TT_slot = 0
  #elif PARADIGME == EVENT_DRIVEN
    initialize_lock(&lock);
  #endif

  for (int i = 0; i < ITERATIONS; ++i) {
    #if PARADIGME == TIME_TRIGGERED
    #elif PARADIGME == EVENT_DRIVEN
      acquire_lock(&lock);
    #endif
      shared_phase = shared_phase ^ 0x1;
      write_buffer(&shared_phase,&conf_param);
    #if PARADIGME == TIME_TRIGGERED
      next_tick(&conf_param.tt_time,&tt_slot);
      next_tick(&conf_param.tt_time,&tt_slot);
    #elif PARADIGME == EVENT_DRIVEN
      release_lock(&lock);
    #endif
  }
  
  int ret = 0;
  corethread_exit(&ret);
  return;
}

int main() {
  struct conf_param_t conf_param;
  #if DATA_PLACEMENT == SHM
    for (int i = 0; i < BUFFER_SIZE; ++i) {
      buf[i] = i;
    }
  #elif  DATA_PLACEMENT == SPM
    conf_param.read_buf_ptr = (BUFFER_T _SPM *)mp_alloc(BUFFER_SIZE*sizeof(BUFFER_T));
    for (int i = 0; i < BUFFER_SIZE; ++i) {
      *(conf_param.read_buf_ptr + i) = i;
    }
  #endif
  #if  LOCKING == SPM_LOCK
    LOCK_T lock;
  #endif

  print_version();
  corethread_t worker_1 = SLAVE_CORE; // For now the core ID
  unsigned short int tt_slot = TT_SCHED_LENGHT;
  unsigned long long int tt_time = get_cpu_usecs() + 2000; // Get time now plus some time for initialization
  conf_param.tt_time = tt_time;
     
  corethread_create(&worker_1,&func_worker_1,(void*)&conf_param);
  puts("Corethread created");

  unsigned short int local_phase = 0;

  #if PARADIGME == TIME_TRIGGERED
    next_tick(&tt_time,&tt_slot);       // Wait for initial time tick
  #elif PARADIGME == EVENT_DRIVEN
    initialize_lock(&lock);
    min_time = ULONG_MAX;
    max_time = 0;
    accum_time = 0;
    cnt_time = 0;
  #endif

  unsigned long long int start = 0;
  unsigned long long int stop = 0;

  for (int i = 0; i < ITERATIONS; ++i) {
    #if PARADIGME == TIME_TRIGGERED
      next_tick(&tt_time,&tt_slot);
      inval_dcache();
      local_phase = local_phase ^ 0x1;
      read_buffer(&local_phase,&conf_param);
      next_tick(&tt_time,&tt_slot);
    #elif PARADIGME == EVENT_DRIVEN
      #ifndef EXCLUDE_LOCK
      start = get_cpu_usecs();
      #endif
      acquire_lock(&lock);
      #ifdef EXCLUDE_LOCK
      start = get_cpu_usecs();
      #endif
      inval_dcache();
      local_phase += shared_phase;
      read_buffer(&shared_phase,&conf_param);

      #ifdef EXCLUDE_LOCK
      stop = get_cpu_usecs();
      #endif
      release_lock(&lock);
      #ifndef EXCLUDE_LOCK
      stop = get_cpu_usecs();
      #endif
      unsigned long long int exe_time = stop - start;
      min_time = (exe_time < min_time) ? exe_time : min_time;
      max_time = (exe_time > max_time) ? exe_time : max_time;
      accum_time += exe_time;
      cnt_time++;
    #endif
  }

  printf("Local phase: %d\n",local_phase);
  
  inval_dcache();
  if (slave_error == 0) {
    puts("Test successfully completed");
  } else {
    printf("Completed test with slave errors: %d\n",slave_error);
  }

  int* res;
  corethread_join(worker_1,&res);

  #if PARADIGME == TIME_TRIGGERED
    printf("Min time left: %llu\tMax time left: %llu\tAccumulated time left: %llu\nCount time left: %llu\tAverage time left: %llu\n", min_time,max_time,accum_time,cnt_time,accum_time/cnt_time);
  #elif PARADIGME == EVENT_DRIVEN
    printf("Min time left: %llu\tMax time left: %llu\tAccumulated time left: %llu\nCount time left: %llu\tAverage time left: %llu\n", min_time,max_time,accum_time,cnt_time,accum_time/cnt_time);
  #endif

  close_lock(&lock);

  puts("Corethread joined");

  return *res;  
}

