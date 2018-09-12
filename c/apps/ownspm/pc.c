/*
    This is a multithread producer-intermediate-consumer application for shared SPMs with ownership.

    Author: Oktay Baris
            Torur Biskopsto Strom
    Copyright: DTU, BSD License
*/

#ifndef _SSPM
#ifndef _SPMPOOL
#ifndef _OWN
#ifndef _MAINMEM
#define _MAINMEM
#endif
#endif
#endif
#endif

//#define _OWNMAINMEM

#define DEBUG


#include <stdio.h>
#include <machine/patmos.h>

#include "libcorethread/corethread.h"

#include "setup.h"


// Measure execution time with the clock cycle timer
volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
volatile _UNCACHED int timeStamps[4]={0};
volatile _UNCACHED int sum = 0;

//flags
#ifdef _SSPM
#define _NAME "sspm"
typedef volatile _IODEV int * buf_ptr_t;
typedef buf_ptr_t buf_rdy_ptr_t;
#define NEXTSPM BUFFER_SIZE
#define NEXTSPMRDY 1
#endif
#ifdef _SPMPOOL
#define _NAME "spmpool"
typedef volatile _IODEV int * buf_ptr_t;
typedef buf_ptr_t buf_rdy_ptr_t;
#define NEXTSPM SPMPOOL_NEXT
#define NEXTSPMRDY NEXTSPM
#endif
#ifdef _OWN
#define _NAME "own"
typedef volatile _IODEV int * buf_ptr_t;
#ifdef _OWNMAINMEM
typedef volatile _UNCACHED int * buf_rdy_ptr_t;
volatile _UNCACHED int buf_rdy[(MAX_CPU_CNT-1)*2]={0};
#else
typedef _IODEV int volatile * buf_rdy_ptr_t;
#endif
#define NEXTSPM NEXT
#define NEXTSPMRDY 1
#endif
#ifdef _MAINMEM
#define _NAME "mainmem"
typedef volatile int * buf_ptr_t;
typedef volatile _UNCACHED int * buf_rdy_ptr_t;
int buf[BUFFER_SIZE*(MAX_CPU_CNT-1)*2]={0};
volatile _UNCACHED int buf_rdy[(MAX_CPU_CNT-1)*2]={0};
#define NEXTSPM BUFFER_SIZE
#define NEXTSPMRDY 1
#endif


void producer(const buf_ptr_t buf_ptr, const buf_rdy_ptr_t buf_rdy_ptr) {

  buf_ptr_t buf_to_ptr;
  buf_rdy_ptr_t buf_to_rdy_ptr;

  int buf_sw = 0;

  for(int i = 0; i < DATA_LEN; i += BUFFER_SIZE){
    buf_sw = !buf_sw;
    if(buf_sw) {
      buf_to_ptr = buf_ptr+(2*NEXTSPM);
      buf_to_rdy_ptr = buf_rdy_ptr+(2*NEXTSPMRDY);
    }
    else {
      buf_to_ptr = buf_ptr+(3*NEXTSPM);
      buf_to_rdy_ptr = buf_rdy_ptr+(3*NEXTSPMRDY);
    }

    while(*buf_to_rdy_ptr == 1) {
      ;
    }

    //Producer starting time stamp
    if(i==0)
      timeStamps[0] = *timer_ptr;

#ifdef _MAINMEM
    inval_dcache(); //invalidate the data cache
#endif

    //producing data for the buffer
    for ( int j = 0; j < BUFFER_SIZE; j++ )
        *(buf_to_ptr+j) = 1 ; // produce data
      
    *buf_to_rdy_ptr = 1;
  }

  //Producer finishing time stamp
  timeStamps[1] = *timer_ptr;
}

void intermediate(const buf_ptr_t buf_ptr, const buf_rdy_ptr_t buf_rdy_ptr) {


  buf_ptr_t buf_from_ptr;
  buf_ptr_t buf_to_ptr;
  buf_rdy_ptr_t buf_from_rdy_ptr;
  buf_rdy_ptr_t buf_to_rdy_ptr;

  int buf_sw = 0;

  for(int i = 0; i < DATA_LEN; i += BUFFER_SIZE){
    buf_sw = !buf_sw;
    if(buf_sw) {
      buf_from_ptr = buf_ptr+(0*NEXTSPM);
      buf_to_ptr = buf_ptr+(2*NEXTSPM);
      buf_from_rdy_ptr = buf_rdy_ptr+(0*NEXTSPMRDY);
      buf_to_rdy_ptr = buf_rdy_ptr+(2*NEXTSPMRDY);
    }
    else {
      buf_from_ptr = buf_ptr+(1*NEXTSPM);
      buf_to_ptr = buf_ptr+(3*NEXTSPM);
      buf_from_rdy_ptr = buf_rdy_ptr+(1*NEXTSPMRDY);
      buf_to_rdy_ptr = buf_rdy_ptr+(3*NEXTSPMRDY);
    }
    
    while(*buf_from_rdy_ptr == 0) {
      ;
    }
      
    while(*buf_to_rdy_ptr == 1) {
      ;
    }

#ifdef _MAINMEM
    inval_dcache(); //invalidate the data cache
#endif

    //consuming and producing data
    for ( int j = 0; j < BUFFER_SIZE; j++ )
      *(buf_to_ptr+j) = *(buf_from_ptr+j);
    
    *buf_from_rdy_ptr = 0;
    *buf_to_rdy_ptr = 1;
  }
}

