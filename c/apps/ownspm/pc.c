/*
    This is a multithread producer-consumer application for shared SPMs with ownership.

    Author: Oktay Baris
            Torur Biskopsto Strom
    Copyright: DTU, BSD License
*/

#ifndef _SSPM
#ifndef _MULTIOWN
#ifndef _OWN
#ifndef _MAINMEM
#define _MAINMEM
#endif
#endif
#endif
#endif


#include <stdio.h>
#include <machine/patmos.h>
#include <machine/spm.h>

#include "libcorethread/corethread.h"

#include "ownspm.h"

// Measure execution time with the clock cycle timer
volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
volatile _UNCACHED int start=0;
volatile _UNCACHED int stop=0;
volatile _UNCACHED int timeStamps[4]={0};

//flags
#ifdef _SSPM
#define _NAME "sspm"
volatile int _SPM *data_ready1;
volatile int _SPM *data_ready2;
#endif
#ifdef _MULTIOWN
#define _NAME "multiown"
#include "spmpool.h"
volatile int _SPM *data_ready1;
volatile int _SPM *data_ready2;
#endif
#ifdef _OWN
#define _NAME "own"
volatile _UNCACHED int data_ready1_val;
volatile _UNCACHED int *data_ready1 = &data_ready1_val;
volatile _UNCACHED int data_ready2_val;
volatile _UNCACHED int *data_ready2 = &data_ready2_val;
#endif
#ifdef _MAINMEM
#define _NAME "mainmem"
volatile _UNCACHED int data_ready1_val;
volatile _UNCACHED int *data_ready1 = &data_ready1_val;
volatile _UNCACHED int data_ready2_val;
volatile _UNCACHED int *data_ready2 = &data_ready2_val;
#endif



#ifdef _MAINMEM
//data
int data[BUFFER_SIZE*2]={0};
#endif

// Producer
void producer() {

  int id = get_cpuid();

#ifdef _SSPM
  volatile int _SPM  *buffer1_ptr = &spm_ptr[0];
  volatile int _SPM  *buffer2_ptr = &spm_ptr[BUFFER_SIZE];
#endif
#ifdef _MULTIOWN
  volatile int _SPM  *buffer1_ptr = spm_base(0);
  volatile int _SPM  *buffer2_ptr = spm_base(1);
#endif
#ifdef _OWN
  volatile int _SPM  *buffer1_ptr = &spm_ptr[NEXT*id];
  volatile int _SPM  *buffer2_ptr = &spm_ptr[NEXT*(id+1)];
#endif
#ifdef _MAINMEM
  volatile int *buffer1_ptr = &data[0];
  volatile int *buffer2_ptr = &data[BUFFER_SIZE];
#endif



  int i=0;

  while(i < DATA_LEN){

    while(*data_ready1 == 1) {
      ;
    }

    //Producer starting time stamp
    if(i==0){timeStamps[0] = *timer_ptr;}

#ifdef _MAINMEM
    inval_dcache(); //invalidate the data cache
#endif

    //producing data for the buffer 1
    for ( int j = 0; j < BUFFER_SIZE; j++ ) {
        *(buffer1_ptr+j) = 1 ; // produce data
    }

    *data_ready1 = 1;
    i += BUFFER_SIZE;

    while(*data_ready2 == 1) {
      ;
    }

#ifdef _MAINMEM
    inval_dcache(); //invalidate the data cache
#endif

    //producing data for the buffer 2
    for ( int j = 0; j < BUFFER_SIZE; j++ ) {
      *(buffer2_ptr+j) = 2 ; // produce data
    }

    *data_ready2 = 1;
    i += BUFFER_SIZE;
  }

  //Producer finishing time stamp
  timeStamps[1] = *timer_ptr;

  return;
}



// Consumer
void consumer(void *arg) {

  int id = get_cpuid();

#ifdef _SSPM
  volatile int _SPM  *buffer1_ptr = &spm_ptr[0];
  volatile int _SPM  *buffer2_ptr = &spm_ptr[BUFFER_SIZE];
#endif
#ifdef _MULTIOWN
  volatile int _SPM  *buffer1_ptr = spm_base(0);
  volatile int _SPM  *buffer2_ptr = spm_base(1);
#endif
#ifdef _OWN
  volatile int _SPM  *buffer1_ptr = &spm_ptr[NEXT*(id-1)];
  volatile int _SPM  *buffer2_ptr = &spm_ptr[NEXT*id];
#endif
#ifdef _MAINMEM
  volatile int *buffer1_ptr = &data[0];
  volatile int *buffer2_ptr = &data[BUFFER_SIZE];
#endif

  int i=0; 
  int sum = 0;

  while(i < DATA_LEN){
 
    while(*data_ready1 == 0) {
      ;
    }
        
    //Consumer starting time stamp
    if(i==0){timeStamps[2] = *timer_ptr;}

#ifdef _MAINMEM
    inval_dcache(); //invalidate the data cache
#endif

    //consuming data from the buffer 1
    for ( int j = 0; j < BUFFER_SIZE; j++ ) {
      sum += *(buffer1_ptr+j);
    }

    *data_ready1 = 0;
    i += BUFFER_SIZE;

    while(*data_ready2 == 0) {
      ;
    }

#ifdef _MAINMEM
    inval_dcache(); //invalidate the data cache
#endif

    //consuming data from the buffer 2
    for (int j = 0; j < BUFFER_SIZE; j++ ) {
      sum += *(buffer2_ptr+j);
    }
    *data_ready2 = 0;
    i += BUFFER_SIZE;

  }
   //Consumer finishing time stamp
   timeStamps[3] = *timer_ptr;
   return;
}



int main() {

#ifdef _SSPM
  data_ready1 = &spm_ptr[2*BUFFER_SIZE];
  data_ready2 = &spm_ptr[2*BUFFER_SIZE+1];
#endif
#ifdef _MULTIOWN
  // We statically assign the SPMs so we simply set the ownership
  spm_sched_wr(0,3);
  spm_sched_wr(1,3);
  spm_sched_wr(2,3);
  // Flags are put into a 3rd SPM instead of main memory
  data_ready1 = spm_base(2);
  data_ready2 = spm_base(2)+1;
#endif

  *data_ready1=0;
  *data_ready2=0;

  int id = get_cpuid(); 
  int cnt = get_cpucnt();



  printf("Total %d Cores\n",cnt); // print core count

  // printf("Writing the data to the SPM ...\n"); 

  corethread_create(1, &consumer, NULL); 
  producer();

  void * dummy;  
  corethread_join(1,&dummy);

  // printf("Computation is Done !!\n");

  //Debug

  // printf("The Producer starts at %d \n", timeStamps[0]);
  // printf("The Producer finishes at %d \n", timeStamps[1]);
  //printf("The Consumer starts at %d \n", timeStamps[2]);
  // printf("The Consumer finishes at %d \n", timeStamps[3]);
  printf("End-to-End Latency is %d clock cycles\n    \
         for %d words of bulk data\n   \
         and %d of buffer size\n", timeStamps[3]-timeStamps[0],DATA_LEN,BUFFER_SIZE);
  int cycles = timeStamps[3]-timeStamps[0];
  printf("measure pc_"_NAME": %d.%d cycles per word for %d words in %d words buffer\n",
    cycles/DATA_LEN, cycles*10/DATA_LEN%10, DATA_LEN, BUFFER_SIZE);


/*
//Debug : screen output data
  for (int i=0; i<DATA_LEN*2; ++i) {
        printf("The Output Data %d is %d \n",i, spm_ptr[i]);
   }
  */

}
