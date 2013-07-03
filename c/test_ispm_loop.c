#include <machine/spm.h>

int main() 
{

	int (*start_program)() = (int (*)()) (0x800000);
	volatile _SPM int *ispm_ptr = (volatile _SPM int *) 0x800000;

	/*
	This is the code being loaded into the ISPM and executed below
	int main() {

		volatile _SPM int *led_ptr = (volatile _SPM int *) 0xF0000200;
		for(;;)
		{
			*led_ptr = 1;
		}
	}
	*/

	*(ispm_ptr+0) = 0x00020001;
	*(ispm_ptr+1) = 0x87c40000;
	*(ispm_ptr+2) = 0xf0000200;
	*(ispm_ptr+3) = 0x06400000;
	*(ispm_ptr+4) = 0x00400000;
	*(ispm_ptr+5) = 0x02c42080;
	start_program();
}
