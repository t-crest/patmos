/*
	Test function call and stack manipulation.

	Author: Martin Schoeberl
	Copyright: DTU, BSD License
*/

int foo(int a, int b) {

	int c = a+b;
	c++;
	return c;
}

int main() {

	volatile int *read_val = (int *) 0x00000004;
	volatile int *uart_val_ptr = (int *) 0x00000004;	
	volatile int *led_ptr = (int *) 0xF0000200;

	volatile int i, j, k;
	volatile int arr[30];

	i = *uart_val_ptr;
	j = *uart_val_ptr;
	k = foo(i, j);
	arr[20] = foo(arr[0], arr[1]);
	*led_ptr = arr[20]+k;
}

