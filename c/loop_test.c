/*
	This is the first C based program executed on the FPGA version
	of Patmos. A carefully written embedded hello world program.

	Author: Martin Schoeberl
	Copyright: DTU, BSD License
*/

int main() {

	volatile int *led_ptr = (int *) 0xF0000200;
	int i, j, k;
	volatile int str [10];

	//for (;;) {
		k = 25;
		for (i=20; i>=0; i--)
			str[i] = i + 1;
		while (k > 0)
			k--;
		for (i=20; i>=10; i--)
			j = str[i];
	//	k = j + 1;
		/*for (i=2000; i!=0; --i)
			for (j=2000; j!=0; --j)
				*led_ptr = 0;*/
	//}
}
