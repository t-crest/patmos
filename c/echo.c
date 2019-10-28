/*
	Echo characters received from the UART to the UART.
	Toggles LEDs on every received character.

	TODO: IO is defined via ld/st local, but the compiler generates
	a different ld/st type here.

	Author: Martin Schoeberl
	Copyright: DTU, BSD License
*/

#include <machine/spm.h>
#include <stdio.h>

int main() {

	volatile _SPM int *led_ptr = (volatile _SPM int *) PATMOS_IO_LED;
	char val;

	for (;;) {
		val = getchar();
		putchar(val);
		*led_ptr = ~val;
	}
}
