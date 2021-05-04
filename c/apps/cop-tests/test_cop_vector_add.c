#include <stdio.h>
#include <machine/patmos.h>

volatile _UNCACHED int outputs[4] __attribute__((aligned(16))) = {0, 0, 0, 0};

int main() {
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

		printf("Vector Addition Vector 0 is: [%d, %d, %d, %d]\n", inputs[0], inputs[1], inputs[2], inputs[3]);
		printf("Vector Addition Vector 1 is: [%d, %d, %d, %d]\n", inputs[4], inputs[5], inputs[6], inputs[7]);
		printf("Vector Addition Result is:   [%d, %d, %d, %d]\n", outputs[0], outputs[1], outputs[2], outputs[3]);
	}
}


