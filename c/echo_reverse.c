/*
	There shall be a comment, explaining what the program does.

	Author: Sahar
	Copyright: DTU, BSD License
*/

int main() {

	volatile int *read_val = (int *) 0x00000004;
	volatile int *uart_val_ptr = (int *) 0x00000004;	
	volatile int *led_ptr = (int *) 0xF0000200;
	int i, j;
	int flag = 0;
	int cmp1 = 2;
	int cmp2 = 1;
	int status;
	//int read_val;
	static int read_string[10];
	int num = 0;
	for (;;) {
		for(num = 9; num >= 0; num--)		
		{
			read_string[num] = num;		
		}
		for(num = 0; num <= 9; num++)		
		{
			*read_val = read_string[num];
		}
	}
}

