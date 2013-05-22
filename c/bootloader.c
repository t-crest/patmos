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

	int frame_data_size = 0;
	int frame_position = 0;
	unsigned char uart_byte;

	const int STATE_SECTION_AMOUNT = 0;
	const int STATE_SECTION_SIZE = 1;
	const int STATE_SECTION_OFFSET = 2;
	const int STATE_SECTION_DATA = 3;
	const int STATE_START = 4;
	int state = STATE_SECTION_AMOUNT;

	int section_data_count = 0;
	int section_size = 0;
	int section_offset = 0;
	int section_amount = 0;
	int section_count = 0;
	int instruction = 0;

	int integer_byte_count = 0;

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
				if(state < STATE_SECTION_DATA)
				{
					if(state == STATE_SECTION_AMOUNT)
					{
						section_amount |= uart_byte <<  ((4-1-integer_byte_count)*8);
					}
					else if(state == STATE_SECTION_SIZE)
					{
						section_size |= uart_byte <<  ((4-1-integer_byte_count)*8);
					}
					else if(state == STATE_SECTION_OFFSET)
					{
						section_offset |= uart_byte <<  ((4-1-integer_byte_count)*8);
					}
					integer_byte_count++;
					if(integer_byte_count == 4)
					{
						state++;
						integer_byte_count = 0;
					}
				}
				else if(state == STATE_SECTION_DATA)
				{
					instruction |= uart_byte << ((4-1-integer_byte_count)*8);
					integer_byte_count++;
					if(integer_byte_count == 4)
					{
						integer_byte_count = 0;
						*(ispm_ptr-1+(section_offset+section_data_count)/4) = instruction;
						section_data_count += 4;
					}

					if(section_data_count == section_size)
					{
						state = STATE_SECTION_SIZE;
						section_data_count = 0;
						section_size = 0;
						section_offset = 0;
						section_count++;
						if(section_count == section_amount)
						{
							state = STATE_START;
						}
					}
				}
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
					if(state == STATE_START)
					{
						*led_ptr = 0xAB;

						//End of program transmission
						//Jump to program execution
						start_program();
					}
					//uart_printc('o');
					//uart_flush();

				}
				else
				{
					*uart_data = 'r';
					//uart_printc('r');
					//uart_flush();
				}
				frame_data_size = 0;
				frame_position = 0;
				crc  = 0xFFFFFFFF;
				hostcrc = 0;
			}
		}
	}
	return 0;
}
