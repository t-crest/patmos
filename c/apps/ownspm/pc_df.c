/*
    This is a multithread producer-consumer data flow application for shared SPMs with ownership.

    Author: Oktay Baris
            Torur Biskopsto Strom
    Copyright: DTU, BSD License
*/

#ifndef _SSPM_DF
#ifndef _MULTIOWN_DF
#ifndef _OWN_DF
#ifndef _MAINMEM_DF
#define _MULTIOWN_DF
#endif
#endif
#endif
#endif


#include <stdio.h>
#include <machine/patmos.h>
#include <machine/spm.h>

#include "libcorethread/corethread.h"

#include "ownspm.h"

// Measure execution time with the clock cycle timer
volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
volatile _UNCACHED int start=0;
volatile _UNCACHED int stop=0;
volatile _UNCACHED int timeStamps[4]={0};

//flags
#ifdef _SSPM_DF
#define _NAME "sspm_df"
#endif
#ifdef _MULTIOWN_DF
#define _NAME "multiown_df"
#include "spmpool.h"
#endif
#ifdef _OWN_DF
#define _NAME "own_df"
volatile _UNCACHED int status[STATUS_LEN];
#endif
#ifdef _MAINMEM_DF
#define _NAME "mainmem_df"
//flags
volatile _UNCACHED int status[STATUS_LEN];
//data
volatile int data[BUFFER_SIZE*STATUS_LEN];
#endif

// Producer
void producer() {

  int id = get_cpuid();

#ifdef _SSPM_DF
  //data buffers
  volatile int _SPM  *buffer1_ptr = &spm_ptr[STATUS_LEN+0];
  volatile int _SPM  *buffer2_ptr = &spm_ptr[STATUS_LEN+BUFFER_SIZE];
  //flag pointers
  volatile int _SPM *b1_ready= &spm_ptr[0];  
  volatile int _SPM *b2_ready= &spm_ptr[STATUS_OFFSET];  
#endif
#ifdef _MULTIOWN_DF
  // data buffers
  volatile int _SPM *buffer1_ptr = spm_base(id);
  volatile int _SPM *buffer2_ptr = spm_base((id+1));
  //flag pointers
  volatile int _SPM *b1_ready=spm_base(FLAG_OFFSET);
  volatile int _SPM *b2_ready=STATUS_OFFSET+spm_base(FLAG_OFFSET);
#endif
#ifdef _OWN_DF
  //data buffers
  volatile int _SPM *buffer1_ptr = &spm_ptr[NEXT*id];
  volatile int _SPM *buffer2_ptr = &spm_ptr[NEXT*(id+1)];
  //flag pointers
  volatile _UNCACHED int *b1_ready= &status[0];  
  volatile _UNCACHED int *b2_ready= &status[STATUS_OFFSET];  
#endif
#ifdef _MAINMEM_DF
  //data pointers
  volatile int *buffer1_ptr = &data[0];
  volatile int *buffer2_ptr = &data[BUFFER_SIZE];
  //flag pointers
  volatile _UNCACHED int *b1_ready= &status[0];  
  volatile _UNCACHED int *b2_ready= &status[STATUS_OFFSET+0];  

#endif

  int i=0;

  while(i < DATA_LEN){

    while(*b1_ready == 1) {
      ;
    }
    //Producer starting time stamp
    if(i==0){timeStamps[0] = *timer_ptr;}

#ifdef _MAINMEM_DF
    inval_dcache(); //invalidate the data cache
#endif

    //producing data for the buffer 1
    for ( int j = 0; j < BUFFER_SIZE; j++ ) {
        *(buffer1_ptr+j) = 1 ; // produce data
    }

    *b1_ready = 1;
    i += BUFFER_SIZE;

    while(*b2_ready == 1) {
      ;
    }

#ifdef _MAINMEM_DF
    inval_dcache(); //invalidate the data cache
#endif

    //producing data for the buffer 2
    for ( int j = 0; j < BUFFER_SIZE; j++ ) {
      *(buffer2_ptr+j) = 2 ; // produce data
    }

    *b2_ready = 1;
    i += BUFFER_SIZE;
  }

  //Producer finishing time stamp
  timeStamps[1] = *timer_ptr;

  return;
}

