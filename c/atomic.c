#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "atomic.h"
#include <machine/exceptions.h>


int inline try_lock( volatile _SPM lock_t *l){
	
	int syncAddr = SCHEDULE_SYNC;
	lock_t lock_value;
	
	intr_disable();
	asm volatile(
				"lwl $r0 = [%[sync]];" 	// Sync to TDMA
				"lwl %[lock_value] = [%[lock]];"	// Load lock value
				"swl [%[lock]] = %[LOCKED];"	// write lock = LOCKED
				: [lock_value] "=r" (lock_value), [lock] "+rm" (l)
				: [sync] "r" (syncAddr), [LOCKED] "r" (LOCKED)
				: "$r0"
	);
	intr_enable();

	return  lock_value == OPEN ;
}

void inline lock(volatile _SPM lock_t *l){
	// This loop is needlessly slow because the compiler 
	// pushes some values to main memory with each iteration.
	// Look at the generated assembly for details.
	while( !try_lock(l) ){}
}

void release( volatile _SPM lock_t *l ){
	*l = OPEN;
}
