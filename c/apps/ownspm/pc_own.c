/*
    This is a multithread producer-consumer application for shared SPMs with ownership.

    Author: Oktay Baris
    Copyright: DTU, BSD License
*/
#include <machine/patmos.h>
#include <machine/spm.h>

#include "include/patio.h"
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

  volatile int _SPM  *buffer1_ptr;
  volatile int _SPM  *buffer2_ptr;

  int id = get_cpuid();

  int i=0;
  int sum=0;

  while(i<DATA_LEN/BUFFER_SIZE){

    buffer1_ptr = &spm_ptr[NEXT*id];
    buffer2_ptr = &spm_ptr[NEXT*(id+1)];

    if(data_ready1 == 0){
        
          //Producer starting time stamp
          if(i==0){timeStamps[0] = *timer_ptr;}

          //producing data for the buffer 1
          for ( int j = 0; j < BUFFER_SIZE; j++ ) {
              *buffer1_ptr++ = 1 ; // produce data
          }
          data_ready1 = 1;
          i++;
    }
    
    if( data_ready2 == 0){
          
          //producing data for the buffer 2
          for ( int j = 0; j < BUFFER_SIZE; j++ ) {
              *buffer2_ptr++ = 2 ; // produce data
          }

          data_ready2 = 1;
          i++;

    }

  }
   //Producer finishing time stamp
   timeStamps[1] = *timer_ptr;

  return;
}



// Consumer
void consumer(void *arg) {

  volatile int _SPM *buffer1_ptr,*buffer2_ptr;

  int id = get_cpuid();

  int i=0; 
  int sum = 0;

  while(i<DATA_LEN/BUFFER_SIZE){

    buffer1_ptr = &spm_ptr[NEXT*(id-1)];
    buffer2_ptr = &spm_ptr[NEXT*(id)];

    if(data_ready1 == 1){
        
        //Consumer starting time stamp
        if(i==0){timeStamps[2] = *timer_ptr;}

        //consuming data from the buffer 1
        for ( int j = 0; j < BUFFER_SIZE; j++ ) {
            sum += *buffer1_ptr++;
        }

        data_ready1 = 0;
        i++;

    }
    
    if(data_ready2 == 1){

        //consuming data from the buffer 2
        for ( int j = 0; j < BUFFER_SIZE; j++ ) {
            sum += *buffer2_ptr++;
        }
        data_ready2 = 0;
        i++;
      }

  }
   //Consumer finishing time stamp
   timeStamps[3] = *timer_ptr;
   return;
}



int main() {

  unsigned i,j;

  data_ready1=0;
  data_ready2=0;

  int id = get_cpuid(); 
  int cnt = get_cpucnt();

  int parameter = 1;

  printf("Total %d Cores\n",get_cpucnt()); // print core count

  printf("Writing the data to the SPM ...\n"); 


  corethread_create(1, &consumer, &parameter); 
  producer();  

  corethread_join(1,&parameter);

  printf("Computation is Done !!\n");

  //Debug

  printf("The Producer starts at %d \n", timeStamps[0]);
  printf("The Producer finishes at %d \n", timeStamps[1]);
  printf("The Consumer starts at %d \n", timeStamps[2]);
  printf("The Consumer finishes at %d \n", timeStamps[3]);
  printf("End-to-End Latency is %d clock cycles\n    \
         for %d words of bulk data\n   \
         and %d of buffer size\n", timeStamps[3]-timeStamps[0],DATA_LEN,BUFFER_SIZE);


/*
//Debug : screen output data
  for (int i=0; i<DATA_LEN*2; ++i) {
        printf("The Output Data %d is %d \n",i, spm_ptr[i]);
   }
  */

}
