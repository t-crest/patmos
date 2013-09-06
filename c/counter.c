/*
	This program receives a char from UART and counts the number in a default string, and prints the number to UART.
	Author: Sahar
	Copyright: DTU, BSD License
*/

#include <machine/spm.h>

int main() {

	volatile _SPM int *uart_stat_ptr = (volatile _SPM int *) 0xF0000800;
	volatile _SPM int *uart_val_ptr = (volatile _SPM int *) 0xF0000804;
	int i;
	int counter = 0;
	int flag = 0;
	int cmp1 = 2;
	int cmp2 = 1;
	int status;
	char  read_val;
	char str[15] = {'a', 'v', 'c', 'v', 'g', '7', 'v', 'a', 'p', 'e', 'm', 'u', 'y', 'v', 'o'};
	while (!flag )
		{
			status = *uart_stat_ptr & cmp1;
			if (status == 2)
				{flag = 1; break;}
				
		}
	read_val =  *uart_val_ptr;
	flag = 0;
	for (i = 0; i < 15; i++)
		if (read_val == str[i])
			counter++;
	while (!flag )
		{
			status = *uart_stat_ptr & cmp2;
			if (status == 1)
				{flag = 1; break;}
				
		}
		*uart_val_ptr = counter + '0';
		flag = 0;
		

}

