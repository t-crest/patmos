/*
    A small demo program demostrating the use of argo noc for communication between patmos processors
    Master core initiates a message that is sent from one core to the next (based on CORE_ID). 
    Each core adds its ID to the sum of ids and forwards the message to the next core,
    until it reches the master again.

    Author: Evangelia Kasapaki
    Copyright: DTU, BSD License

*/

#define CORES 4

const int NOC_MASTER = 0;
#include <string.h>
#include <machine/spm.h>
#include "libnoc/noc.h"
#include "patio.h"

static void blink(int nblinks);

static void master(void);

static void slave(void);

/*/////////////////////////////////////////////////////////////////////////
// Main application
/////////////////////////////////////////////////////////////////////////*/

int main() {


    	// Clear scratch pad in all cores
	// 16+16 integers
	int i;
    	for(i = 0; i < NOC_CORES*4; i++) {
        	*(NOC_SPM_BASE+i) = 0;
		*(NOC_SPM_BASE+NOC_CORES*4+i) = 0;
    	}

	//if (CPU_ID == 0) {
	if(CORE_ID == 0) {
    		master();
  	} else {
    		slave();
  	}

  	return 0;
}

static void blink(int nblinks) {

	volatile _SPM int *led_ptr = (volatile _SPM int *) 0xF0000900;

	// Blinking lights
        int k, i, j;
        for (k = 0; k < nblinks; k++)
        {
            //UART_DATA = '1';
            for (i=2000; i!=0; --i)
                for (j=2000; j!=0; --j)
                    *led_ptr = 1;


            //UART_DATA = '0';
            for (i=2000; i!=0; --i)
                for (j=2000; j!=0; --j)
                    *led_ptr = 0;

        }
	return;
}


static void master(void) {
	blink(6);

	volatile _SPM char *spm_base = (volatile _SPM char *) NOC_SPM_BASE;		//0
	volatile _SPM char *spm_slave = spm_base+NOC_CORES*4*4; 			//64

	// message to be send
        const char *msg = "Hello slaves sum_id:0";

	// put message to spm
	int i;
	for (i = 0; i < 21; i++) {
		*(spm_base+i) = *(msg+i);
	}
//	*(spm_base+i) = '\0';

	// send message
	noc_send(1, spm_slave, spm_base, 21); //12 bytes

	WRITE("MASTER: message sent: ",22);
	WRITE(spm_base, 21);
	WRITE("\n",1);

	// wait and poll
	while(*(spm_slave+20) == 0) {;}
	blink(6);
	WRITE("MASTER: finished polling\n",26);

        // received message
	WRITE("MASTER: message received: ",26);
        WRITE(spm_slave, 21);//strlen(NOC_SPM_BASE+NOC_CORES*4));
        WRITE("\n",1);

	return;
}

static void slave(void) {

	volatile _SPM char *spm_base = (volatile _SPM char *) NOC_SPM_BASE; 			//0
	volatile _SPM char *spm_slave = spm_base+NOC_CORES*4*4; 				//64

	// wait and poll until message arrives
	while(*(spm_slave+20) == 0) {;}

	// put message for master to spm
        const char *msg = "Hello master ";
	int i;
	for (i = 6; i < 12; i++) {
		*(spm_slave+i) = *(msg+i);
	}

	// PROCESS : add ID to sum_id
	*(spm_slave+20) = *(spm_slave+20) + CORE_ID;

	// send to next slave
	int rcv_id = (CORE_ID==3)? 0 : CORE_ID+1;
	noc_send(rcv_id, spm_slave, spm_slave, 21);

	return;
}


