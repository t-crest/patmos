/*
	C test application for the Argo 2.0 NOC.

	Author: Luca Pezzarossa (lpez@dtu.dk)
	Copyright: DTU, BSD License
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <machine/patmos.h>
#include <machine/spm.h>

#include "libnoc/noc.h"

#define CORETHREAD_INIT
const int NOC_MASTER = 0;
#include "libcorethread/corethread.h"

//#include "libmp/mp.h"
//#include <math.h>

//this should not be needed, NOC_CORES should be used instead, but it gives errors.
//#define NOC_CORES 9

//volatile _UNCACHED int bandwidth_results[NOC_CORES][NOC_CORES];// bandwidth_results[sender][receiver]//this contains the amount of CC needed to send a block (-1 means: channel not available)

volatile _UNCACHED int bandwidth_results[9][9];// bandwidth_results[sender][receiver]//this contains the amount of CC needed to send a block (-1 means: channel not available)


#define BLOCK_SIZE 4096 //blocksize in bites

void clear_bandwidth_results(){
	for (int i = 0; i < NOC_CORES; i++) {
		for (int j = 0; j < NOC_CORES; j++) {
			bandwidth_results[i][j] = -1;
		}
	}
	return;
}

void print_bandwidth_results(){
	printf("\tto:\nfrom:\t");
	for (int i = 0; i < NOC_CORES; i++) {
		printf("%d\t", i);
	}
	printf("\n");
	for (int i = 0; i < NOC_CORES; i++) {
		printf("%d\t", i);
		for (int j = 0; j < NOC_CORES; j++) {
			if(bandwidth_results[i][j]==-1){
				printf("N/A\t");
			}else{
				printf("%d\t", bandwidth_results[i][j]);
			}
		}
		printf("\n");
	}
	return;
}

void generate_bandwidth_results(){ //bandwith results need to be cleared
	volatile _SPM unsigned char * block = ((volatile _SPM unsigned char *) NOC_SPM_BASE);
	
	printf("DEBUG!\n");

	//Initialize memorycontent with incremental values
	for (int i = 0; i < BLOCK_SIZE; i++) {
		block[i] = (unsigned char)(i & 0x000000FF);
		printf("DEBUG! n:%d\n", i);
	}



	long long unsigned t1, t2;
	for (int i = 0; i < NOC_CORES; i++) {
		if(i != get_cpuid()){//do not try to send to yourself
			
			t1 = get_cpu_cycles();
			//start to send the block
			noc_send((unsigned) i, ((_SPM long long unsigned int *) NOC_SPM_BASE), block, (size_t) BLOCK_SIZE);

			//check for timeout (this adds a tolerance on the result)
			while(noc_done((unsigned) i)==1){;}

			//if the sending is done, get the time
			t2 = get_cpu_cycles();

			//calculate and correct the result with #defined values (from experiments)
			bandwidth_results[get_cpuid()][i] = t2-t1;
			//write it in 
		}
		
	}

	//int noc_done(unsigned dma_id);// 1 The transfer has finished. 0 Otherwise.
	//void noc_send(unsigned dma_id, volatile void _SPM *dst, volatile void _SPM *src, size_t size);



	return;
}


//   Main application
int main() {
/*	_SPM
	double *x = ((_SPM double *) NOC_SPM_BASE)+128;
	_SPM
	double *y = ((_SPM double *) NOC_SPM_BASE)+256;
	char c = 'v';
	char s = 'y';
	long long unsigned int dir;
	int m_r;
	int done = 0;
	long long unsigned int m = 4;
	long long unsigned int n;
	long long unsigned int t1, t2, ttot;
*/

/*	// Clear scratch pad in all cores
	for (int i = 0; i < NOC_CORES * 4; i++) {
		*(NOC_SPM_BASE + i) = 0;
		*(NOC_SPM_BASE + NOC_CORES * 4 + i) = 0;
	}
*/
	//Print the header
	printf("\n-------------------------------------------------------------\n");
	printf("-          C test application for the Argo 2.0 NOC          -\n");
	printf("-                           ;-)                             -\n");
	printf("-------------------------------------------------------------\n");

	//Main loop
	bool loop = true;
	char c = 'v';
	while (loop) {
		printf("\nAvailable operations:\n1 -> Test bandwidth of the current schedule\n2 -> operation 2\n3 -> operation 3\n4 -> operation 4\ne -> exit\n");

		printf("\nSelect operation: ");
		scanf(" %c", &c);
		while ((c != 'e') && (c != '1') && (c != '2') && (c != '3')	&& (c != '4')) {
			printf("Operation not valid! Select operation: ");
			scanf(" %c", &c);
		};
		switch (c) {
			case '1':
				printf("\nOperation 1: test bandwidth of the current schedule\n");
				clear_bandwidth_results();
				generate_bandwidth_results();
				print_bandwidth_results();
				break;
			case '2':
				printf("\nOperation 2\n");
				break;
			case '3':
				printf("\nOperation 3\n");
				break;
			case '4':
				printf("\nOperation 4\n");
				break;
			case 'e':
				loop = false;
				break;

		}
	}
	printf("\nGoodbye!\n");
	return 0;
}
