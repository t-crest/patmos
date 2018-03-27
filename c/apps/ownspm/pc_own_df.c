/*
    This is a multithread producer-consumer data-flow application for shared SPMs with ownership.

    Author: Oktay Baris
    Copyright: DTU, BSD License
*/
#include <machine/patmos.h>
#include <machine/spm.h>

#include "include/patio.h"
#include "libcorethread/corethread.h"

#define DATA_LEN 4096 // words
#define BUFFER_SIZE 512 //words
#define CNT 4

#define NEXT 0x10000/4 // SPMs are placed every 64 KB 

volatile int _SPM *spm_ptr = (( volatile _SPM int *)0xE8000000);

// Measure execution time with the clock cycle timer
volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
volatile _UNCACHED int start=0;
volatile _UNCACHED int stop=0;
volatile _UNCACHED int timeStamps[3]={0};

//Status pointers in the main memory
volatile _UNCACHED int status[(CNT-1)*2]={0};

volatile _UNCACHED int owner;


// Producer
void producer() {

  volatile int _SPM  *outbuffer1_ptr;
  volatile int _SPM  *outbuffer2_ptr;

  volatile _UNCACHED  int *data_ready= &status[0];  
  volatile _UNCACHED  int *data_valid= &status[1];  

  int id = get_cpuid();
  int cnt = get_cpucnt();

  int i=0; 

  while(i<(DATA_LEN/(BUFFER_SIZE*2))){

      if( (id == owner) && (*data_ready == *data_valid)){

         // printf("The owner2 is %d \n", owner);

          outbuffer1_ptr = &spm_ptr[NEXT*id];
          outbuffer2_ptr = &spm_ptr[NEXT*(id+1)];
        
           //Producer starting time stamp
          if(i==0){timeStamps[0] = *timer_ptr;}

          //producing data for the buffer 1
          for ( int j = 0; j < BUFFER_SIZE; j++ ) {
              *outbuffer1_ptr++ = 1 ; // produce data
          }
          
          //producing data for the buffer 2
          for ( int j = 0; j < BUFFER_SIZE; j++ ) {
              *outbuffer2_ptr++ = 2 ; // produce data
          }

          //Producer finishing time stamp
          if(i==(DATA_LEN/(BUFFER_SIZE*2))-1){timeStamps[1] = *timer_ptr;}

          *data_ready = !(*data_ready);
          i++;

          // transfer the ownership to the next core
          if (id < cnt - 1) {
                ++owner;
          } else {
                owner = 0;
          }         
      }

  }
  //Producer finishing time stamp
  //timeStamps[1] = *timer_ptr;
  

  return;
}



//intermediate
void intermediate(void *arg){

    int id = get_cpuid();
    int cnt = get_cpucnt();

    //mem flag pointers for reading
    volatile _UNCACHED int *data_valid0= &status[(id-1)*2];
    volatile _UNCACHED int *data_ready0= &status[(id-1)*2+1];

    volatile _UNCACHED int *data_ready1= &status[(id-1)*2+2];
    volatile _UNCACHED int *data_valid1= &status[(id-1)*2+3];

    //buffer pointers
    volatile int _SPM  *inbuffer1_ptr;
    volatile int _SPM  *inbuffer2_ptr;
    volatile int _SPM  *outbuffer1_ptr;
    volatile int _SPM  *outbuffer2_ptr;

    int i=0; 

    while(i<(DATA_LEN/(BUFFER_SIZE*2))){

        if( (id == owner) && ((*data_ready0 != *data_valid0 ) && (*data_ready1 == *data_valid1)) ){

          inbuffer1_ptr = &spm_ptr[2*NEXT*(id-1)];
          inbuffer2_ptr = &spm_ptr[2*NEXT*(id-1)+NEXT];
          outbuffer1_ptr = &spm_ptr[2*NEXT*(id-1)+NEXT*2];
          outbuffer2_ptr = &spm_ptr[2*NEXT*(id-1)+NEXT*3];

          //producing data for the buffer 1
          for ( int j = 0; j < BUFFER_SIZE; j++ ) {
              *outbuffer1_ptr++ = *inbuffer1_ptr++ +1 ; // produce data
          }
          
          //producing data for the buffer 2
          for ( int j = 0; j < BUFFER_SIZE; j++ ) {
              *outbuffer2_ptr++ = *inbuffer2_ptr++ +2 ; // produce data
          }

          // update the flags
          *data_ready1 =  !(*data_ready1);
          *data_ready0 =  !(*data_ready0);
          //for the time being for flow control
          i++;

          // transfer the ownership to the next core
          if (id < cnt - 1) {
                ++owner;
          } else {
                owner = 0;
          }
         
      }

    }

    return;

}


// Consumer
void consumer(void *arg) {

  volatile int _SPM  *output_ptr, *inbuffer1_ptr,*inbuffer2_ptr;

  int id = get_cpuid();
  int cnt = get_cpucnt();

  //flag pointers
  volatile _UNCACHED int *data_ready= &status[(id-1)*2];  
  volatile _UNCACHED int *data_valid= &status[(id-1)*2+1]; 

  // this region of the SPM  is used for debugging
  output_ptr= &spm_ptr[NEXT*(cnt+2)];

  int i=0; 
  while( i<(DATA_LEN/(BUFFER_SIZE*2))){

      if( (id == owner) && (*data_ready != *data_valid)){

        inbuffer1_ptr = &spm_ptr[2*NEXT*(id-1)];
        inbuffer2_ptr = &spm_ptr[2*NEXT*(id-1)+NEXT];
        
        //Consumer starting time stamp
        if(i==0){
            timeStamps[2] = *timer_ptr;
            }

        //consuming data from the buffer 1
        for ( int j = 0; j < BUFFER_SIZE; j++ ) {
            *output_ptr++ = (*inbuffer1_ptr++) -1;
        }
        
        //consuming data from the buffer 2
        for ( int j = 0; j < BUFFER_SIZE; j++ ) {
            *output_ptr++ = (*inbuffer2_ptr++) -2;
        }

        //Consumer finishing time stamp
        if(i==(DATA_LEN/(BUFFER_SIZE*2))-1){
            timeStamps[3] = *timer_ptr;
            }

        *data_valid = !(*data_valid);
        i++; 

        // transfer the ownership to the next core
        if (id < (cnt-1)){
            owner++;
        }else{
            owner=0;
        }      
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

  owner = 0; //initially core 0 is the owner

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
  printf("The end-to-end latency is  %d clock cycles \n", timeStamps[3]-timeStamps[0]);

  for (int i=0; i<BUFFER_SIZE*2*(cnt-1)+DATA_LEN; ++i) {
        printf("The Output Data %d is %d \n",i, spm_ptr[i]);
   }
  
}
