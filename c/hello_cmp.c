/*
	Author: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
	Copyright: DTU, BSD License
*/
#include <machine/spm.h>
#include "boot.h"

#define MEM_TEST 2

int main() {
	int i, j, k;
	int error = 0;
	int test = 0;

	for (i=MEM_TEST; i!=0; --i){ // Read from main memory
		j = *(MEM+i);
		if (j != 0){	// If data is not what we expect write error
			error++;
		}
	}
	if (error != 0){
		WRITE("MEMORY uninitialized\n",21);
	}
	error = 0;

	for (k = 0; k < 10; k++) { // Test 10 times
		for (i = MEM_TEST; i != 0; --i) // Write to main memory
			*(MEM+i) = i;

		for (i=MEM_TEST; i!=0; --i){ // Read from main memory
			j = *(MEM+i);
			if (j != i){	// If data is not what we expect write error
				UART_DATA = 'e';
				error++;
			}
		}
		if (error != 0){
			test++;
			UART_DATA = '\n';
		}
		error = 0;
	}
	if (test != 0){
		WRITE("Errors\n",7);
	} else {
		WRITE("Success\n",8);
	}

	for (;;) { // Blink LEDS forever
		for (i=2000; i!=0; --i)
			for (j=2000; j!=0; --j)
				LEDS = 1;

		for (i=2000; i!=0; --i)
			for (j=2000; j!=0; --j)
				LEDS = 0;
	}
}
