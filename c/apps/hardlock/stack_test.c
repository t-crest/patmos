#include <stdio.h>
#include "libcorethread/corethread.h"

#define TIMER_CLK_LOW *((volatile _IODEV int *) (PATMOS_IO_TIMER + 0x4))

//#define _HARDLOCK_

#ifdef _HARDLOCK_
#include "stack_hardlock.c"
#else
#include "stack_lock_free.c"
#endif

#define ITERATIONS 100
#define NODES_PER_CORE 2

int test(head_t head)
{
	int ret = 0;
	for(int i = 0; i < ITERATIONS; i++) {
		node_t * node1 = pop(head);
		if(!node1)
			return -1;
		
		node_t * node2 = pop(head);
		if(!node2)
			return -1;
		
		push(head, node1);
		push(head, node2);
	}
	return 0;
}

void worker_init(void* arg) {
  int ret = test((head_t)arg);
  corethread_exit((void *)ret);
  return;
}

node_t nodes[128];
int main()
{
	int cpucnt = get_cpucnt();
	head_t head = (_iodev_ptr_t)PATMOS_IO_CASPM;

	void * arg = (void *)head;
	const int nodecnt = cpucnt*NODES_PER_CORE;
	
	// Push all nodes on the stack
	for(int i = 0; i < nodecnt; i++) {
		nodes[i].val = i;
		push(head,&nodes[i]);
	}
	asm("nop");
	int start = TIMER_CLK_LOW;
	int cores = cpucnt;
	
	for(int i = 1; i < cores; i++)
		corethread_create(i,&worker_init,arg);
	int res = test(head);
	
	for(int i = 1; i < cores; i++) {
		void * _res;
		corethread_join(i, &_res);
		res |= (int)_res;
	}
	int stop = TIMER_CLK_LOW;
	asm("nop");
	
	printf("Finished in %d cycles\n",stop-start);
	
	int sum = 0;
	
	for(int i = 0; i < nodecnt; i++) {
		node_t * node = pop(head);
		if(!node) {
			printf("Only %d nodes on stack. Should be %d\n",i,nodecnt);
			return -1;
		}
		sum += node->val;
	}
	
	int expsum = ((cpucnt*NODES_PER_CORE)-1)*((cpucnt*NODES_PER_CORE)/2);
	if(sum != expsum)
		printf("Error with sum. Expected: %d Actual:%d\n",expsum,sum);
	
	if(res != 0)
			printf("Error %d\n",res);
	return res;
}