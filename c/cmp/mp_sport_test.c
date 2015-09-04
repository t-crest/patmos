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

#define TT_PERIOD_US    1000
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

#if BUFFERING == SHM
  typedef unsigned char BUFFER_T;
#elif BUFFERING == SPM
  typedef unsigned char _SPM BUFFER_T;
#endif

struct conf_param_t {
  unsigned long long int tt_time;
  unsigned char _SPM * read_buf_ptr;
  unsigned char _SPM * write_buf_ptr;
};

BUFFER_T buf[BUFFER_SIZE];
unsigned short int shared_phase = 0;
_LOCK_T lock;
int slave_error = 0;

void wait_for_tt_tick(unsigned long long int *tt_time, unsigned short int *tt_slot) {
  unsigned long long int nxt_tt_time = *tt_time + TT_PERIOD_US;
  if (nxt_tt_time < get_cpu_usecs()) {
    // abort, time period has been exceeded.
    if (get_cpuid() == 0) {
      puts("TT period time exceeded");
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

void read_buffer(unsigned short int phase, struct conf_param_t * conf_param) {
  #if BUFFERING == SHM || BUFFERING == SPM
    int error = 0;
    //#if BUFFERING == SHM
      inval_dcache();
    //#endif
    unsigned char mem;
    for (int i = 0; i < sizeof(buf); ++i) {
      #if BUFFERING == SHM
        mem = buf[i];
      #elif BUFFERING == SPM
        mem = *((conf_param->read_buf_ptr) + i);
      #endif
      if (phase == 0 && mem != i) {
        error++;
      } else if (phase != 0 && mem != (unsigned char)(sizeof(buf) - i)) {
        error++;
      }

      if (error != 0) {
        if (get_cpuid() == 0) {\
          inval_dcache();
          puts("Reading wrong values");
          printf("mem: %d\tphase: %d\tsizeof(buf) - i: %d\n",mem,phase,sizeof(buf) - i);
          printf("write_buf_ptr: %#08x\tread_buf_ptr: %#08x\n",conf_param->write_buf_ptr,conf_param->read_buf_ptr);
        } else {
          slave_error++;
        }
        exit(1);
      }
    }

  #else
    #error Unsupported Buffering type
  #endif
}

void write_buffer(unsigned short int phase, struct conf_param_t * conf_param) {
  #if BUFFERING == SHM
    if (phase == 0) {
      for (int i = sizeof(buf)-1; i >= 0 ; --i) {
        buf[i] = (unsigned char)(sizeof(buf) - i);
      }
    } else {
      for (int i = sizeof(buf)-1; i >= 0 ; --i) {
        buf[i] = (unsigned char)i;
      }
    }

  #elif BUFFERING == SPM
    if (phase == 0) {
      for (int i = 0; i < sizeof(buf); ++i) {
        *((conf_param->read_buf_ptr) + i) = (unsigned char)(sizeof(buf) - i);
      }
    } else {
      for (int i = 0; i < sizeof(buf); ++i) {
        *((conf_param->read_buf_ptr) + i) = (unsigned char)i;
      }
    }
    noc_send(0,conf_param->read_buf_ptr,conf_param->write_buf_ptr,BUFFER_SIZE);

    while(!noc_done(0));

  #else
    #error Unsupported Buffering type
  #endif
}

void aquire_lock(){
  #if BUFFERING == SHM
  #elif BUFFERING == SPM
  #else
    #error Unsupported Buffering type
  #endif
}

void release_lock(){

}

void func_worker_1(void* arg) {
  struct conf_param_t conf_param;
  conf_param.tt_time = ((struct conf_param_t*)arg)->tt_time;
  conf_param.read_buf_ptr = ((struct conf_param_t*)arg)->read_buf_ptr;

  unsigned short int tt_slot = TT_SCHED_LENGHT;

  #if BUFFERING == SPM
    conf_param.write_buf_ptr = (unsigned char _SPM *)mp_alloc(BUFFER_SIZE);
    ((struct conf_param_t*)arg)->write_buf_ptr = conf_param.write_buf_ptr;
  #endif

  #if PARADIGME == TIME_TRIGGERED
    
    wait_for_tt_tick(&conf_param.tt_time,&tt_slot);       // Wait for initial time tick
    // TT_slot = 0
    for (int i = 0; i < 1000; ++i) {
      write_buffer(shared_phase,&conf_param);
      if (shared_phase == 1) {
        shared_phase = 0;
      } else {
        shared_phase = 1;
      }
      wait_for_tt_tick(&conf_param.tt_time,&tt_slot);
      wait_for_tt_tick(&conf_param.tt_time,&tt_slot);
    }


  #elif PARADIGME == EVENT_DRIVEN
    #if BUFFERING == SHM
      for (int i = 0; i < 1000; ++i) {
          __lock_acquire(lock);
          write_buffer(shared_phase,&conf_param);
          if (shared_phase == 1) {
            shared_phase = 0;
          } else {
            shared_phase = 1;
          }
          __lock_release(lock);
      }
      
    #elif BUFFERING == SPM
      for (int i = 0; i < 1000; ++i) {
          __lock_acquire(lock);
          write_buffer(shared_phase,&conf_param);
          if (shared_phase == 1) {
            shared_phase = 0;
          } else {
            shared_phase = 1;
          }
          __lock_release(lock);
      }
    #else
      #error Unsupported Buffering type
    #endif
  #else
    #error Unsupported Paradigme
  #endif


  int ret = 0;
  corethread_exit(&ret);
  return;
}

int main() {
  struct conf_param_t conf_param;
  conf_param.read_buf_ptr = (unsigned char _SPM *)mp_alloc(BUFFER_SIZE);
  for (int i = 0; i < sizeof(buf); ++i) {
    buf[i] = i;
    *(conf_param.read_buf_ptr + i) = i;
  }
  __lock_init(lock);
  puts("Master");
  corethread_t worker_1 = 2; // For now the core ID
  unsigned long long int tt_time = get_cpu_usecs();
  conf_param.tt_time = tt_time;
  
  unsigned short int tt_slot = TT_SCHED_LENGHT;
  
  corethread_create(&worker_1,&func_worker_1,(void*)&conf_param);

  unsigned short int local_phase = 0;

  #if PARADIGME == TIME_TRIGGERED
    wait_for_tt_tick(&tt_time,&tt_slot);       // Wait for initial time tick
    // TT_slot = 0

    // Make time triggered version unidirectional and pass address in struct through corethread_create
    for (int i = 0; i < 1000; ++i) {
      wait_for_tt_tick(&tt_time,&tt_slot);
      local_phase += shared_phase;
      read_buffer(shared_phase,&conf_param);
      
      wait_for_tt_tick(&tt_time,&tt_slot);
    }

  #elif PARADIGME == EVENT_DRIVEN
    
    for (int i = 0; i < 1000; ++i) {
        __lock_acquire(lock);
        local_phase += shared_phase;
        read_buffer(shared_phase,&conf_param);
        __lock_release(lock);

    }
      
  #else
    #error Unsupported Paradigme
  #endif

  printf("Local phase: %d\n",local_phase);
  __lock_close(lock);
  
  if (slave_error == 0) {
    puts("Test successfully completed");
  } else {
    puts("Completed test with slave errors");
  }
  int* res;
  corethread_join(worker_1,&res);

  puts("Corethread joined");

  return *res;  
}

