/*
    This is a multithread producer-consumer data-flow application 
    for a single shared SPM with TDM access.

    Author: Oktay Baris
    Copyright: DTU, BSD License
*/

#include <stdio.h>
#include <machine/patmos.h>
#include <machine/spm.h>

//#include "include/patio.h"
#include "libcorethread/corethread.h"

#include "ownspm.h"

#define CNT 4 //cores
#define STATUS (2*CNT) // no of status flags

// Measure execution time with the clock cycle timer
volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
volatile _UNCACHED int start=0;
volatile _UNCACHED int stop=0;
volatile _UNCACHED int timeStamps[4]={0};

// Producer
void producer() {

  // pointers to buffers
  volatile int _SPM  *outbuffer1_ptr= &spm_ptr[STATUS+0];
  volatile int _SPM  *outbuffer2_ptr= &spm_ptr[STATUS+BUFFER_SIZE];

  // pointers to status flags for buffers
  volatile int _SPM *b1_ready= &spm_ptr[0];    
  volatile int _SPM *b2_ready= &spm_ptr[1];   

  int i=0; 

  while(i<DATA_LEN){

      while( *b1_ready != 0){
          ;
      }
 
         //Producer starting time stamp
         if(i==0){timeStamps[0] = *timer_ptr;}

          int len = DATA_LEN - i;
          if(BUFFER_SIZE < len)
          len = BUFFER_SIZE;

          //producing data for the buffer 1
          for ( int j = 0; j < len; j++ ) {
              *(outbuffer1_ptr+j) = 1 ; // produce data
          }
         // flip the data ready flag for buffer 1
          *b1_ready = 1;
           i += len;
      

      while( *b2_ready != 0){
          ;
      }
          
          //producing data for the buffer 2
          for ( int j = 0; j < len; j++ ) {
              *(outbuffer2_ptr+j) = 2 ; // produce data
          }
          // flip the data ready flag for buffer 2
          *b2_ready = 1;
           i += len;         
      
  }
//producer finishing time stamp
  timeStamps[1] = *timer_ptr;
  return;
}



//intermediate
void intermediate(void *arg){

    int id = get_cpuid();

    //flag pointers for buffers
    volatile int _SPM *b1_ready0= &spm_ptr[0];
    volatile int _SPM *b2_ready0= &spm_ptr[1];
    volatile int _SPM *b1_ready1= &spm_ptr[2];
    volatile int _SPM *b2_ready1= &spm_ptr[3];
    //pointers buffering the data
    volatile int _SPM  *inbuffer1_ptr = &spm_ptr[STATUS+BUFFER_SIZE*0];
    volatile int _SPM  *inbuffer2_ptr = &spm_ptr[STATUS+BUFFER_SIZE*1];
    volatile int _SPM  *outbuffer1_ptr = &spm_ptr[STATUS+BUFFER_SIZE*2];
    volatile int _SPM  *outbuffer2_ptr  = &spm_ptr[STATUS+BUFFER_SIZE*3];

    int i=0; 

    while(i<DATA_LEN){


        while( !((*b1_ready0 == 1 ) && (*b1_ready1 == 0)) ){
            ;
        }

                int len = DATA_LEN - i;
                if(BUFFER_SIZE < len)
                len = BUFFER_SIZE;

                //producing data for the buffer 1
                for ( int j = 0; j < len; j++ ) {
                    *(outbuffer1_ptr+j) = *(inbuffer1_ptr+j) +1 ; // produce data
                }
        
        // flip the flags for buffer 1
          *b1_ready1 =  1;
          *b1_ready0 =  0;
          i += len;        
    

        while(!((*b2_ready0 == 1 ) && (*b2_ready1 == 0)) ){
            ;
        }
                 len = DATA_LEN - i;
                 if(BUFFER_SIZE < len)
                len = BUFFER_SIZE;

                //producing data for the buffer 2
                for ( int j = 0; j < len; j++ ) {
                    *(outbuffer2_ptr+j) = *(inbuffer2_ptr+j) +2 ; // produce data
                }

        // flip the flag for buffer 2
        *b2_ready1 =  1;
        *b2_ready0 =  0;
         i += len;
      
    }
    return;
}


// Consumer
void consumer(void *arg) {

  int id = get_cpuid();
  int cnt = get_cpucnt();

  volatile int _SPM *inbuffer1_ptr= &spm_ptr[STATUS+BUFFER_SIZE*2];
  volatile int _SPM *inbuffer2_ptr= &spm_ptr[STATUS+BUFFER_SIZE*3];


  //flag pointers for buffer 1
  volatile int _SPM *b1_ready= &spm_ptr[2];  
  //flag pointers for buffer 2
  volatile int _SPM *b2_ready= &spm_ptr[3];  

  int i=0;
  int sum=0;

  while(i<DATA_LEN){


     while(*b1_ready != 1){
         ;
     }

        int len = DATA_LEN - i;
        if(BUFFER_SIZE < len)
        len = BUFFER_SIZE;

        //Consumer starting time stamp
        if(i==0){timeStamps[2] = *timer_ptr;}

        //consuming data from the buffer 1
        for ( int j = 0; j < len; j++ ) {
            sum += *(inbuffer1_ptr+j);
        }

        // flip the flag for buffer 1
        *b1_ready = 0;
         i += len; 
    

    while( *b2_ready != 1){
        ;
    }
        
        //consuming data from the buffer 2
        for ( int j = 0; j < len; j++ ) {
            sum += *(inbuffer2_ptr+j);
        }
        // flip the flag for buffer 2
        *b2_ready = 0;
         i += len;        
      
  }
   //Consumer finishing time stamp
   timeStamps[3] = *timer_ptr;

  return;
}



int main() {

  unsigned i,j;

  int id = get_cpuid(); 
  int cnt = get_cpucnt();

  int parameter = 1;

  printf("Total %d Cores\n",get_cpucnt()); 

  corethread_create(1, &intermediate, NULL);
  corethread_create(2, &consumer, NULL); 

  producer();

  for(j=1; j<3; ++j) { 
    void *dummy; 
    corethread_join(j,&dummy);
  }

  //Debug  
  printf("End-to-End Latency is %d clock cycles\n    \
         for %d words of bulk data\n   \
         and %d of buffer size\n", timeStamps[3]-timeStamps[0],DATA_LEN,BUFFER_SIZE);
  int cycles = timeStamps[3]-timeStamps[0];
  printf("measure pc_sspm_df: %d.%d cycles per word for %d words in %d words buffer\n",
    cycles/DATA_LEN, cycles*10/DATA_LEN%10, DATA_LEN, BUFFER_SIZE);
}
