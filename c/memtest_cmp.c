/*
	Author: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
	Copyright: DTU, BSD License
*/
#include <stdio.h>
#include <machine/spm.h>
#include <machine/patmos.h>

// we assume 2 MB memory, less than 400 KB for program,
// heap, and stack
#define LENGTH (2000000-400000)/4
#define CNT 10

// Start the memory test some bytes above heap start
// as stdio needs the heap for buffers (40000 bytes reserved)
// Now hardcoded for merging with bootable
extern char _end;
// #define TEST_START ((volatile _UNCACHED int *) (&_end)+10000)
#define TEST_START ((volatile _UNCACHED int *) 250000)

int main() {
	int res;
	int error = 0;
	int test = 0;

 printf("%d %d\n", (int) TEST_START, (int) &_end);

	if (get_cpuid() == 0) {
		// MS: does the following reading from uninitialized memory
		// make sense?
//		for (int i=0; i<=LENGTH; i++){ // Read from main memory
//			res = *(TEST_START+i);
//			if (res != 0){	// If data is not what we expect write error
//				error++;
//			}
//		}
//		if (error != 0){
//			puts("TEST_STARTORY uninitialized\n");
//		}
//		error = 0;

		for (int k = 0; k < CNT; k++) { 
			putchar('.');
			fflush(NULL);
			for (int i=0; i<=LENGTH; i++) // Write to main memory
				*(TEST_START+i) = i;

			for (int i=0; i<=LENGTH; i++){ // Read from main memory
				res = *(TEST_START+i);
				if (res != i){	// If data is not what we expect write error
					puts("e");
					error++;
				}
			}
			if (error != 0){
				test++;
				puts("\n");
			}
			error = 0;
		}
		puts("");
		if (test != 0){
			puts("Errors\n");
		} else {
			puts("Success\n");
		}

	} else {
		//for (int k = 0; k < 100; ++k)
		//{
		//	for (int i=0; i<=LENGTH; i++){ // Read from main memory
		//		res = *(TEST_START+i);
		//		if (res == 0){	// If data is not what we expect write error
		//			error = error;
		//		} else {
		//			error++;
		//		}
		//	}
		//}
	}

	puts("Finished\n");

	return 0;
}