//intermediate
void intermediate(void *arg){
  int id = get_cpuid();

#ifdef _SSPM_DF
  //flag pointers 
  volatile int _SPM *b1_ready0= &spm_ptr[id-1];
  volatile int _SPM *b1_ready1= &spm_ptr[id];
  volatile int _SPM *b2_ready0= &spm_ptr[STATUS_OFFSET+id-1];
  volatile int _SPM *b2_ready1= &spm_ptr[STATUS_OFFSET+id];
  // data pointers
  volatile int _SPM *inbuffer1_ptr= &spm_ptr[STATUS_LEN+BUFFER_SIZE*(id-1)*2];
  volatile int _SPM *inbuffer2_ptr= &spm_ptr[STATUS_LEN+BUFFER_SIZE*(id-1)*2+BUFFER_SIZE];
  volatile int _SPM *outbuffer1_ptr= &spm_ptr[STATUS_LEN+BUFFER_SIZE*(id-1)*2+BUFFER_SIZE*2];
  volatile int _SPM *outbuffer2_ptr= &spm_ptr[STATUS_LEN+BUFFER_SIZE*(id-1)*2+BUFFER_SIZE*3];
#endif
#ifdef _MULTIOWN_DF
  //flag pointers 
  volatile int _SPM *b1_ready0= spm_base(FLAG_OFFSET)+id-1;
  volatile int _SPM *b1_ready1= spm_base(FLAG_OFFSET)+id;
  volatile int _SPM *b2_ready0= STATUS_OFFSET+spm_base(FLAG_OFFSET)+id-1;
  volatile int _SPM *b2_ready1= STATUS_OFFSET+spm_base(FLAG_OFFSET)+id;
  // data pointers
  volatile int _SPM *inbuffer1_ptr= spm_base(((id-1)*2+0));
  volatile int _SPM *inbuffer2_ptr= spm_base(((id-1)*2+1));
  volatile int _SPM *outbuffer1_ptr= spm_base(((id-1)*2+2));
  volatile int _SPM *outbuffer2_ptr= spm_base(((id-1)*2+3));
#endif
#ifdef _OWN_DF
  //flag pointers 
  volatile _UNCACHED int *b1_ready0= &status[id-1];
  volatile _UNCACHED int *b1_ready1= &status[id];
  volatile _UNCACHED int *b2_ready0= &status[STATUS_OFFSET+id-1];
  volatile _UNCACHED int *b2_ready1= &status[STATUS_OFFSET+id];
  // data pointers
  volatile int _SPM *inbuffer1_ptr= &spm_ptr[NEXT*(id-1)*2];
  volatile int _SPM *inbuffer2_ptr= &spm_ptr[NEXT*(id-1)*2+NEXT];
  volatile int _SPM *outbuffer1_ptr= &spm_ptr[NEXT*(id-1)*2+2*NEXT];
  volatile int _SPM *outbuffer2_ptr= &spm_ptr[NEXT*(id-1)*2+3*NEXT];
#endif
#ifdef _MAINMEM_DF
  //flag pointers 
  volatile _UNCACHED int *b1_ready0= &status[id-1];
  volatile _UNCACHED int *b1_ready1= &status[id];
  volatile _UNCACHED int *b2_ready0= &status[STATUS_OFFSET+id-1];
  volatile _UNCACHED int *b2_ready1= &status[STATUS_OFFSET+id];
  // data pointers
  volatile int *inbuffer1_ptr= &data[BUFFER_SIZE*(id-1)*2];
  volatile int *inbuffer2_ptr= &data[BUFFER_SIZE*(id-1)*2+BUFFER_SIZE];
  volatile int *outbuffer1_ptr= &data[BUFFER_SIZE*(id-1)*2+BUFFER_SIZE*2];
  volatile int *outbuffer2_ptr= &data[BUFFER_SIZE*(id-1)*2+BUFFER_SIZE*3];
#endif

int i=0;
int sum = 0;

while(i < DATA_LEN){
 
    while( !((*b1_ready0 == 1)&&(*b1_ready1==0))) {
      ;
    }

    #ifdef _MAINMEM_DF
    inval_dcache(); //invalidate the data cache
    #endif
    //consuming data from the buffer 1
    for ( int j = 0; j < BUFFER_SIZE; j++ ) {
      *(outbuffer1_ptr+j) = *(inbuffer1_ptr+j) +1 ;
    }

    *b1_ready0 =  0;
    *b1_ready1 =  1;
    i += BUFFER_SIZE;

    while( !((*b2_ready0 == 1)&&(*b2_ready1==0) )) {
      ;
    }

    #ifdef _MAINMEM_DF
    inval_dcache(); //invalidate the data cache
    #endif
    //consuming data from the buffer 1
    for ( int j = 0; j < BUFFER_SIZE; j++ ) {
      *(outbuffer2_ptr+j) = *(inbuffer2_ptr+j) +1 ;
    }

    *b2_ready1 =  1;
    *b2_ready0 =  0;
    i += BUFFER_SIZE;
  
}
return;

}


