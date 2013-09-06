/*
	This program prints 10 first fibonacci numbers in hex format, for converting to characters / is needed which will generate a big ROM.
	Author: Sahar
	Copyright: DTU, BSD License
*/

#include <machine/spm.h>

int main() {

	volatile _SPM int *uart_stat_ptr = (volatile _SPM int *) 0xF0000800;
	volatile _SPM int *uart_val_ptr = (volatile _SPM int *) 0xF0000804;
	int i, j;
	int a = 0;
	int b = 1;
	int fib = 0;
	int flag = 0;
	int status;
	int cmp2 = 1;

	for (i = 0; i < 10; i++)
	{
		fib = a + b;
		a = b;
		b = fib;
		while (!flag )
		{
			status = *uart_stat_ptr & cmp2;
			if (status == 1)
				{
					flag = 1; 
					break;
				}		
		}
		*uart_val_ptr = fib + '0';	
		flag = 0;
	}

		

}

