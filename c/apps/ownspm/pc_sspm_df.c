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
#define BUFFER_SIZE 512 // words
#define CNT 4 //cores
#define STATUS ((CNT-1)*2) // no of status flags


volatile int _SPM *spm_ptr = (( volatile int _SPM *)0xE8000000);

// Measure execution time with the clock cycle timer
volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
volatile _UNCACHED int start=0;
volatile _UNCACHED int stop=0;
volatile _UNCACHED int timeStamps[3]={0};


// Producer
void producer() {

  volatile int _SPM  *outbuffer1_ptr;
  volatile int _SPM  *outbuffer2_ptr;

  volatile int _SPM *data_ready= &spm_ptr[0];  
  volatile int _SPM *data_valid= &spm_ptr[1];  

  int id = get_cpuid();
  int cnt = get_cpucnt();

  int i=0; 

  while(i<(DATA_LEN/(BUFFER_SIZE*2))){

      if( *data_valid == *data_ready){

          outbuffer1_ptr = &spm_ptr[STATUS+0];
          outbuffer2_ptr = &spm_ptr[STATUS+BUFFER_SIZE];
        
          timeStamps[0] = *timer_ptr;

          //producing data for the buffer 1
          for ( int j = 0; j < BUFFER_SIZE; j++ ) {
              *outbuffer1_ptr++ = 1 ; // produce data
          }
          
          //producing data for the buffer 2
          for ( int j = 0; j < BUFFER_SIZE; j++ ) {
              *outbuffer2_ptr++ = 2 ; // produce data
          }

          *data_ready = !(*data_ready);
          i++;
               
      }

  }

  timeStamps[1] = *timer_ptr;

  return;
}



//intermediate
void intermediate(void *arg){

    int id = get_cpuid();
    int cnt = get_cpucnt();

    //flag pointers for reading
    volatile int _SPM *data_valid0= &spm_ptr[(id-1)*2];
    volatile int _SPM *data_ready0= &spm_ptr[(id-1)*2+1];

    volatile int _SPM *data_ready1= &spm_ptr[(id-1)*2+2];
    volatile int _SPM *data_valid1= &spm_ptr[(id-1)*2+3];

    //buffer pointers
    volatile int _SPM  *inbuffer1_ptr;
    volatile int _SPM  *inbuffer2_ptr;
    volatile int _SPM  *outbuffer1_ptr;
    volatile int _SPM  *outbuffer2_ptr;

    int i=0; 

    while(i<(DATA_LEN/(BUFFER_SIZE*2))){

        if( (*data_ready0 != *data_valid0 ) && (*data_ready1 == *data_valid1) ){


          inbuffer1_ptr = &spm_ptr[STATUS+BUFFER_SIZE*(id-1)*2];
          inbuffer2_ptr = &spm_ptr[STATUS+BUFFER_SIZE*(id-1)*2+BUFFER_SIZE];
          outbuffer1_ptr = &spm_ptr[STATUS+BUFFER_SIZE*(id-1)*2+BUFFER_SIZE*2];
          outbuffer2_ptr = &spm_ptr[STATUS+BUFFER_SIZE*(id-1)*2+BUFFER_SIZE*3];

          //producing data for the buffer 1
          for ( int j = 0; j < BUFFER_SIZE; j++ ) {
              *outbuffer1_ptr++ = *inbuffer1_ptr++ +1 ; // produce data
          }
          
          //producing data for the buffer 2
          for ( int j = 0; j < BUFFER_SIZE; j++ ) {
              *outbuffer2_ptr++ = *inbuffer2_ptr++ +2 ; // produce data
          }

          *data_ready1 =  !(*data_ready1);
          *data_ready0 =  !(*data_ready0);
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

  volatile int _SPM *data_ready= &spm_ptr[(id-1)*2];  
  volatile int _SPM *data_valid= &spm_ptr[(id-1)*2+1]; 

  // this region of the SPM  is used for debugging
  output_ptr= &spm_ptr[STATUS+BUFFER_SIZE*2*(cnt-1)];

  int i=0; 
  while(i<(DATA_LEN/(BUFFER_SIZE*2))){

      if( *data_ready != *data_valid){

        inbuffer1_ptr = &spm_ptr[STATUS+BUFFER_SIZE*(id-1)*2];
        inbuffer2_ptr = &spm_ptr[STATUS+BUFFER_SIZE*(id-1)*2+BUFFER_SIZE];
        
        //Consumer starting time stamp
        if(i==0){timeStamps[2] = *timer_ptr;}

        //consuming data from the buffer 1
        for ( int j = 0; j < BUFFER_SIZE; j++ ) {
            *output_ptr++ = (*inbuffer1_ptr++) -1;
        }
        
        //consuming data from the buffer 2
        for ( int j = 0; j < BUFFER_SIZE; j++ ) {
            *output_ptr++ = (*inbuffer2_ptr++) -2;
        }

        *data_valid = !(*data_valid);
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
  printf("The end-to-end latency is  %d clock cycles \n", timeStamps[3]-timeStamps[0]);

  for (int i=0; i<+STATUS+BUFFER_SIZE*2*(cnt-1)+DATA_LEN; ++i) {
        printf("The Output Data %d is %d \n",i, spm_ptr[i]);
   }
  
}
