/*
	A very small program to test if main memory is working.

	Author: Wolfgang Puffitsch
	Copyright: DTU, BSD License
*/

#include <machine/patmos.h>
#include <machine/spm.h>

int main() __attribute__((naked,used));

#define _stack_cache_base 0x2f00
#define _shadow_stack_base 0x3f00

#define INTMEM  ((volatile int *) 0x0)
#define CHARMEM ((volatile char *) 0x0)

int main()
{
  volatile _SPM int *led_ptr = (volatile _SPM int *) 0xF0000900;
  volatile _SPM int *uart_ptr = (volatile _SPM int *) 0xF0000804;

  unsigned i, k;

  INTMEM[0x10] = 0x12345678;
  INTMEM[0x11] = 0x9abcdef0;

  for (i = 0; i < 8; i++) {
	*uart_ptr = CHARMEM[0x10*sizeof(int)+i];
	*led_ptr = CHARMEM[0x10*sizeof(int)+i];
	for (k = 0; k < 1000000; k++) {
	  asm volatile("");
	}
  }

  *uart_ptr = '\n';

  //  for(;;);
}
