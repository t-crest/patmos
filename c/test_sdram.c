/*
 Test SDRAM I/O device

 Outputs '?' to show it is ready, and waits for character input to start the test.
 T1.a: Test the connection to I/O device's buffer (write/read the string of 'A','B',...)
 T1.b: Test single block store/load (write/read the string of 'A','B',...)
 T2: Test multiple blocks. Store ASCII table into memory, and read it back

 Author: Edgar Lakis
 Copyright: DTU, BSD License
*/

/*********************** Test parameters ******************************/
// The SDRAM_IO device address size in Patmos (bits). This is the same for both
// boards, to avoid changing the address mapping. The larger size for ML605 board
// is used
#define SDRAM_IO_ADDR_WIDTH 4	// 2^4 addresses * 4 bytes == 64 bytes

//#define EINDHOVEN_CNTRL	// The test is for Eindhoven controller (undefine to use altera board)

// The size of line supported by controller (words)
#ifdef EINDHOVEN_CNTRL
#define N_BURST_WORDS (64/4)	// The Eindhoven controller uses 64 byte transfers
#else
#define N_BURST_WORDS (32/4)	// The simple controller uses 32 byte transfers
#endif

// The number of addressess used for one word
#ifdef EINDHOVEN_CNTRL
#define WORD_ADDR 4	// The Eindhoven controller is byte addressable
#else
#define WORD_ADDR 1	// The simple controller is word addressable
#endif

//#define TEST_MEMORY_BYTES	(64*1024*1024)
#define TEST_MEMORY_BYTES	(64*1024)

// One block
#define TEST1_SIZE N_BURST_WORDS
#define TEST1_START 'A'

// ASCII table from ' '
#define TEST2_START ' '
#define TEST2_SIZE 93





/**********************************************************************/
/***********************  IO parameters *******************************/

/* UART */
volatile int *uart_status_ptr = (int *) 0xF0000000;
volatile int *uart_data_ptr = (int *) 0xF0000004;


/* The outermost fake loop is to make a macro resolve to single statement
 * This way macro can be used in any context where statemetn is required
 */
#define uart_in(var) do { while(!(*uart_status_ptr & 2)); (var) = *uart_data_ptr; } while (0)
#define uart_out(val) do { while(!(*uart_status_ptr & 1)); *uart_data_ptr = (val); } while (0)

/* SDRAM IO */
volatile int *sdram_io = (int *) 0xF0000300;

#define SDRAM_ADDR_REG     (1 << SDRAM_IO_ADDR_WIDTH)
/* Same offset is used for both STATUS (during read) and COMMAND (during write) */
#define SDRAM_STATUS_REG  ((1 << SDRAM_IO_ADDR_WIDTH) +1)
#define SDRAM_COMMAND_REG ((1 << SDRAM_IO_ADDR_WIDTH) +1)

#define SDRAM_CMD_LOAD_LINE 0
#define SDRAM_CMD_STORE_LINE 1

#define SDRAM_READY 0
#define SDRAM_BUSY 1

#define sdram_wait_ready()     do { while(sdram_io[SDRAM_STATUS_REG] == SDRAM_BUSY);} while (0)
#define sdram_load_line(addr)  do { sdram_wait_ready(); sdram_io[SDRAM_ADDR_REG] = (addr*WORD_ADDR); sdram_io[SDRAM_COMMAND_REG] = SDRAM_CMD_LOAD_LINE; sdram_wait_ready(); } while (0)
#define sdram_store_line(addr)  do { sdram_wait_ready(); sdram_io[SDRAM_ADDR_REG] = (addr*WORD_ADDR); sdram_io[SDRAM_COMMAND_REG] = SDRAM_CMD_STORE_LINE; sdram_wait_ready(); } while (0)
/**********************************************************************/

int main() {
	unsigned  c;
	int  i;
	int err_cnt;

	for (;;) {
		uart_out('?');
		uart_in(c);
		err_cnt = 0;

//#define MSG(m) for (char *s=(m); *s; s++) uart_out(*s)
#define MSG(m) { char b[]=(m); for (char *s=b; *s; s++) uart_out(*s); }

		//MSG("\n T1.a:"); // One block from I/O device\n:");
		uart_out('\n');
		uart_out('T');
		uart_out('1');
		uart_out('a');
		uart_out(':');
		for (i=0; i < TEST1_SIZE; i++) {
			sdram_io[i] = TEST1_START+i;
		}
		for (i=0; i < TEST1_SIZE; i++) {
			uart_out(sdram_io[i]);
		}

		//MSG("\n T1.b: One block through memory\n:");
		uart_out('\n');
		uart_out('T');
		uart_out('1');
		uart_out('b');
		uart_out(':');
		// Write test pattern
		for (i=0; i < TEST1_SIZE; i++) {
			sdram_io[i] = TEST1_START+i;
		}
		sdram_store_line(0);
		// Overwrite with '0'
		for (i=0; i < TEST1_SIZE; i++) {
			sdram_io[i] = '0';
		}
		// Read test pattern
		sdram_load_line(0);
		for (i=0; i < TEST1_SIZE; i++) {
			uart_out(sdram_io[i]);
		}

		//MSG("\n T2: Multiple blocks through memory\n");
		uart_out('\n');
		uart_out('T');
		uart_out('2');
		uart_out('\n');
		err_cnt = 0;

		uart_out('W');
		uart_out(':');
		for (i=0; i < TEST2_SIZE;i += N_BURST_WORDS) {
			for (int j=0; j < N_BURST_WORDS; j++) {
				sdram_io[j] = TEST2_START+i+j;
				uart_out(TEST2_START+i+j);
			}
			sdram_store_line(i);
		}

		uart_out('\n');
		uart_out('R');
		uart_out(':');
		for (i=0; i < TEST2_SIZE;i += N_BURST_WORDS) {
			sdram_load_line(i);
			for (int j=0; j < N_BURST_WORDS; j++) {
				if (sdram_io[j] != TEST2_START+i+j) {
					err_cnt++;
				}
				uart_out(sdram_io[j]);
			}
		}

		uart_out('\n');
		if (err_cnt != 0) {
			for (i=30; i >= 0; i-=3) {
				uart_out('0' + ((err_cnt >> i)&7));
			}
			//MSG(" (octal) errors\n");
			uart_out('e');
			uart_out('r');
			uart_out('r');
			uart_out('\n');
		} else {
			uart_out('O');
			uart_out('K');
			uart_out('\n');
		}
	}
}
