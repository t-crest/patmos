/*
    This is program for testing cashe and external memory by writing a long array and reading it back

    Author: Anthon V. Riber
    Copyright: DTU, BSD License
*/

#include "include/bootable.h"
#include "include/patio.h"
#include <machine/patmos.h>
#include <machine/spm.h>

#define LOOP_DELAY 100

int main() {
	global_mode();
	volatile _SPM int *led_ptr  = (volatile _SPM int *) PATMOS_IO_LED;
	int i, j;
	
	char a[8000];
	for(int i = 0; i < 8000; i++){
		a[i] = '0' + (i % 10);
	}

	for(int i = 0; i < 8000; i++){
		WRITECHAR(a[i]);
	}
}