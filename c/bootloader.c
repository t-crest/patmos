#include <machine/patmos.h>
#include <machine/spm.h>

int main() __attribute__((naked,used));

extern int _stack_cache_base, _shadow_stack_base;

#define MEM         ((volatile int *) 0x0)
#define SPM         ((volatile _SPM int *) 0x0)

#define UART_STATUS *((volatile _SPM int *) 0xF0000800)
#define UART_DATA   *((volatile _SPM int *) 0xF0000804)
#define LEDS        *((volatile _SPM int *) 0xF0000900)

#define XDIGIT(c) ((c) <= 9 ? '0' + (c) : 'a' + (c) - 10)

#define WRITE(data,len) do { \
  unsigned i; \
  for (i = 0; i < (len); i++) {		   \
    while ((UART_STATUS & 0x01) == 0); \
    UART_DATA = (data)[i];			   \
  } \
} while(0)

int main()
{
    // setup stack frame and stack cache.
    asm volatile ("mov $r29 = %0;" // initialize shadow stack pointer"
                  "mts $ss  = %1;" // initialize the stack cache's spill pointer"
                  "mts $st  = %1;" // initialize the stack cache's top pointer"
                  "li $r30 = %2;" // initialize return base"
                  : : "r" (_shadow_stack_base-16),
                      "r" (_stack_cache_base-16),
                      "i" (&main));

	int entrypoint = 0;
	int section_number = -1;
	int section_count = 0;
	int section_filesize = 0;
	int section_offset = 0;
	int section_memsize = 0;
	int integer = 0;
	int section_byte_count = 0;
	enum state { STATE_ENTRYPOINT, STATE_SECTION_NUMBER,
                 STATE_SECTION_FILESIZE, STATE_SECTION_OFFSET, STATE_SECTION_MEMSIZE,
                 STATE_SECTION_DATA };

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
		LEDS = current_state;
		if(UART_STATUS & 0x02)
		{

			int data = UART_DATA;
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
				if(packet_byte_count < CRC_LENGTH)
				{
					received_crc |= data << ((CRC_LENGTH-packet_byte_count-1)*8);
				}
				else if(packet_byte_count < packet_size+CRC_LENGTH)
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
						  switch(current_state)
							{
							case STATE_ENTRYPOINT:
							  entrypoint = integer;
							  break;
							case STATE_SECTION_NUMBER:
							  section_number = integer;
							  break;
							case STATE_SECTION_FILESIZE:
							  section_filesize = integer;
							  break;
							case STATE_SECTION_OFFSET:
							  section_offset = integer;
							  break;
							case STATE_SECTION_MEMSIZE:
							  section_memsize = integer;
							  break;
							default:
							  /* never happens */;
							}

							section_byte_count = 0;
							current_state++;
						}
					}
					else
					{
						//In case of data less than 4 bytes write everytime
						//Write to ISPM
						if ((section_offset+section_byte_count-1) >> 16 == 0x01) {
						  *(SPM+(section_offset+section_byte_count-1)/4) = integer;
						}
						//Write to main memory
						*(MEM+(section_offset+section_byte_count-1)/4) = integer;

						if(section_byte_count == section_filesize)
						{
						    // Align to next word boundary
						    section_byte_count = (section_byte_count + 3) & ~3;
						    // Fill up uninitialized areas with zeros
						    while (section_byte_count < section_memsize)
						    {
						        if ((section_offset+section_byte_count) >> 16 == 0x01) {
						          *(SPM+(section_offset+section_byte_count)/4) = 0;
						        }
						        *(MEM+(section_offset+section_byte_count)/4) = 0;
						        section_byte_count += 4;
							}
							// Values for next segment
							section_byte_count = 0;
							section_count++;
							current_state = STATE_SECTION_FILESIZE;
						}
					}
					if(section_byte_count%4 == 0)
					{
						integer = 0;
					}
				}

				packet_byte_count++;
				if(packet_byte_count == packet_size+CRC_LENGTH)
				{
					calculated_crc = calculated_crc ^ 0xFFFFFFFF; //Flipped final value
					if(calculated_crc == received_crc)
					{
						UART_DATA = 'o';
						if(section_count == section_number)
						{
							//End of program transmission
							//Jump to program execution
							int retval = (*(volatile int (*)())entrypoint)();

							// Compensate off-by-one of return offset with NOP
							// (internal base address is 0 after booting).
							// Return may be "unclean" and leave registers clobbered.
							asm volatile ("nop" : :
										  : "$r2", "$r3", "$r4", "$r5",
											"$r6", "$r7", "$r8", "$r9",
											"$r10", "$r11", "$r12", "$r13",
											"$r14", "$r15", "$r16", "$r17",
											"$r18", "$r19", "$r20", "$r21",
											"$r22", "$r23", "$r24", "$r25",
											"$r26", "$r27", "$r28", "$r29");

							// Print exit magic and return code
							{
							  char msg[10];
							  msg[0] = '\0';
							  msg[1] = 'x';
							  msg[2] = retval & 0xff;
							  WRITE(msg, 3);
							}
							// Start again
							// TODO: replace with a real reset
							main();
						}
					}
					else
					{
						UART_DATA = 'r';
					}
					packet_size = 0;
				}
			}
		}
	}
	return 0;
}
