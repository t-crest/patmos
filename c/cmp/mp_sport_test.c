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
#include "libcorethread/corethread.h"
#include "libmp/mp.h"

#define TT_PERIOD_US    200
#define TT_SCHED_LENGHT 16
#define TIME_TRIGGERED  0
#define EVENT_DRIVEN    1
#ifndef PARADIGME
  #define PARADIGME     TIME_TRIGGERED
#endif

#define SHM             0
#define SPM             1
#ifndef BUFFERING
  #define BUFFERING     SHM
#endif

#define BUFFER_SIZE     64

#define SLAVE_CORE 2
#define ITERATIONS 1000

#if BUFFERING == SHM
  typedef unsigned char BUFFER_T;
  typedef _LOCK_T LOCK_T;
  LOCK_T lock;
  BUFFER_T buf[BUFFER_SIZE];
#elif BUFFERING == SPM
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
  //typedef unsigned char _SPM BUFFER_T;
  typedef struct _SPM_LOCK_T LOCK_T_S;
  typedef LOCK_T_S _SPM * LOCK_T;

  volatile LOCK_T_S* volatile master_lock_ptr = NULL;
  volatile LOCK_T_S* volatile slave_lock_ptr = NULL;
#endif

struct conf_param_t {
  unsigned long long int tt_time;
  unsigned char _SPM * read_buf_ptr;
  unsigned char _SPM * write_buf_ptr;
};


volatile unsigned short int shared_phase = 0;
volatile int slave_error = 0;
volatile int slave_done = 0;

volatile int _UNCACHED master_locked = 0;
volatile int _UNCACHED slave_locked = 0;

void print_version() {
  #if PARADIGME == TIME_TRIGGERED
    printf("PARADIGME:\tTIME_TRIGGERED\n");
  #elif PARADIGME == EVENT_DRIVEN
    printf("PARADIGME:\tEVENT_DRIVEN\n");
  #else
    printf("PARADIGME:\tUNSUPPORTED\n");
    exit(1);
  #endif
  #if BUFFERING == SHM
    printf("BUFFERING:\tSHM\n");
    return;
  #elif BUFFERING == SPM
    printf("BUFFERING:\tSPM\n");
    return;
  #else
    printf("BUFFERING:\tUNSUPPORTED\n");
    exit(1);
  #endif
}

