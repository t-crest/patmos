/*
    This is a multicore application that each core produces and consumes 1KB of bulk data .

    Author: Oktay Baris
    Copyright: DTU, BSD License
*/
#include <stdio.h>

#include "include/patio.h"
#include "libcorethread/corethread.h"

#define DATA_LEN 256 // words

volatile int _SPM *spm_ptr = (( volatile _SPM int *)0xE8000000);

//Input Data 
static int inData[ DATA_LEN ] =
{
 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
   1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
     1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
      1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
       1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
        1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
         1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
          1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
           1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
            1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
             1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
              1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16
              
};


// write the bulk data to the SPM memory
void memRead(void *source){

  int *src=source;

  for (int i=0; i<DATA_LEN; ++i) {
    spm_ptr[i] = src[i];
  }
return;
}


// The main function for the threads on the cores
void work(void *arg) {

 volatile int _SPM  *output_ptr, *input_ptr;

  int id = get_cpuid();
  int cnt = get_cpucnt();
  
  input_ptr= &spm_ptr[DATA_LEN*(id-1)];
  output_ptr= &spm_ptr[DATA_LEN*(id)];

    for ( int j = 0; j < DATA_LEN; j++ ) {
        output_ptr[j] = input_ptr[j]*id;
    }

  return;
}



int main() {

  unsigned i,j;

  int id = get_cpuid(); // id=0
  int cnt = get_cpucnt();

 int parameter = 1;

  printf("Total %d Cores\n",get_cpucnt()); // print core count

  printf("Writing the data to the SPM ...\n"); 

  // Copy the bulk data to the SPM
  memRead(inData);

  for (i=1; i<cnt; ++i) {
    int core_id = i; // The core number
    corethread_create(core_id, &work, &parameter); 
  }

  for(i=1; i<cnt; ++i) { 
    int parameter = 1;
    corethread_join(i,&parameter);
  }

  printf("Computation is Done !!\n");

  //Debug

  for (int i=0; i<DATA_LEN*(cnt-1); ++i) {
        printf("The Output Data %d is %d \n",i, spm_ptr[DATA_LEN+i]);
   }
  

}