void consumer(const buf_ptr_t buf_ptr, const buf_rdy_ptr_t buf_rdy_ptr) {

  buf_ptr_t buf_from_ptr;
  buf_rdy_ptr_t buf_from_rdy_ptr;

  int _sum = 0;
  int buf_sw = 0;

  for(int i = 0; i < DATA_LEN; i += BUFFER_SIZE){
    buf_sw = !buf_sw;
    if(buf_sw) {
      buf_from_ptr = buf_ptr+(0*NEXTSPM);
      buf_from_rdy_ptr = buf_rdy_ptr+(0*NEXTSPMRDY);
    }
    else {
      buf_from_ptr = buf_ptr+(1*NEXTSPM);
      buf_from_rdy_ptr = buf_rdy_ptr+(1*NEXTSPMRDY);
    }
    
    while(*buf_from_rdy_ptr == 0) {
      ;
    }

    //Consumer starting time stamp
    if(i==0) 
      timeStamps[2] = *timer_ptr;

#ifdef _MAINMEM
    inval_dcache(); //invalidate the data cache
#endif

    for ( int j = 0; j < BUFFER_SIZE; j++ )
      _sum += *(buf_from_ptr+j);
    
      *buf_from_rdy_ptr = 0;
  }

  //Consumer finishing time stamp
  timeStamps[3] = *timer_ptr;

  sum = _sum;
}

void setup() {

  int cpuid = get_cpuid();

  int cpucnt = get_cpucnt();
  if(MAX_CPU_CNT < cpucnt)
    cpucnt = MAX_CPU_CNT;
  
  int bufid = (cpuid-1)*2;

  buf_ptr_t buf_ptr;
  buf_rdy_ptr_t buf_rdy_ptr;

#ifdef _SSPM
  buf_ptr = (buf_ptr_t)(PATMOS_IO_SPM)+(bufid*BUFFER_SIZE);
  buf_rdy_ptr = (buf_rdy_ptr_t)(PATMOS_IO_SPM)+((cpucnt*2*BUFFER_SIZE)+bufid);
#endif
#ifdef _SPMPOOL
  buf_ptr = (buf_ptr_t)spm_base(bufid);
  buf_rdy_ptr = (buf_rdy_ptr_t)(spm_base(bufid)+BUFFER_SIZE);
#endif
#ifdef _OWN
  buf_ptr = (buf_ptr_t)(PATMOS_IO_OWNSPM)+(NEXT*bufid);
#ifdef _OWNMAINMEM
  buf_rdy_ptr = &buf_rdy[bufid];
#else
  buf_rdy_ptr = (buf_rdy_ptr_t)(PATMOS_IO_SPM)+bufid;
#endif
#endif
#ifdef _MAINMEM
  buf_ptr = &buf[bufid*BUFFER_SIZE];
  buf_rdy_ptr = &buf_rdy[bufid];
#endif

  if(cpuid == 0)
    producer(buf_ptr, buf_rdy_ptr);
  else if(cpuid == cpucnt - 1)
    consumer(buf_ptr, buf_rdy_ptr);
  else
    intermediate(buf_ptr, buf_rdy_ptr);
  
  corethread_exit((void *)0);
}

int main() {
  
  int cpucnt = get_cpucnt();
  if(MAX_CPU_CNT < cpucnt)
    cpucnt = MAX_CPU_CNT;

#ifdef DEBUG
  printf("Total %d Cores\n",cpucnt); // print core count
#endif

  // Buffer initialization
  for(int i = 0; i < cpucnt*2; i++) {
#ifdef _SPMPOOL
    // We statically assign the SPMs so we simply set the ownership
    spm_sched_wr(i, (1 << (i >> 1)) + (1 << ((i >> 1)+1)));
#endif
  }

  for(int i = 1; i < cpucnt; i++)
   corethread_create(i, &setup, NULL);

  setup();

  void * dummy;
  for(int i = 1; i < cpucnt; i++)
    corethread_join(i, &dummy);

#ifdef DEBUG
  printf("The Producer starts at %d \n", timeStamps[0]);
  printf("The Producer finishes at %d \n", timeStamps[1]);
  printf("The Consumer starts at %d \n", timeStamps[2]);
  printf("The Consumer finishes at %d \n", timeStamps[3]);
  printf("End-to-End Latency is %d clock cycles\n    \
         for %d words of bulk data\n   \
         and %d of buffer size\n", timeStamps[3]-timeStamps[0],DATA_LEN,BUFFER_SIZE);
  int cycles = timeStamps[3]-timeStamps[0];
  printf(""_NAME": %d.%d cycles per word for %d words in %d words buffer\n",
    cycles/DATA_LEN, cycles*10/DATA_LEN%10, DATA_LEN, BUFFER_SIZE);

  printf("The sum is %d\n", sum);
#endif
}
