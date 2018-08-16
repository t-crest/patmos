/*
    Read the MPU interface values then do some blinking.
    Additional to the blinking LED we write to the UART '0' and '1' (if available).

    Author: Martin Schoeberl
    Copyright: DTU, BSD License
*/

#include <stdio.h>
#include <machine/spm.h>

int main() {

	volatile _SPM int *led_ptr  = (volatile _SPM int *) 0xF0090000;
	volatile _SPM int *uart_ptr = (volatile _SPM int *) 0xF0080004;
	volatile _SPM int *mpu_ptr = (volatile _SPM int *) 0xF00F0000;
	int i, j;

	// Read the MPU interface values
	for (i=0; i<10; ++i) {
		printf("Data %d: %d\n", i, mpu_ptr[i]);
	}

	// And now do some LED blinking
	for (;;) {
		*uart_ptr = '1';
		for (i=2000; i!=0; --i)
			for (j=2000; j!=0; --j)
				*led_ptr = 1;


		*uart_ptr = '0';
		for (i=2000; i!=0; --i)
			for (j=2000; j!=0; --j)
				*led_ptr = 0;

	}
}
