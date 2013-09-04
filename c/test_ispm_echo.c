#include <machine/spm.h>

int main() 
{

	int (*start_program)() = (int (*)()) (0x10000); //Call immediate uses words and not bytes
	volatile _SPM int *ispm_ptr = (volatile _SPM int *) 0x10000; //Writing uses bytes

	//Writes echo.c to ISPM and jumps to it
	*(ispm_ptr+0) = 0x24c0030;
	*(ispm_ptr+1) = 0x40001;
	*(ispm_ptr+2) = 0x87c20000;
	*(ispm_ptr+3) = 0xf0000900;
	*(ispm_ptr+4) = 0x2c21100;
	*(ispm_ptr+5) = 0x87c40000;
	*(ispm_ptr+6) = 0xf0000800;
	*(ispm_ptr+7) = 0x6400008;
	*(ispm_ptr+8) = 0x87c60000;
	*(ispm_ptr+9) = 0xf0000804;
	*(ispm_ptr+10) = 0x80000;
	*(ispm_ptr+11) = 0x28a3080;
	*(ispm_ptr+12) = 0x208400b;
	*(ispm_ptr+13) = 0x2c23280;
	*(ispm_ptr+14) = 0x2c21200;
	*(ispm_ptr+15) = 0x28a2080;
	*(ispm_ptr+16) = 0x400000;
	*(ispm_ptr+17) = 0x1ca5002;
	*(ispm_ptr+18) = 0x2025030;
	*(ispm_ptr+19) = 0xe7ffffc;
	*(ispm_ptr+20) = 0x400000;
	*(ispm_ptr+21) = 0x400000;
	*(ispm_ptr+22) = 0x67ffff5;
	*(ispm_ptr+23) = 0x400000;
	*(ispm_ptr+24) = 0x400000;

	start_program();
}
