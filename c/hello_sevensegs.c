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

void printSegmentInt(unsigned base_addr, int number, int displayCount)__attribute__((noinline));

void resetDisp(unsigned disp_addr, int from, int to){
	volatile _IODEV unsigned *disp_ptr = (volatile _IODEV unsigned *) disp_addr;
	unsigned pos = 0;
	for(pos=from; pos < to; pos++){
		*disp_ptr = DISP_SYM_MASK | 0x7F;
		disp_ptr++;
	}
}

void inputDisp(unsigned disp_addr, int from, int to){
	volatile _IODEV unsigned *disp_ptr = (volatile _IODEV unsigned *) disp_addr;
	unsigned pos = 0;
	for(pos=from; pos < to; pos++){
		*disp_ptr = DISP_SYM_MASK | 0x77;
		disp_ptr++;
	}
}

void printSegmentInt(unsigned base_addr, int number, int displayCount) {
	volatile _IODEV unsigned *disp_ptr = (volatile _IODEV unsigned *) base_addr;
	unsigned pos = 0;
	unsigned byte_mask = 0x0000000F;
	unsigned range = (number > 0) ? displayCount : displayCount-1;	//reserve one digit for '-' symbol
	unsigned value = abs(number);
	for(pos=0; pos < range; pos++) {
		*disp_ptr = (unsigned)((value & byte_mask) >> (pos*4));
		byte_mask = byte_mask << 4;
		disp_ptr += 1;
	}
	if (number < 0) {
		*disp_ptr = DISP_SYM_MASK | 0x3F;
	}
}

int main(int argc, char **argv)
{
	volatile _IODEV int *uart_ptr = (volatile _IODEV int *)	PATMOS_IO_UART;
	volatile _IODEV int *led_ptr  = (volatile _IODEV int *) PATMOS_IO_LED;
	volatile _IODEV int *disp_ptr = (volatile _IODEV int *)	PATMOS_IO_SEGDISP;

	int intro, i, j;

	puts("\n");
  	puts("Hello, Seven Segment!");

	//Test free drawing
	inputDisp(PATMOS_IO_SEGDISP, 0, 8);

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
	for(int i=0x0; i<8; i++){
		disp_ptr[i] = (unsigned int) i;
	}

	//Demo user input
	int x = 0;
	printf("\nEnter a number to be display:\n");
	scanf("%d", &x);
	resetDisp(PATMOS_IO_SEGDISP, 0, 8);
	printSegmentInt(PATMOS_IO_SEGDISP, x, 8);

	return 0;
}