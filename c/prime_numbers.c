/*
	This is a program printing prime numbers less than 100, for % stdlib is needed which will generate a big ROM not runnable.
	Author: Sahar
	Copyright: DTU, BSD License
*/

#include <machine/spm.h>

int main() {

	volatile _SPM int *uart_stat_ptr = (volatile _SPM int *) 0xF0000800;
	volatile _SPM int *uart_val_ptr = (volatile _SPM int *) 0xF0000804;
	int i, j;
	int prime = 1;
	int flag = 0;
	volatile int str [10];
	int status;
	int cmp2 = 1;

	//for (;;) {
	for (i = 0; i <= 10; i += 2)
	{
		for (j = i - 1; j >= 0; j--)
			if (i%j == 0)
				{ 	
					prime = 0;
					break;
				}
		if (prime == 1) // print to UART
		{	
			while (!flag )
			{
				status = *uart_stat_ptr & cmp2;
				if (status == 1)
					{
						flag = 1; 
						break;
					}		
			}
			*uart_val_ptr = i;	
			flag = 0;
			prime = 1;
		}
	}
		

}

