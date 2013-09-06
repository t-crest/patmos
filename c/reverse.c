/*
	This code receives several characters until it encounters the new line where it echos the received characters in reverse!

	Author: Sahar
	Copyright: DTU, BSD License
*/

#include <machine/spm.h>

int main() {

	volatile _SPM int *uart_stat_ptr = (volatile _SPM int *) 0xF0000800;
	volatile _SPM int *uart_val_ptr = (volatile _SPM int *) 0xF0000804;
	volatile _SPM int *led_ptr = (volatile _SPM int *) 0xF0000900;
	int i, j, k;
	int flag = 0;
	int flag2 = 0;
	int cmp1 = 2;
	int cmp2 = 1;
	int status;
	char read_val;
	static char read_string[1000];
	int num = 0;
	for(;;) {
	  k = 0;

	  for(;;) {
		while (!(*uart_stat_ptr & 2)) {
		  /* wait */
		}
		read_val = *uart_val_ptr;
		*uart_val_ptr = read_val;
		if ((int)read_val == '\n') {
		  break;
		}	  
		read_string[k] = read_val; 
		k++;		
	  }
	
	  for (num = k-1 ; num >= 0; num--) {
		while (!(*uart_stat_ptr & 1)) {
		  /* wait */
		}
		*uart_val_ptr =  read_string[num];
	  }
	}
}

