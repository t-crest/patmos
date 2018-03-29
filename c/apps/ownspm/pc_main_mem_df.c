/*
    This is a multithread producer-consumer application with message passing over main memory.

    Author: Oktay Baris
    Copyright: DTU, BSD License
*/
#include <machine/patmos.h>
#include <machine/spm.h>

#include "include/patio.h"
#include "libcorethread/corethread.h"

#define DATA_LEN 4096 // words
#define BUFFER_SIZE 128 // a buffer size of 128W requires 3MB spm size for 4 cores
#define CNT 4 //cores
#define STATUS_LEN (CNT-1) // no of status flags for buffer1

// Measure execution time with the clock cycle timer
volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
volatile _UNCACHED int start=0;
volatile _UNCACHED int stop=0;
volatile _UNCACHED int timeStamps[4]={0};

//data
volatile int data[BUFFER_SIZE*STATUS_LEN*2]={0};
//flags
volatile _UNCACHED int status[STATUS_LEN*2]={0};

// Producer
void producer() {

  volatile int *buffer1_ptr;
  volatile int *buffer2_ptr;

  // pointers to status flags for buffer 1
  volatile _UNCACHED int *b1_ready= &status[0];  
  // pointers to status flags for buffer 2
  volatile _UNCACHED int *b2_ready= &status[STATUS_LEN+0];  
 
  int i=0;

  while(i<DATA_LEN/BUFFER_SIZE){

    buffer1_ptr = &data[0];
    buffer2_ptr = &data[BUFFER_SIZE];

    if( *b1_ready == 0){
        
          //Producer starting time stamp
          if(i==0){timeStamps[0] = *timer_ptr;}
          inval_dcache(); //invalidate the data cache
          //producing data for the buffer 1
          for ( int j = 0; j < BUFFER_SIZE; j++ ) {
              *buffer1_ptr++ = 1 ; // produce data
          }
          *b1_ready = 1;
          i++;
    }
    
    if( *b2_ready == 0){

          inval_dcache(); //invalidate the data cache
          //producing data for the buffer 2
          for ( int j = 0; j < BUFFER_SIZE; j++ ) {
              *buffer2_ptr++ = 2 ; // produce data
          }

          *b2_ready = 1;
          i++;
    }
  }
   //Producer finishing time stamp
   timeStamps[1] = *timer_ptr;
   return;
}


//intermediate
void intermediate(void *arg){

    int id = get_cpuid();

    //flag pointers for buffer 1
    volatile _UNCACHED int *b1_ready0= &status[id-1];
    volatile _UNCACHED int *b1_ready1= &status[id];

    //flag pointers for buffer 2
    volatile _UNCACHED int *b2_ready0= &status[STATUS_LEN+id-1];
    volatile _UNCACHED int *b2_ready1= &status[STATUS_LEN+id];

    //pointers buffering the data
    volatile int *inbuffer1_ptr;
    volatile int *inbuffer2_ptr;
    volatile int *outbuffer1_ptr;
    volatile int *outbuffer2_ptr;

    int i=0; 

    while(i<DATA_LEN/BUFFER_SIZE){

        inbuffer1_ptr = &data[BUFFER_SIZE*(id-1)*2];
        inbuffer2_ptr = &data[BUFFER_SIZE*(id-1)*2+BUFFER_SIZE];
        outbuffer1_ptr = &data[BUFFER_SIZE*(id-1)*2+BUFFER_SIZE*2];
        outbuffer2_ptr = &data[BUFFER_SIZE*(id-1)*2+BUFFER_SIZE*3];

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

  volatile int *buffer1_ptr,*buffer2_ptr;

  int id = get_cpuid();

    //flag pointers for buffer 1
  volatile _UNCACHED int *b1_ready= &status[id-1];  

  //flag pointers for buffer 2
  volatile _UNCACHED int *b2_ready= &status[STATUS_LEN+ id-1];  

  int i=0; 
  int sum=0;

  while(i<DATA_LEN/BUFFER_SIZE){

    buffer1_ptr = &data[BUFFER_SIZE*(id-1)*2];
    buffer2_ptr = &data[BUFFER_SIZE*(id-1)*2+BUFFER_SIZE];

    if( *b1_ready == 1){
        
        //Consumer starting time stamp
        if(i==0){timeStamps[2] = *timer_ptr;}
        inval_dcache(); //invalidate the data cache
        //consuming data from the buffer 1
        for ( int j = 0; j < BUFFER_SIZE; j++ ) {
            sum += (*buffer1_ptr++);
        }
        *b1_ready = 0;
        i++;
    }
    
    if( *b2_ready == 1){

        //consuming data from the buffer 2
        inval_dcache(); //invalidate the data cache
        for ( int j = 0; j < BUFFER_SIZE; j++ ) {
            sum += (*buffer2_ptr++);
        }
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

  int id = get_cpuid(); 
  int cnt = get_cpucnt();

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

/*
  //Debug : screen output data
  for (int i=0; i<DATA_LEN*2; ++i) {
        printf("The Output Data %d is %d \n",i, spm_ptr[i]);
   }
*/

}
