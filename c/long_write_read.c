/*
    This is bootable program for testing cache and external memory by writing a long array and reading it back

    Author: Anthon V. Riber
    Copyright: DTU, BSD License
*/

#include "include/bootable.h"
#include "include/patio.h"
#include <machine/patmos.h>
#include <machine/spm.h>
#include <stdio.h>

int main() {
	global_mode();
	
	char a[8000];
	for(int i = 0; i < 8000; i++){
		a[i] = '0' + (i % 10);
	}

	for(int i = 0; i < 8000; i++){
		WRITECHAR(a[i]);
	}
}