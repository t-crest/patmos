/*
    This is a multithread producer-consumer application 
    for a single shared SPM with TDM access.

    Author: Oktay Baris
    Copyright: DTU, BSD License
*/
#include <machine/patmos.h>
#include <machine/spm.h>

#include "include/patio.h"
#include "libcorethread/corethread.h"

#define DATA_LEN  4096 // words
#define BUFFER_SIZE 256 // words

volatile int _SPM *spm_ptr = (( volatile _SPM int *)0xE8000000);

// Measure execution time with the clock cycle timer
volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
volatile _UNCACHED int start=0;
volatile _UNCACHED int stop=0;
volatile _UNCACHED int timeStamps[4]={0};

//flags
volatile int _SPM *data_ready1,*data_ready2;

// Producer
void producer() {

  volatile int _SPM  *buffer1_ptr;
  volatile int _SPM  *buffer2_ptr;

  data_ready1=&spm_ptr[0];
  data_ready2=&spm_ptr[1];

  int id = get_cpuid();
  int cnt = get_cpucnt();

  int i=0; 

  while(i<DATA_LEN/(BUFFER_SIZE)){

    buffer1_ptr = &spm_ptr[2];
    buffer2_ptr = &spm_ptr[2+BUFFER_SIZE];

    if( *data_ready1 == 0){

          //Producer starting time stamp
          if(i==0){timeStamps[0] = *timer_ptr;}

          //producing data for the buffer 1
          for ( int j = 0; j < BUFFER_SIZE; j++ ) {
              *buffer1_ptr++ = 1 ; // produce data
          }
          // set the data ready flag for buffer 1
          *data_ready1 =  1;
          i++;
     }
     
     if( *data_ready2 == 0){
          
          //producing data for the buffer 2
          for ( int j = 0; j < BUFFER_SIZE; j++ ) {
              *buffer2_ptr++ = 2 ; // produce data
          }
         // set the data ready flag for buffer 2
         *data_ready2 =  1;
          i++;

     }
    //i++;
   }

 //producer finishing time stamp
  timeStamps[1] = *timer_ptr;

  return;
}



// Consumer
void consumer(void *arg) {

  volatile int _SPM  *output_ptr, *buffer1_ptr,*buffer2_ptr;

  // this region of the SPM  is used for debugging
  output_ptr= &spm_ptr[DATA_LEN];
  
  data_ready1=&spm_ptr[0];
  data_ready2=&spm_ptr[1];


  int id = get_cpuid();
  int cnt = get_cpucnt();


  int i=0; 
  while(i<DATA_LEN/(BUFFER_SIZE)){

      buffer1_ptr = &spm_ptr[2];
      buffer2_ptr = &spm_ptr[2+BUFFER_SIZE];

      if( *data_ready1 == 1){
        
        //Consumer starting time stamp
        if(i==0){timeStamps[2] = *timer_ptr;}

        //consuming data from the buffer 1
        for ( int j = 0; j < BUFFER_SIZE; j++ ) {
            *output_ptr++ = (*buffer1_ptr++) +1;
        }
        // lower the data ready flag for buffer 1
        *data_ready1 = 0;
        i++;
      } 
      
      if( *data_ready2 == 1){
    
        //consuming data from the buffer 2
        for ( int j = 0; j < BUFFER_SIZE; j++ ) {
            *output_ptr++ = (*buffer2_ptr++) +2;
        }
        // lower the data ready flag for buffer 2
        *data_ready2 = 0;
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

  int id = get_cpuid(); // id=0
  int cnt = get_cpucnt();

  int parameter = 1;

  printf("Total %d Cores\n",get_cpucnt()); // print core count

  corethread_create(1, &consumer, &parameter); 
  producer();  
  corethread_join(1,&parameter);

  printf("Computation is Done !!\n");

  //Debug
  printf("The Producer starts at %d \n", timeStamps[0]);
  printf("The Producer finishes at %d \n", timeStamps[1]);
  printf("The Consumer starts at %d \n", timeStamps[2]);
  printf("The Consumer finishes at %d \n", timeStamps[3]);

  for (int i=0; i<DATA_LEN*2; ++i) {
        printf("The Output Data %d is %d \n",i, spm_ptr[i]);
   }
  

}
