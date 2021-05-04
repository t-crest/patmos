#include <stdio.h>

int main() {

	int res = 0;
	for(int i=1; i<=45; i++)
	{
		asm (
			"mov $r3 = %[a]\n\t"
			".word 0x03443011\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"

			".word 0x03480113\n\t"
			"mov %[d] = $r4\n\t"

			"nop\n\t"

			".word 0x03443011\n\t"
			"nop\n\t"
			".word 0x03480113\n\t"
			"mov %[d] = $r4\n\t"

			: [d] "=r" (res)
			: [a] "r" (i)
			: "3", "4", "5" );
		printf("Fibonacci sequence after %d iterations: %d\n", i, res);
	}
}


