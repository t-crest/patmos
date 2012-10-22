/*
	This program prints 10 first fibonacci numbers in hex format, for converting to characters / is needed which will generate a big ROM.
	Author: Sahar
	Copyright: DTU, BSD License
*/

int main() {

	volatile int *uart_stat_ptr = (int *) 0xF0000000;
	volatile char *uart_val_ptr = (int *) 0xF0000004;
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

