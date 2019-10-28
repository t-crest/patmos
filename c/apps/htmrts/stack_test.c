#include <stdio.h>
#include "libcorethread/corethread.h"

#define TIMER_CLK_LOW *((volatile _IODEV int *) (PATMOS_IO_TIMER + 0x4))

#ifdef _HARDLOCK_
#define NAME "hardlock"
#include "stack_hardlock.c"
#define STACK PATMOS_IO_SPM
#elif _CAS_
#define NAME "cas"
#include "stack_cas.c"
#define STACK PATMOS_IO_SPM
#else
#define NAME "lock-free"
#include "stack_lock_free.c"
#define STACK HTMRTS_BASE
#endif

#ifndef MAX_CPU_CNT
#define MAX_CPU_CNT 8
#endif

#ifndef ITERATIONS
#define ITERATIONS 100
#endif

#ifndef ELEMENTS_PER_CORE
#define ELEMENTS_PER_CORE 2
#endif

_UNCACHED int start_flag = 0;

int test(stack_t * stack_ptr)
{
	while(start_flag == 0) {asm("");}
	element_t * elms[ELEMENTS_PER_CORE];
	int ret = 0;
	for(int i = 0; i < ITERATIONS; i++) {
		for(int j = 0; j < ELEMENTS_PER_CORE; j++) {
			element_t * element_ptr = pop(stack_ptr);
			if(!element_ptr)
				return -1;
			elms[j] = element_ptr;
		}
		
		for(int j = 0; j < ELEMENTS_PER_CORE; j++)
			push(stack_ptr, elms[j]);
	}
	return 0;
}

void worker_init(void* arg) 
{
	int ret = test((stack_t *)arg);
	corethread_exit((void *)ret);
	return;
}

element_t elements[128];

int main()
{
	int cpucnt = get_cpucnt();
	if(MAX_CPU_CNT < cpucnt)
		cpucnt = MAX_CPU_CNT;
	
	printf("Stack test using \n\tsynchronization:%s\n\tcores:%d\n\titerations:%d\n\telements per core:%d\n",NAME,cpucnt,ITERATIONS,ELEMENTS_PER_CORE);
	
	stack_t * stack_ptr = (stack_t *)STACK;
	
	intialize(stack_ptr);

	void * arg = (void *)stack_ptr;
	const int elementcnt = cpucnt*ELEMENTS_PER_CORE;
	
	// Push all elements on the stack
	for(int i = 0; i < elementcnt; i++) {
		intialize_element(&elements[i],i);
		push(stack_ptr,&elements[i]);
	}
	
	for(int i = 1; i < cpucnt; i++)
		corethread_create(i,&worker_init,arg);
	
	asm volatile ("" : : : "memory");
	start_flag = 1;
	int start = TIMER_CLK_LOW;
	asm volatile ("" : : : "memory");
	
	int res = test(stack_ptr);
	
	for(int i = 1; i < cpucnt; i++) {
		void * _res;
		corethread_join(i, &_res);
		res |= (int)_res;
	}
	
	asm volatile ("" : : : "memory");
	int stop = TIMER_CLK_LOW;
	asm volatile ("" : : : "memory");
	
	printf("Finished in %d cycles\n",stop-start);
	
	int sum = 0;
	
	for(int i = 0; i < elementcnt; i++) {
		element_t * element_ptr = pop(stack_ptr);
		if(!element_ptr) {
			printf("Only %d elements on stack. Should be %d\n",i,elementcnt);
			return -1;
		}
		sum += element_ptr->val;
	}
	
	int expsum = ((cpucnt*ELEMENTS_PER_CORE)-1)*((cpucnt*ELEMENTS_PER_CORE)/2);
	if(sum != expsum)
		printf("Error with sum. Expected: %d Actual:%d\n",expsum,sum);
	
	if(res != 0)
			printf("Error %d\n",res);
	return res;
}