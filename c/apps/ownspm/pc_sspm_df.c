/*
    This is a multithread producer-consumer data-flow application 
    for a single shared SPM with TDM access.

    Author: Oktay Baris
    Copyright: DTU, BSD License
*/
#include <machine/patmos.h>
#include <machine/spm.h>

#include "include/patio.h"
#include "libcorethread/corethread.h"

#define DATA_LEN 4096 // words
#define BUFFER_SIZE 128 // a buffer size of 128 Word requires 3MB SPM size for 4 cores
#define CNT 4 //cores
#define B1_STATUS (CNT-1) // no of status flags for buffer1
#define STATUS (B1_STATUS*2) // no of status flags

volatile int _SPM *spm_ptr = (( volatile int _SPM *)0xE8000000);

// Measure execution time with the clock cycle timer
volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
volatile _UNCACHED int start=0;
volatile _UNCACHED int stop=0;
volatile _UNCACHED int timeStamps[4]={0};

// Producer
void producer() {

  // pointers to buffers
  volatile int _SPM  *outbuffer1_ptr;
  volatile int _SPM  *outbuffer2_ptr;

  // pointers to status flags for buffer 1
  volatile int _SPM *b1_ready= &spm_ptr[0];    
  //flag pointers for buffer 2
  volatile int _SPM *b2_ready= &spm_ptr[B1_STATUS+0];   

  int i=0; 

  while(i<DATA_LEN/BUFFER_SIZE){

      outbuffer1_ptr = &spm_ptr[STATUS+0];
      outbuffer2_ptr = &spm_ptr[STATUS+BUFFER_SIZE];

      if( *b1_ready == 0){
 
         //Producer starting time stamp
         if(i==0){timeStamps[0] = *timer_ptr;}

          //producing data for the buffer 1
          for ( int j = 0; j < BUFFER_SIZE; j++ ) {
              *outbuffer1_ptr++ = 1 ; // produce data
          }
         // flip the data ready flag for buffer 1
          *b1_ready = 1;
          i++;
      }

      if( *b2_ready == 0){
          
          //producing data for the buffer 2
          for ( int j = 0; j < BUFFER_SIZE; j++ ) {
              *outbuffer2_ptr++ = 2 ; // produce data
          }
          // flip the data ready flag for buffer 2
          *b2_ready = 1;
          i++;           
      }
  }
//producer finishing time stamp
  timeStamps[1] = *timer_ptr;
  return;
}

//intermediate
void intermediate(void *arg){

    int id = get_cpuid();

    //flag pointers for buffer 1
    volatile int _SPM *b1_ready0= &spm_ptr[id-1];
    volatile int _SPM *b1_ready1= &spm_ptr[id];
    //flag pointers for buffer 2
    volatile int _SPM *b2_ready0= &spm_ptr[B1_STATUS+id-1];
    volatile int _SPM *b2_ready1= &spm_ptr[B1_STATUS+id];
    //pointers buffering the data
    volatile int _SPM  *inbuffer1_ptr;
    volatile int _SPM  *inbuffer2_ptr;
    volatile int _SPM  *outbuffer1_ptr;
    volatile int _SPM  *outbuffer2_ptr;

    int i=0; 

    while(i<DATA_LEN/BUFFER_SIZE){

        inbuffer1_ptr = &spm_ptr[STATUS+BUFFER_SIZE*(id-1)*2];
        inbuffer2_ptr = &spm_ptr[STATUS+BUFFER_SIZE*(id-1)*2+BUFFER_SIZE];
        outbuffer1_ptr = &spm_ptr[STATUS+BUFFER_SIZE*(id-1)*2+BUFFER_SIZE*2];
        outbuffer2_ptr = &spm_ptr[STATUS+BUFFER_SIZE*(id-1)*2+BUFFER_SIZE*3];

        if( (*b1_ready0 == 1 ) && (*b1_ready1 == 0) ){

                //producing data for the buffer 1
                for ( int j = 0; j < BUFFER_SIZE; j++ ) {
                    *outbuffer1_ptr++ = *inbuffer1_ptr++ +1 ; // produce data
                }
        
        // flip the flags for buffer 1
        *b1_ready1 =  1;
        *b1_ready0 =  0;
         i++;         
        }

        if( (*b2_ready0 == 1 ) && (*b2_ready1 == 0) ){
                //producing data for the buffer 2
                for ( int j = 0; j < BUFFER_SIZE; j++ ) {
                    *outbuffer2_ptr++ = *inbuffer2_ptr++ +2 ; // produce data
                }

        // flip the flag for buffer 2
        *b2_ready1 =  1;
        *b2_ready0 =  0;
        i++;        
      }
    }
    return;
}


// Consumer
void consumer(void *arg) {

  volatile int _SPM *inbuffer1_ptr,*inbuffer2_ptr;

  int id = get_cpuid();
  int cnt = get_cpucnt();

  //flag pointers for buffer 1
  volatile int _SPM *b1_ready= &spm_ptr[id-1];  
  //flag pointers for buffer 2
  volatile int _SPM *b2_ready= &spm_ptr[B1_STATUS+id-1];  

  int i=0;
  int sum=0;

  while(i<DATA_LEN/BUFFER_SIZE){

    inbuffer1_ptr = &spm_ptr[STATUS+BUFFER_SIZE*(id-1)*2];
    inbuffer2_ptr = &spm_ptr[STATUS+BUFFER_SIZE*(id-1)*2+BUFFER_SIZE];

    if( *b1_ready == 1){

        //Consumer starting time stamp
        if(i==0){timeStamps[2] = *timer_ptr;}

        //consuming data from the buffer 1
        for ( int j = 0; j < BUFFER_SIZE; j++ ) {
            sum += (*inbuffer1_ptr++);
        }

        // flip the flag for buffer 1
        *b1_ready = 0;
        i++; 
    }

    if( *b2_ready == 1){
        
        //consuming data from the buffer 2
        for ( int j = 0; j < BUFFER_SIZE; j++ ) {
            sum += (*inbuffer2_ptr++);
        }
        // flip the flag for buffer 2
        *b2_ready = 0;
        i++;         
      }
  }
   //Consumer finishing time stamp
   timeStamps[3] = *timer_ptr;

  return;
}



int main() {

  unsigned i,j;

 //set the status flags to 0
  for(int i=0;i<STATUS;++i){
    spm_ptr[i]=0;
  }

  int id = get_cpuid(); 
  int cnt = get_cpucnt();

  int parameter = 1;

  printf("Total %d Cores\n",get_cpucnt()); 

  for (i=1; i<cnt-1; ++i) {
    int core_id = i; 
    int parameter = 1;
    corethread_create(core_id, &intermediate, &parameter);
    printf("thread %d is created \n",i); 
  }

  corethread_create(cnt-1, &consumer, &parameter);

  producer();

  for(j=1; j<cnt; ++j) { 
    int parameter = 1;
    corethread_join(j,&parameter);
  }

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
  for (int i=0; i<+STATUS+BUFFER_SIZE*2*(cnt-1)+DATA_LEN; ++i) {
        printf("The Output Data %d is %d \n",i, spm_ptr[i]);
   }
  */
}
