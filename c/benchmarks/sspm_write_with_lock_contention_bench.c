#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/rtc.h>
#include "libcorethread/corethread.c"
#include "libmp/mp.h"
#include "libmp/mp_internal.h"
#include "libsspm/sspm_properties.h"
#include "libsspm/atomic.h"

#define MP_CHAN_NUM_BUF 2
#define MP_CHAN_BUF_SIZE 40

const int NOC_MASTER = 0;

const int TIMES = 1000;
int cycles[TIMES];

void slave(void* args){
	volatile _SPM lock_t* l = (volatile _SPM lock_t*) (LOWEST_SPM_ADDRESS+4);

	//We inline the lock, so that we maximize the amount of 
	//tries the core can make
	int syncAddr = SCHEDULE_SYNC;
	lock_t lock_value;
	
	asm volatile(
				"loop:"
				"lwl $r0 = [%[sync]];" 			// Sync to TDMA
				"lwl %[lock_value] = [%[lock]];"// Load lock value
				"swl [%[lock]] = %[LOCKED];"	// write lock = LOCKED
				"mov $p1 = %[lock_value];"		
				"($p1) brnd loop;"				// Loop if was LOCKED
				: [lock_value] "=r" (lock_value), [lock] "+rm" (l)
				: [sync] "r" (syncAddr), [LOCKED] "r" (LOCKED)
				: "$r0", "$p1"
	);

	release(l);	

}

int main(){

	int start, end;
	volatile _SPM int* flag = (volatile _SPM int*) LOWEST_SPM_ADDRESS;
	volatile _SPM lock_t* l = (volatile _SPM lock_t*) (LOWEST_SPM_ADDRESS+4);
		
	for(int i = 0; i<NOC_CORES; i++){
		lock(l);
		for(int c = 1; c <= i; c++){
			corethread_create(&c, &slave, NULL);
		}
		
		asm volatile ("" : : : "memory");
		start = get_cpu_cycles();
		asm volatile ("" : : : "memory");

		for(int k = 0; k<TIMES; k++){
			*flag = 1;
		}
		
		asm volatile ("" : : : "memory");
		end = get_cpu_cycles();
		asm volatile ("" : : : "memory");
		
		release(l);
		
		int res;
		for(int c = 1; c <= i; c++){
			corethread_join(c, (void **) &res);
		}
		printf("Traffic cores: %d\n", i);
		printf("%d\n", end-start);
	}
	
	return 0;
}








