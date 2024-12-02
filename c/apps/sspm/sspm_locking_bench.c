#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/rtc.h>
#include "../../libcorethread/corethread.h"
#include "../../libmp/mp.h"
#include "../../libmp/mp_internal.h"
#include "sspm_properties.h"
#include "atomic.h"
#include "led.h"

const int TIMES = 1000;
volatile _UNCACHED int ready;

void slave(void* args){
	led_on();
	volatile _SPM lock_t* l = (volatile _SPM lock_t*) (LOWEST_SSPM_ADDRESS+4);

	//We inline the lock, so that we maximize the amount of 
	//tries the core can make
	int syncAddr = SCHEDULE_SYNC;
	lock_t lock_value;
	
	asm volatile(
				"loop_slave:"
				"lwl $r0 = [%[sync]];" 			// Sync to TDMA
				"lwl %[lock_value] = [%[lock]];"// Load lock value
				"swl [%[lock]] = %[LOCKED];"	// write lock = LOCKED
				"mov $p1 = %[lock_value];"		
				"($p1) brnd loop_slave;"				// Loop if was LOCKED
				: [lock_value] "=r" (lock_value), [lock] "+rm" (l)
				: [sync] "r" (syncAddr), [LOCKED] "r" (LOCKED)
				: "$r0", "$p1"
	);

	release(l);	
	led_off();
}

int main(){
	led_on();
	int start, end;
	int syncAddr = SCHEDULE_SYNC;

	volatile _SPM lock_t* l = (volatile _SPM lock_t*) LOWEST_SSPM_ADDRESS;
	release(l);	
	volatile _SPM lock_t* l2 = (volatile _SPM lock_t*) (LOWEST_SSPM_ADDRESS+4);
	release(l2);

	for(int i = 0; i<NOC_CORES; i++){
		lock(l2);
		for(int c = 1; c <=i;c++){
			corethread_create(c, &slave, NULL);
		}		

		printf("Traffic cores: %d\n", i);
		
		asm volatile ("" : : : "memory");
		start = get_cpu_cycles();
		asm volatile ("" : : : "memory");

		//We inline the lock and loop, so that we minimize the cycle count

		lock_t lock_value;
		asm volatile(
					"li $r1 = 1;"
					"mov $r2 = %[max];"
					"loop_main:"
					"lwl $r0 = [%[sync]];" 			// Sync to TDMA
					"lwl %[lock_value] = [%[lock]];"// Load lock value
					"swl [%[lock]] = %[LOCKED];"	// write lock = LOCKED
				    "cmpneq $p1 = $r1, $r2;"  // Test whether to loop again
					"($p1) br loop_main;"			// Loop if needed
					"add $r1 = $r1, 1;"			// Increment counter
					"nop;"           				// Reserved for branch delay
					: [lock_value] "=r" (lock_value), [lock] "+rm" (l)
					: [sync] "r" (syncAddr), [LOCKED] "r" (LOCKED), [max] "r" (TIMES)
					: "$r0","$r1","$r2", "$p1"
		);
		
		asm volatile ("" : : : "memory");
		end = get_cpu_cycles();
		asm volatile ("" : : : "memory");
		
		release(l2);
		int res;
		for(int c = 1; c <=i; c++){
			corethread_join(c, (void **) &res);
		}
		printf("Cycles: %d\n", end-start);
	}
	led_off();
	return 0;
}








