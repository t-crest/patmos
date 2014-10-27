/*
    This is a minimal C program executed on the FPGA version of Patmos.
    An embedded Hello World program: a blinking LED.

    Additional to the blinking LED we write to the UART '0' and '1' (if available).

    Author: Martin Schoeberl
    Copyright: DTU, BSD License
*/

#include <machine/spm.h>
#include <stdio.h>

int main() {

	volatile _SPM int *led_ptr = (volatile _SPM int *) 0xF0000900;
	int i, j;

	for (;;) {
		putchar('1');
		fflush(stdout);
		for (i=2000; i!=0; --i)
			for (j=2000; j!=0; --j)
				*led_ptr = 1;

		putchar('0');
		fflush(stdout);
		for (i=2000; i!=0; --i)
			for (j=2000; j!=0; --j)
				*led_ptr = 0;

	}
}
