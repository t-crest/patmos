#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>


#include "../../libcorethread/corethread.h"
#include "spmpool.h"

#define DEBUG

// The main function for the other threads on the another cores
void work(void* arg) {

  int id = get_cpuid();
  int spmid = (int)arg;

  for (int i=0; i<100; ++i) {
    spm_write(spmid,i,i); 
  }
  
  corethread_exit((void *)spmid);
}

int main() {
#ifdef DEBUG
  printf("Program Start\n");
#endif
  int cnt = get_cpucnt();
  int spmids[10];
  int dummy = spm_request();
#ifdef DEBUG
      printf("SPMID:%d\n",dummy);
#endif
  for (int i=1; i< cnt; ++i) {

    int spmid = spm_request();
#ifdef DEBUG
      printf("SPMID:%d\n",spmid);
#endif
    spmids[i] = spmid;
    schedule_write(spmid, (1 << i) + 1) ;
    corethread_create(i, &work, (void *)spmid); 
  }

  for (int i=1; i< cnt; ++i) {
    void * res;
    corethread_join(i, &res);
  }
#ifdef DEBUG
  printf("Threads Joined\n");
#endif
  for (int i=1; i<cnt; ++i) {
    int spmid = spmids[i];
#ifdef DEBUG
    printf("SPMID:%d\n",spmid);
#endif
    for (int j=0; j<100; ++j) {
      int data = spm_read(spmid,j);
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
