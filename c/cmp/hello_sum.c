/*
    A small demo program demostrating the use of argo noc for communication between patmos processors
    Master core initiates a message that is sent from one core to the next (based on get_cpuid()). 
    Each core adds its ID to the sum of ids and forwards the message to the next core,
    until it reches the master again.

    Author: Evangelia Kasapaki
    Copyright: DTU, BSD License

*/

const int NOC_MASTER = 0;
#include <string.h>
#include <machine/spm.h>
#include <stdio.h>
#include "libnoc/noc.h"
//#include "include/patio.h"
#include "libcorethread/corethread.h"

void blink(int nblinks);

void master(void);

void slave(void* param);

/*/////////////////////////////////////////////////////////////////////////
// Main application
/////////////////////////////////////////////////////////////////////////*/

int main() {
	int slave_param = 1;

	// Clear scratch pad in all cores
	// 16+16 integers
	int i;
	for(i = 0; i < get_cpucnt()*4; i++) {
		*(NOC_SPM_BASE+i) = 0;
		*(NOC_SPM_BASE+get_cpucnt()*4+i) = 0;
	}

	for(int i = 0; i < get_cpucnt(); i++) {
		if (i != NOC_MASTER) {
			corethread_t ct = (corethread_t)i;
			if(corethread_create(&ct,&slave,(void*)slave_param) != 0){
				printf("Corethread %d not created\n",i);
			}
		}
	}

	master();

	int* ret;
	for (int i = 0; i < get_cpucnt(); ++i) {
		if (i != NOC_MASTER) {
			corethread_join((corethread_t)i,(void**)&ret);
			//printf("Slave %d joined\n",i);
		}
	}

  	return 0;
}

void blink(int nblinks) {

	volatile _SPM int *led_ptr = (volatile _SPM int *) 0xF0090000;

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


void master(void) {
	//blink(6);

	volatile _SPM char *spm_base = (volatile _SPM char *) NOC_SPM_BASE;		//0
	volatile _SPM char *spm_slave = spm_base+get_cpucnt()*16;	 			//64

	// message to be send
        const char *msg_snd = "Hello slaves sum_id:0";
	char msg_rcv[22];

	// put message to spm
	int i;
	for (i = 0; i < 21; i++) {
		*(spm_base+i) = *(msg_snd+i);
	}
	//*(spm_base+i) = '\0';

	puts("MASTER: sending\n");

	// send message
	noc_send(1, spm_slave, spm_base, 21); //21 bytes

	puts("MASTER: message sent: ");
	puts(msg_snd);
	puts("\n");

	// wait and poll
	while(*(spm_slave+20) == 0) {;}
	blink(6);
	puts("MASTER: finished polling\n");

        // received message
	puts("MASTER: message received:");
	// copy message to static location and print
	for (i = 0; i < 21; i++) {
		msg_rcv[i] = *(spm_slave+i);
	}
	msg_rcv[i] = '\0';
	puts(msg_rcv);
	return;
}

void slave(void* param) {

	volatile _SPM char *spm_base = (volatile _SPM char *) NOC_SPM_BASE; 			//0
	volatile _SPM char *spm_slave = spm_base+get_cpucnt()*16;	 				//64

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
	noc_send(rcv_id, spm_slave, spm_slave, 21);

	return;
}


