/*
    This is a multithread producer-consumer application for shared SPMs with ownership.

    Author: Oktay Baris
    Copyright: DTU, BSD License
*/

#include <stdio.h>
#include <machine/patmos.h>
#include <machine/spm.h>

//#include "include/patio.h"
#include "libcorethread/corethread.h"

#include "ownspm.h"

// Measure execution time with the clock cycle timer
volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
volatile _UNCACHED int start=0;
volatile _UNCACHED int stop=0;
volatile _UNCACHED int timeStamps[4]={0};
//flags
volatile _UNCACHED int data_ready1;
volatile _UNCACHED int data_ready2;

// Producer
void producer() {


  int id = get_cpuid();

  volatile int _SPM  *buffer1_ptr = &spm_ptr[NEXT*id];
  volatile int _SPM  *buffer2_ptr = &spm_ptr[NEXT*(id+1)];


  int i=0;

  while(i < DATA_LEN){

    while(data_ready1 == 1) {
      ;
    }

    //Producer starting time stamp
    if(i==0){timeStamps[0] = *timer_ptr;}

    int len = DATA_LEN - i;
    if(BUFFER_SIZE < len)
      len = BUFFER_SIZE;

    //producing data for the buffer 1
    for ( int j = 0; j < len; j++ ) {
        *(buffer1_ptr+j) = 1 ; // produce data
    }

    data_ready1 = 1;
    i += len;

    while(data_ready2 == 1) {
      ;
    }

    len = DATA_LEN - i;
    if(BUFFER_SIZE < len)
      len = BUFFER_SIZE;

    //producing data for the buffer 2
    for ( int j = 0; j < len; j++ ) {
      *(buffer2_ptr+j) = 2 ; // produce data
    }

    data_ready2 = 1;
    i += len;
  }

  //Producer finishing time stamp
  timeStamps[1] = *timer_ptr;

  return;
}



// Consumer
void consumer(void *arg) {

  int id = get_cpuid();
  
  volatile int _SPM  *buffer1_ptr = &spm_ptr[NEXT*(id-1)];
  volatile int _SPM  *buffer2_ptr = &spm_ptr[NEXT*id];


  int i=0; 
  int sum = 0;

  while(i < DATA_LEN){
 
    while(data_ready1 == 0) {
      ;
    }
        
    //Consumer starting time stamp
    if(i==0){timeStamps[2] = *timer_ptr;}

    int len = DATA_LEN - i;
    if(BUFFER_SIZE < len)
      len = BUFFER_SIZE;

    //consuming data from the buffer 1
    for ( int j = 0; j < len; j++ ) {
      sum += *(buffer1_ptr+j);
    }

    data_ready1 = 0;
    i += len;

    while(data_ready2 == 0) {
      ;
    }

    len = DATA_LEN - i;
    if(BUFFER_SIZE < len)
      len = BUFFER_SIZE;

    //consuming data from the buffer 2
    for (int j = 0; j < len; j++ ) {
      sum += *(buffer2_ptr+j);
    }
    data_ready2 = 0;
    i += len;

  }
   //Consumer finishing time stamp
   timeStamps[3] = *timer_ptr;
   return;
}



int main() {

  data_ready1=0;
  data_ready2=0;

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
  printf("measure pc_own: %d.%d cycles per word for %d words in %d words buffer\n",
    cycles/DATA_LEN, cycles*10/DATA_LEN%10, DATA_LEN, BUFFER_SIZE);


/*
//Debug : screen output data
  for (int i=0; i<DATA_LEN*2; ++i) {
        printf("The Output Data %d is %d \n",i, spm_ptr[i]);
   }
  */

}
