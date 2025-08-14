#include <stdio.h>
#include <machine/patmos.h>
#include "../../libcorethread/corethread.h"
#include "testing_settings.h"


void core0(){
	#include "accesses_0.txt.c"	
}
void core1(){
	#include "accesses_1.txt.c"	
}
void core2(){
	#include "accesses_2.txt.c"	
}
void core3(){
	#include "accesses_3.txt.c"	
}

#define CORES 4

void* test_fns[CORES] = {(void*)core0, (void*)core1, (void*)core2, (void*)core3};

volatile int core_timing[CORES];
volatile _UNCACHED int core_status[CORES]; // 0: start, 1: ready, 2: done
volatile _UNCACHED int start = 0;

static inline unsigned long long get_cpu_cycles2(void) {
  unsigned clo, chi;
  _iodev_ptr_t hi_clock = (_iodev_ptr_t)(__PATMOS_TIMER_HICLK);

  // Prevent the compiler from moving memory operation across this boundary
  asm volatile ("" : : : "memory");  
  
  // Order is important here
  asm volatile (
    "lwl %0 = [%2 + 1];"
    "lwl %1 = [%2];"
    :
    "=&r" (clo),
    "=r" (chi)
    :
    "r" (hi_clock)
    :
  );

  // Prevent the compiler from moving memory operation across this boundary
  asm volatile ("" : : : "memory");

  return (((unsigned long long) chi) << 32) | clo;
}

void run_core(void *arg) {
	void (*core_fn)() = arg;
	
	int core_id = get_cpuid();
	int start, end;
	
	core_status[core_id] = 1;
	while(!start){} // wait until the main core issues the start command
		
	asm volatile ("nop;" : : : "memory");
	start = get_cpu_cycles2();
	asm volatile ("nop;" : : : "memory");
	
	core_fn();
	
	asm volatile ("nop;" : : : "memory");
	end = get_cpu_cycles2();
	asm volatile ("nop;" : : : "memory");
		
	core_timing[core_id] = end - start;
	core_status[core_id] = 2;
}

int main() {

	for(int i = 0; i<CORES; i++) {
		core_timing[i] = 0;
		core_status[i] = 0;
	}
	start = 0;
	
	for(int i = 1; i<CORES_RUNNING; i++) {
		corethread_create(i, &run_core, test_fns[i]);
	}
	
	// Wait on other cores to get ready
	int wait;
	do {
		wait = 0;
		for(int i = 1; i<CORES_RUNNING; i++) {
			wait |= core_status[i] != 1; 
		}
	} while(wait);
	
	
	start = 1;
	
	run_core(core0);
	
	// Wait on other cores to finish
	do {
		wait = 0;
		for(int i = 1; i<CORES_RUNNING; i++) {
			wait |= core_status[i] != 2; 
		}
	} while(wait);
	
	printf("%d,%d,%d,%d\n", core_timing[0], core_timing[1], core_timing[2], core_timing[3]);
}

