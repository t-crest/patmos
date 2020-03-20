/*
    A small bootable example intended for testing Argo NoC in simulation. It demonstrates
	a 1-to-all communication of a single packet message.

    Author: Eleftherios Kyriakakis
    Copyright: DTU, BSD License

*/
#include "include/bootable.h"
#include "libnoc/noc.h"
#include "include/debug.h"


const int NOC_MASTER = 0;

#define MP_CHAN_SHORTS_AMOUNT 2

#define MP_CHAN_1_ID 1
#define MP_CHAN_1_NUM_BUF 1
#define MP_CHAN_1_MSG_SIZE MP_CHAN_SHORTS_AMOUNT * 2 // 2 shorts = 4 bytes (actually always multiples of 4)

#define MP_CHAN_2_ID 2
#define MP_CHAN_2_NUM_BUF 1
#define MP_CHAN_2_MSG_SIZE MP_CHAN_SHORTS_AMOUNT * 2 // 2 shorts = 4 bytes

#define KEYS *((volatile _SPM unsigned int *) (PATMOS_IO_KEYS))

int main(void) __attribute__((noreturn));
int main() {
	volatile _SPM char *spm_base = (volatile _SPM char *) NOC_SPM_BASE;
	volatile _SPM char *spm_slave = spm_base+get_cpucnt()*16;

	noc_init();
	
	for(int i = 0; i < get_cpucnt()*4; i++) {
		*(NOC_SPM_BASE+i) = 0;
		*(NOC_SPM_BASE+get_cpucnt()*4+i) = 0;
	}

	do{

		if(get_cpuid()==NOC_MASTER){
			// send message at spm_base to all nodes
			for(int core_id=1; core_id < get_cpucnt(); core_id++){
				// put message to spm
				WRITE("-tx:", 4);
				LEDS = 15;
				for (int j = 0; j < 4; j++) {
					spm_base[j] = j+1+'0';
				}
				WRITE(spm_base, 4);
				LEDS = 7;
				// put message in noc
				noc_write(core_id, spm_slave, spm_base, 4, 0); //4 bytes
				LEDS = 3;
				// wait and poll
				while(spm_slave[3] == 0) {;}
				// received something
				LEDS = 1;
				WRITE("-rx:", 4);
				WRITE(spm_slave, 4);
				//clear buffer
				for(int j=0; j < 4; j ++){
					spm_slave[j] = 0;
				}
			}
		} else {
			// wait and poll
			while(*(spm_slave+3) == 0) {;}
			for (int i = 0; i < 4; i++) {
				spm_slave[i] += get_cpuid();
			}
			noc_write(0, spm_slave, spm_slave, 4, 0);
			while (noc_dma_done(0)){continue;}
			for(int i=0; i < 4;i++){
				spm_slave[i] = 0;
			}
		}

		if(get_cpuid()==NOC_MASTER)	LEDS = 0;

		while(KEYS != 0xE){continue;}

	}while(1);

}