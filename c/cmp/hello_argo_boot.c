/*
    A small bootable example intended for testing Argo NoC in simulation. It demonstrates
	a 1-to-all communication of a single packet message.

    Author: Eleftherios Kyriakakis
    Copyright: DTU, BSD License

*/
#include "include/bootable.h"
#include "libnoc/noc.h"

#include "libmp/mp.h"
#include "include/debug.h"


const int NOC_MASTER = 0;

#define MP_CHAN_SHORTS_AMOUNT 2

#define MP_CHAN_1_ID 1
#define MP_CHAN_1_NUM_BUF 1
#define MP_CHAN_1_MSG_SIZE MP_CHAN_SHORTS_AMOUNT * 2 // 2 shorts = 4 bytes (actually always multiples of 4)

#define MP_CHAN_2_ID 2
#define MP_CHAN_2_NUM_BUF 1
#define MP_CHAN_2_MSG_SIZE MP_CHAN_SHORTS_AMOUNT * 2 // 2 shorts = 4 bytes

int main(void) __attribute__((noreturn));
int main() {
	volatile _SPM char *spm_base = (volatile _SPM char *) NOC_SPM_BASE;
	volatile _SPM char *spm_slave = spm_base+get_cpucnt()*16;

	if(get_cpuid()==NOC_MASTER)	LEDS = 255;

	noc_configure();

	if(get_cpuid()==NOC_MASTER)	LEDS = 127;

	noc_enable();

	if(get_cpuid()==NOC_MASTER)	LEDS = 63;

	for(int i = 0; i < get_cpucnt()*4; i++) {
		*(NOC_SPM_BASE+i) = 0;
		*(NOC_SPM_BASE+get_cpucnt()*4+i) = 0;
	}

	if(get_cpuid()==NOC_MASTER){
		// send message at spm_base to all nodes
		for(int i=1; i < get_cpucnt(); i++){
			// put message to spm
			WRITE("\ntx:", 4);
			LEDS = 31;
			for (int j = 0; j < 4; j++) {
				*(spm_base+j) = i;
			}
			LEDS = 15;
			// put message in noc
			noc_write(i, spm_slave, spm_base, 4, 0); //4 bytes
			LEDS = 7;
			// wait and poll
			while(*(spm_slave+3) == 0) {;}
			LEDS = 3;
			// received something
			WRITE("\nrx:", 4);
			WRITE(spm_slave, 4);
		}
	} else {
		// wait and poll
		while(*(spm_slave+3) == 0) {;}
		for (int i = 0; i < 4; i++) {
			*(spm_slave+i) = 4;
		}
		noc_write(0, spm_slave, spm_slave, 4, 0);
	}

	if(get_cpuid()==NOC_MASTER)	LEDS = 1;
}


