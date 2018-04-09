/*
    This is a multithread producer-consumer data-flow application for shared SPMs with ownership.

    Author: Oktay Baris
    Copyright: DTU, BSD License
*/

#include <stdio.h>
#include <machine/patmos.h>
#include <machine/spm.h>

#include "libcorethread/corethread.h"

#include "ownspm.h"


// Measure execution time with the clock cycle timer
volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
volatile _UNCACHED int timeStamps[4];


volatile _UNCACHED int status0;
volatile _UNCACHED int status1;
volatile _UNCACHED int status2;
volatile _UNCACHED int status3;


// Producer
void producer() {

  volatile int _SPM  *outbuffer1_ptr;
  volatile int _SPM  *outbuffer2_ptr;

  int i=0; 

  while(i<DATA_LEN/BUFFER_SIZE){

    outbuffer1_ptr = &spm_ptr[0];
    outbuffer2_ptr = &spm_ptr[NEXT];

    while(status0 != 0) {
      ;
    }

           //Producer starting time stamp
          if(i==0){timeStamps[0] = *timer_ptr;}

          //producing data for the buffer 1
          for ( int j = 0; j < BUFFER_SIZE; j++ ) {
              *outbuffer1_ptr++ = 1 ; // produce data
          }

          // flip the data ready flag for buffer 1
          status0 = 1;
          i++;

    while(status1 != 0) {
      ;
    }

          //producing data for the buffer 2
          for ( int j = 0; j < BUFFER_SIZE; j++ ) {
              *outbuffer2_ptr++ = 2 ; // produce data
          }

          // flip the data ready flag for buffer 2
          status1 = 1;
          i++;        
  }
//producer finishing time stamp
  timeStamps[1] = *timer_ptr;
  return;
}



// intermediate
void intermediate(void *arg){

    // buffer pointers
    volatile int _SPM  *inbuffer1_ptr;
    volatile int _SPM  *inbuffer2_ptr;
    volatile int _SPM  *outbuffer1_ptr;
    volatile int _SPM  *outbuffer2_ptr;

    int i=0; 

    while(i<DATA_LEN/BUFFER_SIZE){

        inbuffer1_ptr = &spm_ptr[0];
        inbuffer2_ptr = &spm_ptr[NEXT];
        outbuffer1_ptr = &spm_ptr[NEXT*2];
        outbuffer2_ptr = &spm_ptr[NEXT*3];
 
      while (!((status0 == 1) && (status2 == 0))) {
      ;
      }
            //producing data for the buffer 1
            for ( int j = 0; j < BUFFER_SIZE; j++ ) {
                *outbuffer1_ptr++ = *inbuffer1_ptr++ +1 ; // produce data
            }

            // update the flags for buffer 1
            status0 = 0;
            status2 = 1;
            //for the time being for flow control
            i++;
      while (!((status1 == 1) && (status3 == 0))) {
      ;
      }      
            //producing data for the buffer 2
            for ( int j = 0; j < BUFFER_SIZE; j++ ) {
                *outbuffer2_ptr++ = *inbuffer2_ptr++ +2 ; // produce data
            }

            // update the flags for buffer 2
            status1 = 0;
            status3 = 1;
            //for the time being for flow control
            i++;
      }

}


// Consumer
void consumer(void *arg) {

  volatile int _SPM  *output_ptr, *inbuffer1_ptr,*inbuffer2_ptr;

  int i=0; 
  int sum=0;
  while(i<DATA_LEN/BUFFER_SIZE) {

    inbuffer1_ptr = &spm_ptr[NEXT*2];
    inbuffer2_ptr = &spm_ptr[NEXT*3];

    while (status2 != 1) {
      ;
    }


        //Consumer starting time stamp
        if(i==0){
            timeStamps[2] = *timer_ptr;
            }

        //consuming data from the buffer 1
        for ( int j = 0; j < BUFFER_SIZE; j++ ) {
            sum += (*inbuffer1_ptr++);
        }

        status2 = 0;
        i++; 
        
    while (status3 != 1) {
      ;
    }

        //consuming data from the buffer 2
        for ( int j = 0; j < BUFFER_SIZE; j++ ) {
            sum += (*inbuffer2_ptr++);
        }
        status3 = 0;
        i++; 
      
  }
   //Consumer finishing time stamp
   timeStamps[3] = *timer_ptr;

}



int main() {

  unsigned i,j;

  int id = get_cpuid(); // id=0

  status0 = 0;
  status1 = 0;
  status2 = 0;
  status3 = 0;

  int *parameter;

  printf("Total %d Cores\n",get_cpucnt()); // print core count

  corethread_create(1, &intermediate, NULL);
  corethread_create(2, &consumer, NULL);

  producer();
    
  corethread_join(1, (void **) &parameter);
  corethread_join(2, (void **) &parameter);

  //Debug

  // printf("The Producer starts at %d \n", timeStamps[0]);
  // printf("The Producer finishes at %d \n", timeStamps[1]);
  // printf("The Consumer starts at %d \n", timeStamps[2]);
  // printf("The Consumer finishes at %d \n", timeStamps[3]); 
  printf("End-to-End Latency is %d clock cycles\n    \
         for %d words of bulk data\n   \
         and %d of buffer size\n", timeStamps[3]-timeStamps[0],DATA_LEN,BUFFER_SIZE);
  int cycles = timeStamps[3]-timeStamps[0];
  printf("measure pc_own_df: %d.%d cycles per word for %d words in %d words buffer\n",
    cycles/DATA_LEN, cycles*10/DATA_LEN%10, DATA_LEN, BUFFER_SIZE);

/* //Debug : screen output data
  for (int i=0; i<BUFFER_SIZE*2*(cnt-1)+DATA_LEN; ++i) {
        printf("The Output Data %d is %d \n",i, spm_ptr[i]);
   }
  */
}
