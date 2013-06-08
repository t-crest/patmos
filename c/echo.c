/*
	Echo characters received from the UART to the UART.
	Toggles LEDs on every received character.

	TODO: IO is defined via ld/st local, but the compiler generates
	a different ld/st type here.

	Author: Martin Schoeberl
	Copyright: DTU, BSD License
*/

int main() {

	volatile int *led_ptr = (int *) 0xF0000200;
	volatile int *uart_status = (int *) 0xF0000100;
	volatile int *uart_data = (int *) 0xF0000104;
	int tog = 0;
	int status;
	int val;

	*led_ptr = 1;
	for (;;) {
		status = *uart_status;
		if (status & 0x02) {
			val = *uart_data;
			*uart_data = val;
			tog = ~tog;
			*led_ptr = tog;
		}
	}
}
