
#include "libcorethread/corethread.h"
#include <stdio.h>

#define TIMER_CLK_LOW *((volatile _IODEV int *) (PATMOS_IO_TIMER + 0x4))
#define TIMER_US_LOW *((volatile _IODEV int *) (PATMOS_IO_TIMER + 0xc))

#include <machine/patmos.h>
#define RTTM ((_iodev_ptr_t) 0xE80C0000)
#define RTTM_COMMIT ((_SPM char*) 0xE80CFFFC)

volatile _IODEV int *dead_ptr = (volatile _IODEV int *) PATMOS_IO_DEADLINE;
const int BACKOFF_MIN = 10;
const int BACKOFF_MAX = 100;
_UNCACHED int flag = 0;

const int iter = 10;
int writer() {
	while(!flag) {asm("");}
	int tries = 0;
	int cnt = 0;
	int cnt_tmp = 0;
	_iodev_ptr_t write_ptr = RTTM;
	_iodev_ptr_t val_ptr = RTTM+15;
	while(cnt < iter) {
		do {
			*dead_ptr = 100;
			tries++;
			cnt_tmp = (*write_ptr);
			if(cnt_tmp < iter) {
				*(val_ptr+cnt_tmp) = cnt_tmp;
				*write_ptr = ++cnt_tmp;
			}
			*dead_ptr;
		} while(*RTTM_COMMIT != 0);
		tries--;
	  cnt = cnt_tmp;
	}
  return tries;
}

int reader() {
	int tries = 0;
	int cnt = 0;
	int cnt_tmp = 0;
	int sum = 0;
	_iodev_ptr_t write_ptr = RTTM;
	_iodev_ptr_t read_ptr = RTTM+1;
	_iodev_ptr_t val_ptr = RTTM+15;
	while(cnt < iter) {
		int val_tmp;
		
		do {
			tries++;
			cnt_tmp = *read_ptr;
			val_tmp = 0;
			if(cnt_tmp < *write_ptr) {
			  val_tmp = *(val_ptr+cnt_tmp);
				*read_ptr = ++cnt_tmp;
			}
		} while(*RTTM_COMMIT != 0);
		tries--;
		sum += val_tmp;
		cnt = cnt_tmp;
	}
  return sum;
}

void reader_init(void* arg) {
  int ret = reader();
  corethread_exit((void *)ret);
  return;
}

void writer_init(void* arg) {
  int ret = writer();
  corethread_exit((void *)ret);
  return;
}

int main() {
	
	int cpucnt = get_cpucnt();
	printf("cpus:%d\n",cpucnt);
	
	int dum;
	for(int i = 1; i < cpucnt; i++) {
		corethread_create(i,&writer_init,(void *)i);
	}
	flag = 1;
	int sum = reader();
	
	int retries = 0;
	for(int i = 1; i < cpucnt; i++) {
		int _res;
		corethread_join(i, (void **)&_res);
		retries += _res;
	}
	
	printf("Sum: %d Retries: %d\n",sum, retries);
	
	return sum;
}




