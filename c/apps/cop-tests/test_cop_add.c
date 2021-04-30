//#include "include/bootable.h"

int main() {

	unsigned int opa = 1;
	unsigned int opb = 2;
	unsigned int opc = (1 << 31) - 3;
	unsigned int opd = 1 << 31;
	unsigned int res = 0;

	
	// adds all operands together
	// add r1 r2 -> r3
	// add r3 r4 -> r5
	// add r5 r6 -> r7
	asm (	"mov $r1 = %[a]\n\t"
		"mov $r2 = %[b]\n\t"
		"mov $r4 = %[c]\n\t"
		"mov $r6 = %[d]\n\t"
		//add without delay
		".word 0x03461100\n\t"
		".word 0x034A3200\n\t"
		".word 0x034E5300\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		//reset registers
		"mov $r1 = %[a]\n\t"
		"mov $r2 = %[b]\n\t"
		"mov $r4 = %[c]\n\t"
		"mov $r6 = %[d]\n\t"
		//add with delay
		".word 0x03461102\n\t"
		".word 0x034A3202\n\t"
		".word 0x034E5302\n\t"
		"mov %[res] = $r7\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		: [res] "=r" (res)
		: [a] "r" (opa), [b] "r" (opb), [c] "r" (opc), [d] "r" (opd)
		: "1", "2", "3", "4", "5", "6", "7" );
}


