int main() 
{

	int (*start_program)(void) = (int (*)(void)) 0x200000;
	volatile int *ispm_ptr = (int *) 0x200000;

	/*
	This is the code being loaded into the ISPM and executed below
	int main() {

		volatile int *led_ptr = (int *) 0xF0000200;
		for(;;)
		{
			*led_ptr = 1;
		}
	}
	*/

	int instructions[] = {0x00020001,0x87c40000,0xf0000200,0x06400000,0x00400000,0x02c42080};

	int i;
	for (i = 0; i < sizeof(instructions); ++i)
	{
		*(ispm_ptr+i) = instructions[i];
	}


	start_program();
}
