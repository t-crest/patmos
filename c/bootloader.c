#include <machine/patmos.h>
#include <machine/spm.h>

int main() __attribute__((naked,used));

#define _stack_cache_base 0x2f00
#define _shadow_stack_base 0x3f00

int main()
{
	   // setup stack frame and stack cache.
	    asm volatile ("mov $r29 = %0;" // initialize shadow stack pointer"
	                "mts $ss  = %1;" // initialize the stack cache's spill pointer"
	                "mts $st  = %1;" // initialize the stack cache's top pointer"
	                 : : "r" (_shadow_stack_base), "r" (_stack_cache_base));

	volatile _SPM int *ispm_ptr = (_SPM int *) 0x0;
	volatile _SPM int *uart_status_ptr = (_SPM int *) 0xF0000100;
	volatile _SPM int *uart_data = (_SPM int *) 0xF0000104;
	volatile _SPM int *led_ptr = (_SPM int *) 0xF0000200;

	int entrypoint = 0;
	int section_number = -1;
	int section_count = 0;
	int section_offset = 0;
	int section_size = 0;
	int integer = 0;
	int section_byte_count = 0;
	enum state {STATE_ENTRYPOINT, STATE_SECTION_NUMBER, STATE_SECTION_SIZE,
		STATE_SECTION_OFFSET, STATE_SECTION_DATA};

	enum state current_state = STATE_ENTRYPOINT;


	//Packet stuff
	int CRC_LENGTH = 4;
	int packet_byte_count = 0;
	int packet_size = 0;
	unsigned int calculated_crc = 0;
	unsigned int received_crc = 0xFFFFFFFF; //Flipped initial value
	unsigned int poly = 0xEDB88320; //Reversed polynomial

	for (;;)
	{
		int uart_status = *uart_status_ptr;
		*led_ptr = current_state;
		if(uart_status & 0x02)
		{
			int data = *uart_data;
			if(packet_size == 0)
			{
				//First received byte sets the packet size
				packet_size = data;
				packet_byte_count = 0;
				calculated_crc  = 0xFFFFFFFF;
				received_crc = 0;
			}
			else
			{
				if(packet_byte_count < packet_size)
				{
					calculated_crc = calculated_crc ^ data;
					int i;
					for (i = 0; i < 8; ++i)
					{
						if((calculated_crc & 1) > 0)
						{
							calculated_crc = (calculated_crc >> 1) ^ poly;
						}
						else
						{
							calculated_crc = (calculated_crc >> 1);
						}
					}

					integer |= data << ((3-(section_byte_count%4))*8);
					section_byte_count++;

					if(current_state < STATE_SECTION_DATA)
					{
						if(section_byte_count == 4)
						{
							if (current_state == STATE_ENTRYPOINT)
								entrypoint = integer;
							else if (current_state == STATE_SECTION_NUMBER)
								section_number = integer;
							else if (current_state == STATE_SECTION_SIZE)
								section_size = integer;
							else if (current_state == STATE_SECTION_OFFSET)
								section_offset = integer;

							section_byte_count = 0;
							current_state++;
						}
					}
					else
					{
						//In case of data less than 4 bytes write everytime
						*(ispm_ptr+(section_offset/4)+((section_byte_count-1)/4)) = integer;
						if(section_byte_count == section_size)
						{
							//current_state = STATE_SECTION_START;
							section_byte_count = 0;
							section_count++;
							current_state = STATE_SECTION_SIZE;
						}
					}
					if(section_byte_count%4 == 0)
					{
						integer = 0;
					}

				}
				else if(packet_byte_count < packet_size+CRC_LENGTH)
				{
					received_crc |= data << ((packet_size+CRC_LENGTH-packet_byte_count-1)*8);
				}
				packet_byte_count++;
				if(packet_byte_count == packet_size+CRC_LENGTH)
				{
					calculated_crc = calculated_crc ^ 0xFFFFFFFF; //Flipped final value
					if(calculated_crc == received_crc)
					{
						*uart_data = 'o';
						if(section_count == section_number)
						{
							//End of program transmission
							//Jump to program execution
							(*(volatile int (*)())entrypoint)();
						}
					}
					else
					{
						*uart_data = 'r';
					}
					packet_size = 0;
				}
			}
		}
	}
	return 0;
}
