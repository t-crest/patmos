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
	volatile _SPM int *led_ptr  = (volatile _SPM int *) PATMOS_IO_LED;
	int i, j;
	
	int a[8000];
	for(int i = 0; i < 8000; i++){
		a[i] = i;
	}

	char print = '0';
	for(int i = 0; i < 8000; i++){
		for(int j = 7; j >= 0; j--){
			int hex = (a[i] >> (32 - j*4)) & 15;
			if(hex < 10){
				print = '0'+hex;
				
			}else{
				print = 'A' + (hex-10);
			}
			WRITECHAR(print);
		}
		WRITECHAR(' ');
	}
}