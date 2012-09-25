/*
	There shall be a comment, explaining what the program does.

	Author: Sahar
	Copyright: DTU, BSD License
*/

int main() {

	volatile int *uart_stat_ptr = (int *) 0xF0000000;
	volatile int *uart_val_ptr = (int *) 0xF0000004;	
	volatile int *led_ptr = (int *) 0xF0000200;
	int i, j;
	int flag = 0;
	int cmp1 = 2;
	int cmp2 = 1;
	int status;
	char read_val;
	char read_string[10];
	int num = 0;
	for (;;) {
		//for (i=2000; i!=0; --i)
		for(num = 9; num != 0; num--)		
		{	
			while (!flag )
			{
				status = *uart_stat_ptr & cmp1;
				if (status == 2)
					{
						flag = 1; 
						//break;
					}
				
			}
			//read_val =  *uart_val_ptr;
			read_string[num] = *uart_val_ptr;
			//*uart_val_ptr = read_string[num];
			flag = 0;
		}
		flag = 0;
		//*uart_val_ptr = read_string[num];
		for(num = 0; num != 10; num++)		
		{
			while (!flag )
			{
				status = *uart_stat_ptr & cmp2;
				if (status == 1)
					{flag = 1;}
				
			}
//			*uart_val_ptr = num;
			*uart_val_ptr = '*';//read_string[num];//read_val;
			flag = 0;
		}
	}
}

