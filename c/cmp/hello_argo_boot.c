/*
    A small BOOTABLE demo program demostrating the use of the Argo noc for communication between patmos 
    processors Master core initiates a message that is sent from one core to the next (based on get_cpuid()). 
    Each core adds its ID to the sum of ids and forwards the message to the next core,
    until it reches the master again.

    Author: Evangelia Kasapaki
    Adapted to be bootable by: Luca Pezzarossa
    Copyright: DTU, BSD License

*/

#include <machine/spm.h>
#include <machine/exceptions.h>
#include "include/bootable.h"
#include "include/patio.h"
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

#define LED ( *( ( volatile _IODEV unsigned * ) 0xF0090000 ) )

void master() {
	volatile _SPM char *spm_base = (volatile _SPM char *) NOC_SPM_BASE;
	volatile _SPM char *spm_slave = spm_base+get_cpucnt()*16;

	// message to be send
    const char *msg_snd = "ABCD";

	// put message to spm
	int i;
	for (i = 0; i < 4; i++) {
		*(spm_base+i) = *(msg_snd+i);
        LED |= i;
	}

    int statusLeds = LED;

	// WRITE("M:msg=\n",8);
	// WRITE(msg_snd,4);
	// WRITE("\n",1);

	// send message
    LED = 0x0;
	noc_write(1, spm_slave, spm_base, 4, 0); //4 bytes
    LED = statusLeds;

	// WRITE("M->Core#1",9);
	// WRITE("\n",1);

	// send message
    LED = 0x0;
	noc_write(2, spm_slave, spm_base, 4, 0); //4 bytes
    LED = statusLeds;

	// WRITE("M->Core#2",9);
	// WRITE("\n",1);

	// send message
    LED = 0x0;
	noc_write(3, spm_slave, spm_base, 4, 0); //4 bytes
    LED = statusLeds;

	// WRITE("M->Core#3",9);
	// WRITE("\n",1);

	return;
}

void slave() {
	volatile _SPM char *spm_base = (volatile _SPM char *) NOC_SPM_BASE;
	volatile _SPM char *spm_slave = spm_base+get_cpucnt()*16;

	// initialize polling address to 0
	*(spm_slave + 3) = 0;

	// wait and poll until message arrives
	while(*(spm_slave+3) == 0) {;}

	return;
}


/*/////////////////////////////////////////////////////////////////////////
// Main application
/////////////////////////////////////////////////////////////////////////*/

int main(void) __attribute__((noreturn));

int main() {

	if(get_cpuid()==NOC_MASTER) LED = 256;

	int i;
	for(i = 0; i < get_cpucnt()*4; i++) {
		*(NOC_SPM_BASE+i) = 0;
		*(NOC_SPM_BASE+get_cpucnt()*4+i) = 0;
		if(get_cpuid()==NOC_MASTER){
			if(*(NOC_SPM_BASE+i) != 0 || *(NOC_SPM_BASE+get_cpucnt()*4+i) != 0){
				WRITE("spm_init = X\n", 14);
			}
		}
	}
	
	if(get_cpuid()==NOC_MASTER) WRITE("spm_init = O\n", 14);

	unsigned int tdmval = *NOC_TDM_BASE;
	unsigned int schval = *NOC_SCHED_BASE;
	unsigned int dmaval = *NOC_DMA_BASE;

	if(get_cpuid()==NOC_MASTER){
		if(tdmval == 0x4000){
			WRITE("tdmval = O\n", 12);
		} else {
			WRITE("tdmval = X\n", 12);
		}

		if(schval == 0x2000){
			WRITE("schval = O\n", 12);
		} else {
			WRITE("schval = X\n", 12);
		}

		if(dmaval == 0x0000){
			WRITE("dmaval = O\n", 12);
		} else {
			WRITE("dmaval = X\n", 12);
		}
	}
    
	if(get_cpuid()==NOC_MASTER) LED = 128;

	// noc_configure();
	// noc_enable();

    if(get_cpuid()==NOC_MASTER) LED = 64;

	// if (get_cpuid() == NOC_MASTER) {
    // 	master();
	// } else {
	// 	slave();
	// }

	if(get_cpuid()==NOC_MASTER) 
		LED = 511;

	// return 0;
}