void next_tick(unsigned long long int *tt_time, unsigned short int *tt_slot) {
  unsigned long long int nxt_tt_time = *tt_time + TT_PERIOD_US;
  if (nxt_tt_time < get_cpu_usecs()) {
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
  while(nxt_tt_time >= get_cpu_usecs());
  return;
}

void read_buffer(volatile unsigned short int * phase, struct conf_param_t * conf_param) {
  int error = 0;
  //#if BUFFERING == SHM
    inval_dcache();
  //#endif
  unsigned char mem;
  int shm_phase = shared_phase;
  int i;
  for (i = 0; i < BUFFER_SIZE; ++i) {
    #if BUFFERING == SHM
      mem = buf[i];
    #elif BUFFERING == SPM
      mem = *((conf_param->read_buf_ptr) + i);
      //printf("mem: %d\tphase: %d\ti: %d\n",mem,*phase\,i);
    #endif
    if (*phase == 0 && mem != i) {
      error++;
    } else if (*phase != 0 && mem != (unsigned char)(BUFFER_SIZE-1 - i)) {
      error++;
    }

    if (error != 0) {
      if (get_cpuid() == 0) {\
        inval_dcache();
        puts("Reading wrong values");
        printf("errors: %d\tmem: %d\tphase: %d\tshared phase: %d\ti: %d\n",error,mem,*phase,shm_phase,i);
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

void write_buffer(volatile unsigned short int * phase, struct conf_param_t * conf_param) {
  #if BUFFERING == SHM
    if (*phase == 0) {
      for (int i = 0; i < BUFFER_SIZE; ++i) {
        buf[i] = (unsigned char)i;
      }
    } else {
      for (int i = 0; i < BUFFER_SIZE; ++i) {
        buf[i] = (unsigned char)(BUFFER_SIZE-1 - i);
      }
    }

  #elif BUFFERING == SPM
    if (*phase == 0) {
      for (int i = 0; i < BUFFER_SIZE; ++i) {
        *((conf_param->write_buf_ptr) + i) = (unsigned char)i;
      }
    } else {
      for (int i = 0; i < BUFFER_SIZE; ++i) {
        *((conf_param->write_buf_ptr) + i) = (unsigned char)(BUFFER_SIZE-1 - i);
      }
    }
    noc_send(0,conf_param->read_buf_ptr,conf_param->write_buf_ptr,BUFFER_SIZE);

    while(!noc_done(0));

  #endif
}

void initialize_lock(LOCK_T * lock) {
  #if BUFFERING == SHM
    if(get_cpuid() == 0) {
      __lock_init(*lock);
    }
  #elif BUFFERING == SPM
    *lock = (LOCK_T_S _SPM *)mp_alloc(sizeof(LOCK_T_S));
    (*lock)->remote_entering = 0;
    (*lock)->remote_number = 0;
    (*lock)->local_entering = 0;
    (*lock)->local_number = 0;
    if(get_cpuid() == 0) {
      master_lock_ptr = (LOCK_T_S *)*lock;
      while(slave_lock_ptr == NULL) {
        inval_dcache();
      }

      (*lock)->remote_ptr = (LOCK_T_S _SPM *)slave_lock_ptr;
      (*lock)->remote_cpuid = SLAVE_CORE;
    } else {
      slave_lock_ptr = (LOCK_T_S *)*lock;
      while(master_lock_ptr == NULL) {
        inval_dcache();
      }
      
      (*lock)->remote_ptr = (LOCK_T_S _SPM *)master_lock_ptr;
      (*lock)->remote_cpuid = 0;
    }
  #endif
}

void acquire_lock(LOCK_T * lock){
  #if BUFFERING == SHM
    __lock_acquire(*lock);
  #elif BUFFERING == SPM
    //offsetof(LOCK_T_S,entering);
    //offsetof(LOCK_T_S,number);
    /* Write Entering true */
    /* Write Number */
    unsigned remote = (*lock)->remote_cpuid;
    unsigned id = get_cpuid();
    (*lock)->local_entering = 1;
    noc_send(remote,
              (void _SPM *)&((*lock)->remote_ptr->remote_entering),
              (void _SPM *)&(*lock)->local_entering,
              sizeof((*lock)->local_entering));

    /* Enforce memory barrier */
//    while(!noc_done(remote));
    unsigned n = (unsigned)(*lock)->remote_number + 1;
    (*lock)->local_number = n;
    noc_send(remote,
              (void _SPM *)&((*lock)->remote_ptr->remote_number),
              (void _SPM *)&(*lock)->local_number,
              sizeof((*lock)->local_number));

    /* Enforce memory barrier */
    while(!noc_done(remote));

    /* Write Entering false */
    (*lock)->local_entering = 0;
    noc_send(remote,
              (void _SPM *)&((*lock)->remote_ptr->remote_entering),
              (void _SPM *)&(*lock)->local_entering,
              sizeof((*lock)->local_entering));
    /* Wait for remote core not to change number */
    while((*lock)->remote_entering == 1);
    /* Wait to be the first in line to the bakery queue */
    unsigned m = (*lock)->remote_number;
//    unsigned wait_time = 0;
    while( (m != 0) &&
            ( (m < n) || ((m == n) && ( remote < id)))) {
      m = (*lock)->remote_number;
//      wait_time++;
    }
//    inval_dcache();
//    if (id == 0) {
//      master_locked = 1;
//      if (slave_locked == 1){
//        printf("Error wrong acquire!!!!, wait time: %d, remote number: %d, number: %d\n",wait_time,m,n);
//        printf("Addresses lock: %#08x, local_number: %#08x, local_entering: %#08x\n",
//              lock,
//              &((*lock)->local_number),
//              &((*lock)->local_entering));
//        printf("Addresses lock: %#08x, remote_number: %#08x, remote_entering: %#08x\n",
//              (*lock)->remote_ptr,
//              &((*lock)->remote_ptr->remote_number),
//              &((*lock)->remote_ptr->remote_entering));
//      }
//    } else {
//      slave_locked = 1;
//      if (master_locked == 1){
//        slave_error++;
//        //exit(1);
//      }
//    }
    /* Lock is grabbed */  
    return;
  #endif
}

void release_lock(LOCK_T * lock){
  #if BUFFERING == SHM
    __lock_release(*lock);
  #elif BUFFERING == SPM
    //offsetof(LOCK_T_S,entering);
    //offsetof(LOCK_T_S,number);
    /* Write Number */
//    inval_dcache();
//    if (get_cpuid() == 0) {
//      master_locked = 0;
//      if (slave_locked == 1){
//        puts("Error wrong release!!!!");
//      }
//    } else {
//      slave_locked = 0;
//      if (master_locked == 1){
//        slave_error++;
//        //exit(1);
//      }
//    }
    (*lock)->local_number = 0;
    noc_send((*lock)->remote_cpuid,
              (void _SPM *)&((*lock)->remote_ptr->remote_number),
              (void _SPM *)&(*lock)->local_number,
              sizeof((*lock)->local_number));
    /* Enforce memory barrier */
    while(!noc_done((*lock)->remote_cpuid));
    /* Lock is freed */  
    return;
  #endif
}

void close_lock(LOCK_T * lock) {
  #if BUFFERING == SHM
    if (get_cpuid() == 0) {
      __lock_close(*lock);
    }
    
  #elif BUFFERING == SPM

  #endif
}

void func_worker_1(void* arg) {
  struct conf_param_t conf_param;
  conf_param.tt_time = ((struct conf_param_t*)arg)->tt_time;
  conf_param.read_buf_ptr = ((struct conf_param_t*)arg)->read_buf_ptr;

  unsigned short int tt_slot = TT_SCHED_LENGHT;

  #if BUFFERING == SPM
    LOCK_T lock;
    conf_param.write_buf_ptr = (unsigned char _SPM *)mp_alloc(BUFFER_SIZE);
    ((struct conf_param_t*)arg)->write_buf_ptr = conf_param.write_buf_ptr;
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
      inval_dcache();
      if (shared_phase == 0) {
        shared_phase = 1;
      } else {
        shared_phase = 0;
      }
      //shared_phase = shared_phase ^ 0x1;
      write_buffer(&shared_phase,&conf_param);
      inval_dcache();
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
  #if BUFFERING == SHM
    for (int i = 0; i < BUFFER_SIZE; ++i) {
      buf[i] = i;
    }
  #elif  BUFFERING == SPM
    conf_param.read_buf_ptr = (unsigned char _SPM *)mp_alloc(BUFFER_SIZE);
    LOCK_T lock;
    for (int i = 0; i < BUFFER_SIZE; ++i) {
      *(conf_param.read_buf_ptr + i) = i;
    }
  #endif

  print_version();
  corethread_t worker_1 = SLAVE_CORE; // For now the core ID
  unsigned short int tt_slot = TT_SCHED_LENGHT;
  unsigned long long int tt_time = get_cpu_usecs() + 2000; // Get time now plus some time for initialization
  conf_param.tt_time = tt_time;
     
  corethread_create(&worker_1,&func_worker_1,(void*)&conf_param);
  puts("Corethread created");

  volatile unsigned short int local_phase = 0;

  #if PARADIGME == TIME_TRIGGERED
    next_tick(&tt_time,&tt_slot);       // Wait for initial time tick
    //puts("Time tick");
    // TT_slot = 0
  #elif PARADIGME == EVENT_DRIVEN
    initialize_lock(&lock);
    puts("Lock initialized");
  #endif

  for (int i = 0; i < ITERATIONS; ++i) {
    
    #if PARADIGME == TIME_TRIGGERED
      //printf("Iteration count: %d\n",i );
      next_tick(&tt_time,&tt_slot);
      inval_dcache();
      if (local_phase == 0) {
        local_phase = 1;
      } else {
        local_phase = 0;
      }
      //local_phase = local_phase ^ 0x1;
      read_buffer(&local_phase,&conf_param);
      next_tick(&tt_time,&tt_slot);
    #elif PARADIGME == EVENT_DRIVEN
      acquire_lock(&lock);
      //printf("Iteration count: %d\n",i );
      inval_dcache();
      local_phase += shared_phase;
      read_buffer(&shared_phase,&conf_param);
      release_lock(&lock);
    #endif
  }

  printf("Local phase: %d\n",local_phase);
  
  inval_dcache();
  if (slave_error == 0) {
    puts("Test successfully completed");
  } else {
    printf("Completed test with slave errors: %d\n",slave_error);
  }

//  while(slave_done == 0){
//    inval_dcache();
//  }
//  puts("slave_done");
  int* res;
  corethread_join(worker_1,&res);

  close_lock(&lock);

  puts("Corethread joined");

  return *res;  
}

