/*
	Test Seven Segment Display IO devices.

	Author: Lefteris Kyriakakis 
	Copyright: DTU, BSD License
*/
#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>

#define INTRO_DURATION 16
#define DISP_SYM_MASK 0x80

void printSegmentInt(uint base_addr, int number, int displayCount)__attribute__((noinline));

void resetDisp(uint disp_addr, int from, int to){
	volatile _IODEV uint *disp_ptr = (volatile _IODEV uint *) disp_addr;
	uint pos = 0;
	for(pos=from; pos < to; pos++){
		*disp_ptr = DISP_SYM_MASK | 0x7F;
		disp_ptr++;
	}
}

void inputDisp(uint disp_addr, int from, int to){
	volatile _IODEV uint *disp_ptr = (volatile _IODEV uint *) disp_addr;
	uint pos = 0;
	for(pos=from; pos < to; pos++){
		*disp_ptr = DISP_SYM_MASK | 0x77;
		disp_ptr++;
	}
}

void printSegmentInt(uint base_addr, int number, int displayCount) {
	volatile _IODEV uint *disp_ptr = (volatile _IODEV uint *) base_addr;
	uint pos = 0;
	uint byte_mask = 0x0000000F;
	uint range = (number > 0) ? displayCount : displayCount-1;	//reserve one digit for '-' symbol
	uint value = abs(number);
	for(pos=0; pos < range; pos++) {
		*disp_ptr = (uint)((value & byte_mask) >> (pos*4));
		//printf("value %d at disp_addr %p with byte_mask %x\n", *disp_ptr, disp_ptr, byte_mask);
		byte_mask = byte_mask << 4;
		disp_ptr += 1;
	}
	if (number < 0) {
		*disp_ptr = DISP_SYM_MASK | 0x3F;
	}
}

int main(int argc, char **argv)
{
	volatile _SPM int *uart_ptr = (volatile _SPM int *)	 0xF0080004;
	volatile _SPM int *led_ptr  = (volatile _SPM int *)  0xF0090000;
	volatile _IODEV uint *seg_disp0_ptr = (volatile _IODEV uint *) 0xF00B0000;
	volatile _IODEV uint *seg_disp1_ptr = (volatile _IODEV uint *) 0xF00B0004;
	volatile _IODEV uint *seg_disp2_ptr = (volatile _IODEV uint *) 0xF00B0008;
	volatile _IODEV uint *seg_disp3_ptr = (volatile _IODEV uint *) 0xF00B000C;
	volatile _IODEV uint *seg_disp4_ptr = (volatile _IODEV uint *) 0xF00B0010;
	volatile _IODEV uint *seg_disp5_ptr = (volatile _IODEV uint *) 0xF00B0014;
	volatile _IODEV uint *seg_disp6_ptr = (volatile _IODEV uint *) 0xF00B0018;
	volatile _IODEV uint *seg_disp7_ptr = (volatile _IODEV uint *) 0xF00B001C;

	int intro, i, j;

	puts("\n");
  	puts("Hello, Seven Segment!");

	//Test free drawing
	inputDisp(0xF00B0000, 0, 8);

	for (intro=INTRO_DURATION; intro!=0; --intro) {
		*uart_ptr = '1';
		for (i=1024; i!=0; --i)
			for (j=1024; j!=0; --j)
				*led_ptr = 1;
		

		*uart_ptr = '0';
		for (i=1024; i!=0; --i)
			for (j=1024; j!=0; --j)
				*led_ptr = 0;
	}

	//Test numbers
	*seg_disp0_ptr = 0x0;
	*seg_disp1_ptr = 0x1;
	*seg_disp2_ptr = 0x2;
	*seg_disp3_ptr = 0x3;
	*seg_disp4_ptr = 0x4;
	*seg_disp5_ptr = 0x5;
	*seg_disp6_ptr = 0x6;
	*seg_disp7_ptr = 0x7;

	//Demo user input
	int x = 0;
	printf("\nEnter a number to be display:\n");
	scanf("%d", &x);
	resetDisp(0xF00B0000, 0, 8);
	printSegmentInt(0xF00B0000, x, 8);

	return 0;
}