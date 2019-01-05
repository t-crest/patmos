#include <stdio.h>
#include "libcorethread/corethread.h"

#define TIMER_CLK_LOW *((volatile _IODEV int *) (PATMOS_IO_TIMER + 0x4))

#ifdef _HARDLOCK_
#define NAME "hardlock"
#include "stack_hardlock.c"
#else
#define NAME "lock-free"
#include "stack_lock_free.c"
#endif

#ifndef ITERATIONS
#define ITERATIONS 100
#endif
#define ELEMENTS_PER_CORE 2

int test(top_t top)
{
	int ret = 0;
	for(int i = 0; i < ITERATIONS; i++) {
		element_t * element1 = pop(top);
		if(!element1)
			return -1;
		
		element_t * element2 = pop(top);
		if(!element2)
			return -1;
		
		push(top, element1);
		push(top, element2);
	}
	return 0;
}

void worker_init(void* arg) {
  int ret = test((top_t)arg);
  corethread_exit((void *)ret);
  return;
}

element_t elements[128];

int main()
{
	int cpucnt = get_cpucnt();
	
	printf("Using \n\tlock:%s\n\tcores:%d\n\titerations:%d\n",NAME,cpucnt,ITERATIONS);
	
	top_t top = (_iodev_ptr_t)PATMOS_IO_CASPM;

	void * arg = (void *)top;
	const int elementcnt = cpucnt*ELEMENTS_PER_CORE;
	
	// Push all elements on the stack
	for(int i = 0; i < elementcnt; i++) {
		elements[i].val = i;
		push(top,&elements[i]);
	}
	asm("nop");
	int start = TIMER_CLK_LOW;
	int cores = cpucnt;
	
	for(int i = 1; i < cores; i++)
		corethread_create(i,&worker_init,arg);
	int res = test(top);
	
	for(int i = 1; i < cores; i++) {
		void * _res;
		corethread_join(i, &_res);
		res |= (int)_res;
	}
	int stop = TIMER_CLK_LOW;
	asm("nop");
	
	printf("Finished in %d cycles\n",stop-start);
	
	int sum = 0;
	
	for(int i = 0; i < elementcnt; i++) {
		element_t * element = pop(top);
		if(!element) {
			printf("Only %d elements on stack. Should be %d\n",i,elementcnt);
			return -1;
		}
		sum += element->val;
	}
	
	int expsum = ((cpucnt*ELEMENTS_PER_CORE)-1)*((cpucnt*ELEMENTS_PER_CORE)/2);
	if(sum != expsum)
		printf("Error with sum. Expected: %d Actual:%d\n",expsum,sum);
	
	if(res != 0)
			printf("Error %d\n",res);
	return res;
}