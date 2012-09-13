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
	int status = 0;
	int  read_val;
	//while (status == 0)
	//{
		
	//}	
	for (;;) {
/*		for (i=2000; i!=0; --i)
			for (j=2000; j!=0; --j)
				*led_ptr = 1;

		for (i=2000; i!=0; --i)
			for (j=2000; j!=0; --j)
				*led_ptr = 0;*/
		
		while (!flag )
		{

			status = *uart_stat_ptr & cmp1;
			if (status == cmp1)
				flag = 1;
				
		}
		
	for (i=2000; i!=0; --i)
                        for (j=2000; j!=0; --j)
                                *led_ptr = 1;

                for (i=2000; i!=0; --i)
                        for (j=2000; j!=0; --j)
                                *led_ptr = 0;
		read_val =  *uart_val_ptr;
		flag = 0;
		while (flag == 0)
                {

         		status = *uart_stat_ptr & cmp2;
                        if(status == cmp2)
                                flag = 1;
                }

		*uart_val_ptr = read_val; 
	}
}

