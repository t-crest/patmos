/*
    This is a multithread producer-consumer data-flow application for shared SPMs with ownership.

    Author: Oktay Baris
    Copyright: DTU, BSD License
*/
#include <machine/patmos.h>
#include <machine/spm.h>

#include "include/patio.h"
#include "libcorethread/corethread.h"

#define DATA_LEN 1024 // words
#define BUFFER_SIZE 128 // a buffer size of 128W requires 3MB SPM size for 4 cores
#define CNT 4
#define STATUS_LEN ((CNT-1)*2) // no of status flags for a single buffer

#define NEXT 0x10000/4 // SPMs are placed every 64 KB 

volatile int _SPM *spm_ptr = (( volatile _SPM int *)0xE8000000);

// Measure execution time with the clock cycle timer
volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
volatile _UNCACHED int start=0;
volatile _UNCACHED int stop=0;
volatile _UNCACHED int timeStamps[4]={0};

//Status pointers in the main memory
volatile _UNCACHED int status[STATUS_LEN*2]={0};


// Producer
void producer() {

  volatile int _SPM  *outbuffer1_ptr;
  volatile int _SPM  *outbuffer2_ptr;

  // pointers to status flags for buffer 1
  volatile _UNCACHED  int *b1_ready= &status[0];  
  volatile _UNCACHED  int *b1_valid= &status[1];  
  // pointers to status flags for buffer 1
  volatile _UNCACHED  int *b2_ready= &status[STATUS_LEN+0];  
  volatile _UNCACHED  int *b2_valid= &status[STATUS_LEN+1]; 

  int id = get_cpuid();

  int i=0; 

  while(i<DATA_LEN/BUFFER_SIZE){

    outbuffer1_ptr = &spm_ptr[NEXT*id];
    outbuffer2_ptr = &spm_ptr[NEXT*(id+1)];

    if(*b1_ready == *b1_valid){
        
           //Producer starting time stamp
          if(i==0){timeStamps[0] = *timer_ptr;}

          //producing data for the buffer 1
          for ( int j = 0; j < BUFFER_SIZE; j++ ) {
              *outbuffer1_ptr++ = 1 ; // produce data
          }

          // flip the data ready flag for buffer 1
          *b1_ready = !(*b1_ready);
          i++;
    }

    if(*b2_ready == *b2_valid){
          
          //producing data for the buffer 2
          for ( int j = 0; j < BUFFER_SIZE; j++ ) {
              *outbuffer2_ptr++ = 2 ; // produce data
          }

          // flip the data ready flag for buffer 2
          *b2_ready = !(*b2_ready);
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

    // pointers to status flags for buffer 1
    volatile _UNCACHED int *b1_valid0= &status[(id-1)*2];
    volatile _UNCACHED int *b1_ready0= &status[(id-1)*2+1];
    volatile _UNCACHED int *b1_ready1= &status[(id-1)*2+2];
    volatile _UNCACHED int *b1_valid1= &status[(id-1)*2+3];
    // pointers to status flags for buffer 1
    volatile _UNCACHED int *b2_valid0= &status[STATUS_LEN+(id-1)*2];
    volatile _UNCACHED int *b2_ready0= &status[STATUS_LEN+(id-1)*2+1];
    volatile _UNCACHED int *b2_ready1= &status[STATUS_LEN+(id-1)*2+2];
    volatile _UNCACHED int *b2_valid1= &status[STATUS_LEN+(id-1)*2+3];

    //buffer pointers
    volatile int _SPM  *inbuffer1_ptr;
    volatile int _SPM  *inbuffer2_ptr;
    volatile int _SPM  *outbuffer1_ptr;
    volatile int _SPM  *outbuffer2_ptr;

    int i=0; 

    while(i<DATA_LEN/BUFFER_SIZE){

        inbuffer1_ptr = &spm_ptr[2*NEXT*(id-1)];
        inbuffer2_ptr = &spm_ptr[2*NEXT*(id-1)+NEXT];
        outbuffer1_ptr = &spm_ptr[2*NEXT*(id-1)+NEXT*2];
        outbuffer2_ptr = &spm_ptr[2*NEXT*(id-1)+NEXT*3];

        if( (*b1_ready0 != *b1_valid0 ) && (*b1_ready1 == *b1_valid1) ){

            //producing data for the buffer 1
            for ( int j = 0; j < BUFFER_SIZE; j++ ) {
                *outbuffer1_ptr++ = *inbuffer1_ptr++ +1 ; // produce data
            }

            // update the flags for buffer 1
            *b1_ready1 =  !(*b1_ready1);
            *b1_ready0 =  !(*b1_ready0);
            //for the time being for flow control
            i++;

        }

        if((*b2_ready0 != *b2_valid0 ) && (*b2_ready1 == *b2_valid1) ){
          
            //producing data for the buffer 2
            for ( int j = 0; j < BUFFER_SIZE; j++ ) {
                *outbuffer2_ptr++ = *inbuffer2_ptr++ +2 ; // produce data
            }

            // update the flags for buffer 2
            *b2_ready1 =  !(*b2_ready1);
            *b2_ready0 =  !(*b2_ready0);
            //for the time being for flow control
            i++;
         
        }
    }
    return;
}


// Consumer
void consumer(void *arg) {

  volatile int _SPM  *output_ptr, *inbuffer1_ptr,*inbuffer2_ptr;

  int id = get_cpuid();
  int cnt = get_cpucnt();

  /// pointers to status flags for buffer 1
  volatile _UNCACHED int *b1_ready= &status[(id-1)*2];  
  volatile _UNCACHED int *b1_valid= &status[(id-1)*2+1]; 
   // pointers to status flags for buffer 2
  volatile _UNCACHED int *b2_ready= &status[STATUS_LEN+(id-1)*2];  
  volatile _UNCACHED int *b2_valid= &status[STATUS_LEN+(id-1)*2+1]; 

  // this region of the SPM  is used for debugging
  output_ptr= &spm_ptr[NEXT*(cnt+2)];

  int i=0; 
  int sum=0;
  while( i<DATA_LEN/BUFFER_SIZE){

    inbuffer1_ptr = &spm_ptr[2*NEXT*(id-1)];
    inbuffer2_ptr = &spm_ptr[2*NEXT*(id-1)+NEXT];

    if(*b1_ready != *b1_valid){

        //Consumer starting time stamp
        if(i==0){
            timeStamps[2] = *timer_ptr;
            }

        //consuming data from the buffer 1
        for ( int j = 0; j < BUFFER_SIZE; j++ ) {
            sum += (*inbuffer1_ptr++);
        }

        *b1_valid = !(*b1_valid);
        i++; 
    }
        

    if( *b2_ready != *b2_valid){

        //consuming data from the buffer 2
        for ( int j = 0; j < BUFFER_SIZE; j++ ) {
            sum += (*inbuffer2_ptr++);
        }
        *b2_valid = !(*b2_valid);
        i++; 
      
    }
  }
   //Consumer finishing time stamp
   timeStamps[3] = *timer_ptr;

  return;
}



int main() {

  unsigned i,j;

  int id = get_cpuid(); // id=0
  int cnt = get_cpucnt();

  //owner = 0; //initially core 0 is the owner

  int parameter = 1;

  printf("Total %d Cores\n",get_cpucnt()); // print core count

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


/* //Debug : screen output data
  for (int i=0; i<BUFFER_SIZE*2*(cnt-1)+DATA_LEN; ++i) {
        printf("The Output Data %d is %d \n",i, spm_ptr[i]);
   }
  */
}
