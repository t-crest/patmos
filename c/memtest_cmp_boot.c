/*
	Author: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
	Copyright: DTU, BSD License
*/
#include "cmpboot.h"
#include "patio.h"
#include <machine/spm.h>
#include <machine/patmos.h>

#define MEM_TEST 2000000

extern char _end;
#define MEM ((volatile _UNCACHED int *) &_end)

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

	if (CORE_ID == 0){
		for (int i=0; i<=MEM_TEST; i++){ // Read from main memory
			res = *(MEM+i);
			if (res != 0){	// If data is not what we expect write error
				error++;
			}
		}
		if (error != 0){
			WRITE("MEMORY uninitialized\n",21);
		}
		error = 0;

		for (int k = 0; k < 10; k++) { // Test 10 times
			WRITE("Test",4);
			for (int i=0; i<=MEM_TEST; i++) // Write to main memory
				*(MEM+i) = i;

			for (int i=0; i<=MEM_TEST; i++){ // Read from main memory
				res = *(MEM+i);
				if (res != i){	// If data is not what we expect write error
					WRITE("e",1);
					error++;
				}
			}
			if (error != 0){
				test++;
				WRITE("\n",1);
			}
			error = 0;
		}
		if (test != 0){
			WRITE("Errors\n",7);
		} else {
			WRITE("Success\n",8);
		}

	} else {
		//for (int k = 0; k < 100; ++k)
		//{
		//	for (int i=0; i<=MEM_TEST; i++){ // Read from main memory
		//		res = *(MEM+i);
		//		if (res == 0){	// If data is not what we expect write error
		//			error = error;
		//		} else {
		//			error++;
		//		}
		//	}
		//}
	}

	return 0;
}
