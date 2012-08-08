/*
	This is the first C based program executed on the FPGA version
	of Patmos. A carefully written embedded hello world program.

	Author: Martin Schoeberl
	Copyright: DTU, BSD License
*/

int main() {

	volatile int *led_ptr = (int *) 0x10;
	int i, j;

	for (;;) {
		for (i=2000; i!=0; --i)
			for (j=2000; j!=0; --j)
				*led_ptr = 1;

		for (i=2000; i!=0; --i)
			for (j=2000; j!=0; --j)
				*led_ptr = 0;
	}
}
