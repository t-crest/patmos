/*
	This is the first C based program executed on the FPGA version
	of Patmos. A carefully written embedded hello world program.
	Additional to the blinking LED we write to the UART '0' and '1' (if available).

	TODO: IO is defined via ld/st local, but the compiler generates
	a different ld/st type here. We currently accept all ld/st types
	for IO in the HW (not in pasim).

	Author: Martin Schoeberl
	Copyright: DTU, BSD License
*/

#include <machine/spm.h>

int main() {

volatile int *dummy = (int *) 0x123;

	volatile _SPM int *led_ptr = (volatile _SPM int *) 0xF0000200;
	volatile _SPM int *uart_ptr = (volatile _SPM int *) 0xF0000104;
	int i, j;

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