// Consumer
void consumer(void *arg) {

  int id = get_cpuid();

#ifdef _SSPM_DF
  //data
  volatile int _SPM  *buffer1_ptr = &spm_ptr[STATUS_LEN+BUFFER_SIZE*(id-1)*2];
  volatile int _SPM  *buffer2_ptr = &spm_ptr[STATUS_LEN+BUFFER_SIZE*(id-1)*2+BUFFER_SIZE];
  //flag pointers
  volatile int _SPM *b1_ready= &spm_ptr[id-1];
  volatile int _SPM *b2_ready= &spm_ptr[STATUS_OFFSET+id-1];
#endif
#ifdef _MULTIOWN_DF
  //data buffers
  volatile int _SPM  *buffer1_ptr = spm_base(((id-1)*2));
  volatile int _SPM  *buffer2_ptr = spm_base(((id-1)*2+1));
  //flag pointers
  volatile int _SPM *b1_ready= spm_base(FLAG_OFFSET)+id-1;
  volatile int _SPM *b2_ready= STATUS_OFFSET+spm_base(FLAG_OFFSET)+id-1;
#endif
#ifdef _OWN_DF
  //flag pointers
  volatile _UNCACHED int *b1_ready= &status[id-1];
  volatile _UNCACHED int *b2_ready= &status[STATUS_OFFSET+id-1];
  //data buffers
  volatile int _SPM  *buffer1_ptr = &spm_ptr[NEXT*(id-1)*2];
  volatile int _SPM  *buffer2_ptr = &spm_ptr[NEXT*(id-1)*2+NEXT];
#endif
#ifdef _MAINMEM_DF
  //data
  volatile int *buffer1_ptr = &data[BUFFER_SIZE*(id-1)*2];
  volatile int *buffer2_ptr = &data[BUFFER_SIZE*(id-1)*2+BUFFER_SIZE];
  //flag pointers
  volatile _UNCACHED int *b1_ready= &status[id-1];  
  volatile _UNCACHED int *b2_ready= &status[STATUS_OFFSET+id-1];  

#endif

  int i=0; 
  int sum = 0;

  while(i < DATA_LEN){
 
    while(*b1_ready == 0) {
      ;
    }
        
    //Consumer starting time stamp
    if(i==0){timeStamps[2] = *timer_ptr;}

#ifdef _MAINMEM_DF
    inval_dcache(); //invalidate the data cache
#endif

    //consuming data from the buffer 1
    for ( int j = 0; j < BUFFER_SIZE; j++ ) {
      sum += *(buffer1_ptr+j);
    }

    *b1_ready = 0;
    i += BUFFER_SIZE;

    while(*b2_ready == 0) {
      ;
    }

#ifdef _MAINMEM_DF
    inval_dcache(); //invalidate the data cache
#endif

    //consuming data from the buffer 2
    for (int j = 0; j < BUFFER_SIZE; j++ ) {
      sum += *(buffer2_ptr+j);
    }
    *b2_ready = 0;
    i += BUFFER_SIZE;

  }
   //Consumer finishing time stamp
   timeStamps[3] = *timer_ptr;
   return;
}



int main() {

int i,j;  

#ifdef _MAINMEM_DF
  for(i=0; i<STATUS_LEN; ++i) {   
    status[i]=0;
  }
#endif
#ifdef _SSPM_DF
  for(i=0; i<STATUS_LEN; ++i) {   
    spm_ptr[i]=0;
  }
#endif
#ifdef _OWN_DF
  for(i=0; i<STATUS_LEN; ++i) {   
    status[i]=0;
  }
#endif
#ifdef _MULTIOWN_DF
  // We statically assign the SPMs so we simply set the ownership
  int schedule=3;
  for(i=0; i<(CNT-1); ++i) { 
    int k=i*2;
    spm_sched_wr(k,schedule);
    spm_sched_wr(k+1,schedule);
    schedule = schedule*2;
  }
  // Flags are put into a 3rd SPM instead of main memory
  //data_ready1 = spm_base(2);
  //data_ready2 = spm_base(2)+1;
#endif

  int id = get_cpuid(); 
  int cnt = get_cpucnt();
  printf("Total %d Cores\n",cnt); // print core count

// threat creation for a single intermediate node
#ifdef _SINGLE_RELAY 
  corethread_create(1, &intermediate, NULL);
  corethread_create(2, &consumer, NULL);
  producer(); 
  for(j=1; j<3; ++j) { 
    void *dummy;
    corethread_join(j,&dummy);
  }
#endif
// threat creation for several intermediate nodes
#ifdef _MULTI_RELAY

  for (i=1; i<cnt-1; ++i) {
    corethread_create(i, &intermediate, NULL);
    printf("thread %d is created \n",i); 
  }

  corethread_create(cnt-1, &consumer, NULL); 
  producer();
  
   for(j=1; j<cnt; ++j) { 
    void *dummy;
    corethread_join(j,&dummy);
  }
#endif

  printf("End-to-End Latency is %d clock cycles\n    \
         for %d words of bulk data\n   \
         and %d of buffer size\n", timeStamps[3]-timeStamps[0],DATA_LEN,BUFFER_SIZE);
  int cycles = timeStamps[3]-timeStamps[0];
  printf("measure pc_"_NAME": %d.%d cycles per word for %d words in %d words buffer\n",
    cycles/DATA_LEN, cycles*10/DATA_LEN%10, DATA_LEN, BUFFER_SIZE);


}
