/*
    This is a minimal C program executed on the FPGA version of Patmos.
    An embedded Hello World program: a blinking LED.

    Additional to the blinking LED we write to the UART '0' and '1' (if available).

    Author: Martin Schoeberl
    Copyright: DTU, BSD License
*/

#include "include/bootable.h"
#include <machine/spm.h>

#define LOOP_DELAY 2000

int main() {

	volatile _SPM int *led_ptr  = (volatile _SPM int *) PATMOS_IO_LED;
	int i, j;

	for (;;) {
		UART_DATA = '1';
		for (i=LOOP_DELAY; i!=0; --i)
			for (j=LOOP_DELAY; j!=0; --j)
				*led_ptr = 1;


		UART_DATA = '0';
		for (i=LOOP_DELAY; i!=0; --i)
			for (j=LOOP_DELAY; j!=0; --j)
				*led_ptr = 0;

	}
}