/*#include "../../newlib/newlib/libc/machine/patmos/machine/uart.h"

//Dragons' start
int main() __attribute__((naked,used));

#define _stack_cache_base 0x1000
#define _shadow_stack_base 0x2000
*/
int main()
{
  /*	// setup stack frame and stack cache.
  	asm volatile ("mov $r29 = %0;" // initialize shadow stack pointer"
                "mts $ss  = %1;" // initialize the stack cache's spill pointer"
                "mts $st  = %1;" // initialize the stack cache's top pointer"
                 : : "r" (_shadow_stack_base), "r" (_stack_cache_base));
//Dragons' end


	int (*volatile start_program)(void) = (int (*)(void)) 0x200000;
	*/
	int (*start_program)(void) = (int (*)(void)) 0x200000;
	volatile int *ispm_ptr = (int *) 0x200000;

	const unsigned char MAGIC_NUMBER = 0xAB;
	const int CLOCK_RATE = 100000000;
	const int CRC_LENGTH = 4;

	unsigned int crc  = 0xFFFFFFFF; //Flipped initial value
	unsigned int poly = 0xEDB88320; //Reversed polynomial
	unsigned int hostcrc = 0;

	volatile int *led_ptr = (int *) 0xF0000200;
	volatile int *uart_data = (int *) 0xF0000104;
	volatile int *uart_status_ptr = (int *) 0xF0000100;

	int uart_status;
	int reset = 0;
	int program_size = 0;
	unsigned char frame_byte = 0;
	unsigned int frame_data_size = 0;
	unsigned int frame_position = 0;

	unsigned char uart_byte;
	//char uart_byte_temp;
	//char *uart_byte_temp_ptr = &uart_byte_temp;

	for (;;)
	{
		uart_status = *uart_status_ptr;
		//if (uart_read(uart_byte_temp_ptr,1) > 0)
		if(uart_status& 0x02)
		{
			//uart_byte = (unsigned char)uart_byte_temp;
			uart_byte = (unsigned char)*uart_data;
			if(frame_data_size == 0)
			{
				//First received byte sets the frame size
				frame_data_size = uart_byte;
				//*led_ptr = frame_data_size;
			}
			else if(frame_position < frame_data_size)
			{
				crc = crc ^ uart_byte;
				int i;
				for (i = 0; i < 8; ++i)
				{
					if((crc & 1) > 0)
					{
						crc = (crc >> 1) ^ poly;
					}
					else
					{
						crc = (crc >> 1);
					}
				}
				frame_byte = uart_byte;
				*(ispm_ptr+program_size+frame_position) = frame_byte;
				frame_position++;
			}
			else if(frame_position < frame_data_size+CRC_LENGTH)
			{
				int shift = frame_data_size+CRC_LENGTH-frame_position-1;
				hostcrc |= uart_byte <<  (shift*8);
				frame_position++;
			}
			if(frame_position == frame_data_size+CRC_LENGTH)
			{
				crc = crc ^ 0xFFFFFFFF; //Flipped final value
				if(crc == hostcrc)
				{
					*uart_data = 'o';
					//uart_printc('o');
					//uart_flush();
					program_size += frame_position;
					if(frame_byte == MAGIC_NUMBER)
					{
						*led_ptr = 0xAB;
						//End of program transmission
						//Jump to program execution
						start_program();

					}
				}
				else
				{
					*uart_data = 'r';
					//uart_printc('r');
					//uart_flush();
				}
				reset = 1;
			}

		}
		if(reset)
		{
			reset = 0;
			frame_data_size = 0;
			frame_position = 0;
			crc  = 0xFFFFFFFF;
			hostcrc = 0;
		}
	}

	return 0;
}
