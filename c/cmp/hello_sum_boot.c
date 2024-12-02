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


#define MP_CHAN_SHORTS_AMOUNT 2

#define MP_CHAN_1_ID 1
#define MP_CHAN_1_NUM_BUF 1
#define MP_CHAN_1_MSG_SIZE MP_CHAN_SHORTS_AMOUNT * 2 // 2 shorts = 4 bytes (actually always multiples of 4)

#define MP_CHAN_2_ID 2
#define MP_CHAN_2_NUM_BUF 1
#define MP_CHAN_2_MSG_SIZE MP_CHAN_SHORTS_AMOUNT * 2 // 2 shorts = 4 bytes

void master() {
	volatile _SPM char *spm_base = (volatile _SPM char *) NOC_SPM_BASE;
	volatile _SPM char *spm_slave = spm_base+get_cpucnt()*16;

	// message to be send
        const char *msg_snd = "Hello slaves sum_id:0";
	char msg_rcv[22];

	// put message to spm
	int i;
	for (i = 0; i < 21; i++) {
		*(spm_base+i) = *(msg_snd+i);
	}

	WRITE("MASTER: sending\n",16);

	// send message
	noc_write(1, spm_slave, spm_base, 21, 0); //21 bytes

	WRITE("MASTER: message sent: ",22);
	WRITE(msg_snd,21);
	WRITE("\n",1);

	// wait and poll
	while(*(spm_slave+20) == 0) {;}
	WRITE("MASTER: finished polling\n",25);

        // received message
	WRITE("MASTER: message received:",25);
	// copy message to static location and print
	for (i = 0; i < 21; i++) {
		msg_rcv[i] = *(spm_slave+i);
	}
	msg_rcv[i] = '\0';
	WRITE(msg_rcv,21);
	WRITE("\n",1);
	return;
}

void slave() {
	volatile _SPM char *spm_base = (volatile _SPM char *) NOC_SPM_BASE; 
	volatile _SPM char *spm_slave = spm_base+get_cpucnt()*16;

	// wait and poll until message arrives
	while(*(spm_slave+20) == 0) {;}

	// put message for master to spm
	const char *msg = "Hello master ";
	int i;
	for (i = 6; i < 12; i++) {
		*(spm_slave+i) = *(msg+i);
	}

	// PROCESS : add ID to sum_id
	*(spm_slave+20) = *(spm_slave+20) + get_cpuid();

	// send to next slave
	int rcv_id = (get_cpuid()==(get_cpucnt()-1))? 0 : get_cpuid()+1;
	noc_write(rcv_id, spm_slave, spm_slave, 21, 0);

	return;
}


/*/////////////////////////////////////////////////////////////////////////
// Main application
/////////////////////////////////////////////////////////////////////////*/

int main(void) __attribute__((noreturn));

int main() {
	noc_init();

	// Clear scratch pad in all cores
	// 16+16 integers
	int i;
	for(i = 0; i < get_cpucnt()*4; i++) {
		*(NOC_SPM_BASE+i) = 0;
		*(NOC_SPM_BASE+get_cpucnt()*4+i) = 0;
	}

	if (get_cpuid() == NOC_MASTER) {
		master();	
	}else{
		slave();
	}

}


