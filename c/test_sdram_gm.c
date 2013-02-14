/*
 Test SDRAM connected through Global Memory

 Outputs '?' to show it is ready, and waits for character input to start the test.
 T1: Store ASCII table into memory, and read it back
 T2: Do the full memory test

 Author: Edgar Lakis
 Copyright: DTU, BSD License
*/

/*********************** Test parameters ******************************/

//#define TEST_MEMORY_BYTES	(64*1024*1024)
#define TEST_MEMORY_BYTES	(64*1024)

#if 1
// ASCII table from ' '
#define TEST1_START ' '
#define TEST1_SIZE 93
#else
// alphabet from 'A'
#define TEST1_START 'A'
#define TEST1_SIZE 25
#endif



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

/* SDRAM Global Memory */
volatile int *sdram_base = (int *) 0x00000000;

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

		uart_out('\n');
		err_cnt = 0;

		uart_out('W');
		uart_out(':');
		for (i=0; i < TEST1_SIZE;i++) {
			sdram_base[i] = TEST1_START+i;
			uart_out(TEST1_START+i);
		}

		uart_out('\n');
		uart_out('R');
		uart_out(':');
		for (i=0; i < TEST1_SIZE;i++) {
			uart_out(sdram_base[i]);
			if (sdram_base[i] != TEST1_START+i) {
				//uart_out('!');
				err_cnt++;
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
