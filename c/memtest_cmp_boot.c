/*
	Author: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
	Copyright: DTU, BSD License
*/
#include "cmpboot.h"
#include "patio.h"
#include <machine/spm.h>
#include <machine/patmos.h>

// we assume 2 MB memory, less than 400 KB for program,
// heap, and stack
// #define LENGTH (2000000-400000)/4
#define LENGTH 200000/4
#define CNT 100

// Start the memory test some bytes above heap start
// as stdio needs the heap for buffers (40000 bytes reserved)
// Now hardcoded for merging with bootable
extern char _end;
// #define TEST_START ((volatile _UNCACHED int *) (&_end)+10000)
// #define TEST_START ((volatile _UNCACHED int *) 250000)
// MS: better test with cache
#define TEST_START ((volatile int *) 250000)


int main() {
  // setup stack frame and stack cache.
  asm volatile ("mov $r31 = %0;" // initialize shadow stack pointer"
                "mts $ss  = %1;" // initialize the stack cache's spill pointer"
                "mts $st  = %1;" // initialize the stack cache's top pointer"
                : : "r" (&_shadow_stack_base),
                  "r" (&_stack_cache_base));

	int res;
	int error = 0;
	int test = 0;

// printf("%d %d\n", (int) TEST_START, (int) &_end);

	if (CORE_ID == 0) {
		// MS: does the following reading from uninitialized memory
		// make sense?
//		for (int i=0; i<=LENGTH; i++){ // Read from main memory
//			res = *(TEST_START+i);
//			if (res != 0){	// If data is not what we expect write error
//				error++;
//			}
//		}
//		if (error != 0){
//			WRITE("TEST_STARTORY uninitialized\n", 21);
//		}
//		error = 0;

		for (int k = 0; k < CNT; k++) { 
			WRITE(".", 1);
			for (int i=0; i<=LENGTH; i++) // Write to main memory
				*(TEST_START+i) = i;

			for (int i=0; i<=LENGTH; i++){ // Read from main memory
				res = *(TEST_START+i);
				if (res != i){	// If data is not what we expect write error
					WRITE("e", 1);
					error++;
				}
			}
			if (error != 0){
				test++;
				WRITE("\n", 1);
			}
			error = 0;
		}
		WRITE("\n", 1);
		if (test != 0){
			WRITE("Errors\n", 7);
		} else {
			WRITE("Success\n", 8);
		}

	} else {
		//for (int k = 0; k < 100; ++k)
		//{
		//	for (int i=0; i<=LENGTH; i++){ // Read from main memory
		//		res = *(TEST_START+i);
		//		if (res == 0){	// If data is not what we expect write error
		//			error = error;
		//		} else {
		//			error++;
		//		}
		//	}
		//}
	}

	WRITE("Finished\n", 9);

	for (;;);
	// return 0;
}
