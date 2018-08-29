#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>


#include "../../libcorethread/corethread.h"
#include "spmpool.h"

#define DEBUG
#define LEN 10

// The main function for the other threads on the another cores
void work(void* arg) {

  int id = get_cpuid();
  int spmid = (int)arg;
  
  _iodev_ptr_t data_ptr = spm_base(spmid);

  for (int i=0; i<LEN; ++i) {
    *(data_ptr+i) = i; 
  }
  
  corethread_exit((void *)spmid);
}

int main() {

#ifdef DEBUG
  printf("Program Start\n");
#endif
  int cnt = get_cpucnt();
  int spmids[10];
  for (int i=1; i< cnt; ++i) {

    int spmid = spm_req();

#ifdef DEBUG
      printf("SPMID:%d\n",spmid);
#endif

    spmids[i] = spmid;
    spm_sched_wr(spmid, (1 << i) + 1);

    corethread_create(i, &work, (void *)spmid); 
  }

  for (int i=1; i< cnt; ++i) {

    void * res;
    corethread_join(i, &res);
#ifdef DEBUG
    printf("Joined Thread:%d\n",i);
#endif
  }
  for (int i=1; i<cnt; ++i) {
    int spmid = spmids[i];
#ifdef DEBUG
    printf("SPMID:%d\n",spmid);
#endif

    _iodev_ptr_t data_ptr = spm_base(spmid);
    for (int j=0; j<LEN; ++j) {
      int data = *(data_ptr+j);
#ifdef DEBUG
      printf("%d",data);
#endif
    }
#ifdef DEBUG
    printf("\n");
#endif
  }
  return 0;
}
