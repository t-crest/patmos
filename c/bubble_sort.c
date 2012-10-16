/*
	This is a program doing the bubble sort on an array, an improvment can be getting the numbers from UART
	Author: Martin Schoeberl
	Copyright: DTU, BSD License
*/

int main() {

	volatile int *uart_stat_ptr = (int *) 0xF0000000;
	volatile int *uart_val_ptr = (int *) 0xF0000004;
	int i, j, t;
	int prime = 1;
	int flag = 0;
	int str [10] = {1, 5, 6, 4, 2, 7, 3, 9, 8};
	int status;
	int cmp2 = 1;
	int n = 10;
	int temp;
	//for (;;) {
	 for(i=n-2;i>=0;i--)  
         {  
            for(j=0;j<=i;j++)  
             {  
                    if(str[j]>str[j+1])  
                    {  
                    	t=str[j];  
			temp = str[j+1];  
                        str[j]=temp;
                        str[j+1]=t;  
                    }  
             }  

           }
		
	for (j = 0; j <= 10; j++)// print to UART
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
		*uart_val_ptr = str[i];	
		flag = 0;

	}



}

