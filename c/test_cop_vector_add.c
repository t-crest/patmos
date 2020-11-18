#include <stdio.h>

int main() {
	int outputs[4] = {0, 0, 0, 0};
	int res1, res2, res3, res4 ;

	for(int i=0; i < 100; i+=8) {
		int inputs[8] = {i, i+1, i+2, i+3, i+4, i+5, i+6, i+7};
		
		asm (	"mov $r3 = %[a]\n\t"
			"mov $r4 = %[b]\n\t"
			".word 0x03443201\n\t"
			: 
			: [a] "r" (inputs), [b] "r" (outputs)
			: "3", "4");

		int flag = 0;
		while(!flag)
		{
			asm (	".word 0x034A0103\n\t"
				"mov %[a] = $r5\n\t"
				: [a] "=r" (flag)
				: 
				: "5");
		}

		asm (	//"nop\n\t"
			"lwm $r1 = [%[a]]\n\t"
			"lwm $r2 = [%[a] + 1]\n\t"
			"lwm $r3 = [%[a] + 2]\n\t"
			"lwm $r4 = [%[a] + 3]\n\t"
			"mov %[res1] = $r1\n\t"
			"mov %[res2] = $r2\n\t"
			"mov %[res3] = $r3\n\t"
			"mov %[res4] = $r4\n\t"
			: [res1] "=r" (res1), [res2] "=r" (res2), [res3] "=r" (res3), [res4] "=r" (res4)
			: [a] "r" (outputs)
			: "1", "2", "3", "4");

		printf("Vector Addition Vector 0 is: [%d, %d, %d, %d]\n", inputs[0], inputs[1], inputs[2], inputs[3]);
		printf("Vector Addition Vector 1 is: [%d, %d, %d, %d]\n", inputs[4], inputs[5], inputs[6], inputs[7]);
		printf("Vector Addition Result is:   [%d, %d, %d, %d]\n", res1, res2, res3, res4);
	}
}


