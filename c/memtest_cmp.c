/*
	Author: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
	Copyright: DTU, BSD License
*/
#include <machine/spm.h>
#include "patio.h"

#define MEM_OFFSET 0x300000
#define MEM_TEST 2000000

int main() {
	int i, j, k;
	int error = 0;
	int test = 0;

	if (CORE_ID <=3){
		for (i=MEM_OFFSET; i<=MEM_OFFSET+MEM_TEST; i++){ // Read from main memory
			j = *(MEM+i);
			if (j == 0){	// If data is not what we expect write error
				error = error;
			} else {
				error++;
			}
		}
		if (error != 0){
			WRITE("MEMORY uninitialized\n",21);
		}
		error = 0;

		for (k = 0; k < 10; k++) { // Test 10 times
			for (i=MEM_OFFSET; i<=MEM_OFFSET+MEM_TEST; i++) // Write to main memory
				*(MEM+i) = i;

			for (i=MEM_OFFSET; i<=MEM_OFFSET+MEM_TEST; i++){ // Read from main memory
				j = *(MEM+i);
				if (j != i){	// If data is not what we expect write error
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
		for(;;);
	}


}
