#include <stdio.h>
#include "libcorethread/corethread.h"

#define TIMER_CLK_LOW *((volatile _IODEV int *) (PATMOS_IO_TIMER + 0x4))

#ifdef _HARDLOCK_
#define NAME "hardlock"
#include "queue_hardlock.c"
#define QUEUE PATMOS_IO_SPM
#elif _CAS_
#define NAME "cas"
#include "queue_cas.c"
#define QUEUE PATMOS_IO_SPM
#else
#define NAME "lock-free"
#include "queue_lock_free.c"
#define QUEUE HTMRTS_BASE
#endif

#ifndef MAX_CPU_CNT
#define MAX_CPU_CNT 8
#endif

#ifndef ELEMENTS_PER_CORE
#define ELEMENTS_PER_CORE 60
#endif

_UNCACHED int start_flag = 0;

int writer(element_t * elements) 
{
	queue_t * queue_ptr = (queue_t *)QUEUE;
	while(start_flag == 0) {asm("");}
	
	for(int i = 0; i < ELEMENTS_PER_CORE; i++)
		enqueue(queue_ptr, elements+i);

	return 0;
}

int reader(queue_t * queue_ptr) 
{
	int sum = 0;
	while(start_flag == 0) {asm("");}

	for(int i = 0; i < ELEMENTS_PER_CORE; i++) {
		element_t * element_ptr;
		do
		{
			element_ptr = dequeue(queue_ptr);
		} while(!element_ptr);
		// Don't have to commit the following read
		sum += element_ptr->val;
	}
	
	return sum;
}

void writer_init(void* arg) 
{
	int ret = writer((element_t *)arg);
	corethread_exit((void *)ret);
	return;
}

void reader_init(void* arg) 
{
	int ret = reader((queue_t *)arg);
	corethread_exit((void *)ret);
	return;
}

int main()
{
	int cpucnt = get_cpucnt();
	if(MAX_CPU_CNT < cpucnt)
		cpucnt = MAX_CPU_CNT;
	
	int writers = cpucnt/2;
	int readers = writers;
	
	int elementcnt = writers*ELEMENTS_PER_CORE;
	
	printf("Queue test using \n\tsynchronization:%s\n\tcores:%d\n\twriters:%d\n\treaders:%d\n\telements per writer:%d\n",NAME,cpucnt,writers,readers,ELEMENTS_PER_CORE);
	
	queue_t * queue_ptr = (queue_t *)QUEUE;
	element_t * elements = (element_t *)(queue_ptr+1);
	intialize(queue_ptr);
	
	// Initialize all elements
	for(int i = 0; i < elementcnt; i++)
		initialize_element(elements+i,i);
	
	for(int i = 1; i < writers+1; i++) {
		void * arg = (void *)(elements+((i-1)*ELEMENTS_PER_CORE));
		corethread_create(i,&writer_init,arg);
	}
	
	for(int i = writers+1; i < cpucnt; i++) {
		void * arg = (void *)queue_ptr;
		corethread_create(i,&reader_init,arg);
	}
	
	void * arg = (void *)queue_ptr;
	asm volatile ("" : : : "memory");
	int start = TIMER_CLK_LOW;
	start_flag = 1;
	asm volatile ("" : : : "memory");
	
	int sum = 0;
	if(writers+readers == cpucnt)
		sum += reader(queue_ptr);
	
	for(int i = 1; i < writers+1; i++) {
		void * _res;
		corethread_join(i, &_res);
	}
	
	for(int i = writers+1; i < cpucnt; i++) {
		void * _res;
		corethread_join(i, &_res);
		sum += (int)_res;
	}
	int stop = TIMER_CLK_LOW;
	asm volatile ("" : : : "memory");
	
	printf("Finished in %d cycles\n",stop-start);
	
	int expsum;
	if(elementcnt&0x1)
		expsum = ((elementcnt-1)/2)*(elementcnt-2)+(elementcnt-1);
	else
		expsum = (elementcnt/2)*(elementcnt-1);
	
	if(sum != expsum) {
		printf("Error with sum. Expected: %d Actual:%d\n",expsum,sum);
		// Dummy calls to enable analysis
		enqueue(queue_ptr,NULL);
		dequeue(queue_ptr);
		return -1;
	}
	
	return 0;
}