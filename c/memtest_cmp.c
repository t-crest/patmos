/*
	Author: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
	Copyright: DTU, BSD License
*/
#include <stdio.h>
#include <machine/spm.h>
#include <machine/patmos.h>

#define MEM_TEST 2000000

extern char _end;
#define MEM ((volatile _UNCACHED int *) &_end)

int main() {
	int res;
	int error = 0;
	int test = 0;

	if (get_cpuid() == 0){
		for (int i=0; i<=MEM_TEST; i++){ // Read from main memory
			res = *(MEM+i);
			if (res != 0){	// If data is not what we expect write error
				error++;
			}
		}
		if (error != 0){
			puts("MEMORY uninitialized\n");
		}
		error = 0;

		for (int k = 0; k < 10; k++) { // Test 10 times
			for (int i=0; i<=MEM_TEST; i++) // Write to main memory
				*(MEM+i) = i;

			for (int i=0; i<=MEM_TEST; i++){ // Read from main memory
				res = *(MEM+i);
				if (res != i){	// If data is not what we expect write error
					puts("e");
					error++;
				}
			}
			if (error != 0){
				test++;
				puts("\n");
			}
			error = 0;
		}
		if (test != 0){
			puts("Errors\n");
		} else {
			puts("Success\n");
		}

	} else {
		for (int k = 0; k < 100; ++k)
		{
			for (int i=0; i<=MEM_TEST; i++){ // Read from main memory
				res = *(MEM+i);
				if (res == 0){	// If data is not what we expect write error
					error = error;
				} else {
					error++;
				}
			}
		}
	}

	return 0;
}
