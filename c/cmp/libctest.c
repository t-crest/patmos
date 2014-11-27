#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include "libcorethread/corethread.h"

#define ALLOC_SIZE  (16*get_cpuid())
#define ALLOC_COUNT (1*get_cpuid())

void alloc_test(void* param) {
  for (int i = 0; i < ALLOC_COUNT; i++) {
      volatile char *foo = malloc(ALLOC_SIZE);
      for (int k = 0; k < ALLOC_SIZE; k++) {
        foo[k] = get_cpuid();
      }
      free((char *)foo);
    }
}

int main(int argc, char **argv)
{
  int slave_param = 1;

  for(int i = 0; i < get_cpucnt(); i++) {
    if (i != get_cpuid()) {
      corethread_t ct = (corethread_t)i;
      //printf("Corethread %d ",i);
      if(corethread_create(&ct,&alloc_test,(void*)slave_param) != 0){
        //printf("not ");
        printf("Corethread %d not created\n",i);
      }
      //printf("created\n");
    }
  }

  puts("Hello, World!");
  puts("This is Major Tom to Ground Control.");
  puts("Ten-nine-eight-seven-six-five-four-three-two-one.");

  int* ret;
  for (int i = 0; i < get_cpucnt(); ++i) {
    if (i != get_cpuid()) {
      corethread_join((corethread_t)i,(void**)&ret);
      //printf("Slave %d joined\n",i);
    }
  }
  
  
  return 0;
}
