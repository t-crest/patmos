/*
	C test application for the Argo 2.0 NOC.

	Author: Luca Pezzarossa (lpez@dtu.dk)
	Copyright: DTU, BSD License
*/

const int NOC_MASTER = 0;
#define CORETHREAD_INIT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <machine/patmos.h>
//#include <machine/exceptions.h>
//#include <machine/rtc.h>
//#include <machine/boot.h>
#include "libnoc/noc.h"
//#include "libmp/mp.h"
#include "libcorethread/corethread.h"
#include <math.h>

//this should not be needed, NOC_CORES should be used instead, but it gives errors.
#define NOC_CORES 9

#define SEND_TIMEOUT 800000 //in cc

volatile _UNCACHED int bandwidth_results[NOC_CORES][NOC_CORES];// bandwidth_results[sender][receiver]//this contains the amount of CC needed to send a block (-1 means: channel not available)
volatile _UNCACHED int correctness_results[NOC_CORES][NOC_CORES];// correctness_results[sender][receiver]//this contains the amount of CC needed to send a block (-1 means: channel not available)

volatile _UNCACHED int s, d; //global sender and destnation

#define BLOCK_SIZE 4096 //blocksize in bites

volatile _UNCACHED unsigned char random_array[BLOCK_SIZE];

void m_generate_random_array()
{
	for (int i=0; i<BLOCK_SIZE; i++) {
		random_array[i] = ((unsigned char) (rand() & 0x000000FF));
	}
}

void m_clear_bandwidth_results(){
	for (int i = 0; i < NOC_CORES; i++) {
		for (int j = 0; j < NOC_CORES; j++) {
			bandwidth_results[i][j] = -1;
		}
	}
	return;
}

void m_print_bandwidth_results(){
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
			}else if(bandwidth_results[i][j]==0){	
				printf("-\t");
			}else{
				printf("%d\t", bandwidth_results[i][j]);
			}
		}
		printf("\n");
	}
	return;
}

//common
void c_generate_bandwidth_results(){ //bandwith results need to be cleared
	volatile _SPM unsigned char * block = ((volatile _SPM unsigned char *) NOC_SPM_BASE);
	//Initialize memorycontent with incremental values
	for (int i = 0; i < BLOCK_SIZE; i++) {
		block[i] = (unsigned char)(i & 0x000000FF);
	}
	long long unsigned int t1, t2, tmax;
	bool timeout = false;

	//I need to put nocsend in the cache
	//t1 = get_cpu_cycles();
	//tmax = t1 + SEND_TIMEOUT;
	////start to send the block, for sure not to myself -> ((get_cpuid()+1)%NOC_CORES)
	//noc_send((unsigned) ((get_cpuid()+1)%NOC_CORES), ((volatile _SPM void *) NOC_SPM_BASE), block, (size_t) BLOCK_SIZE);

	////check for timeout (this adds a tolerance on the result)
	//while( !(timeout || (noc_done((unsigned)((get_cpuid()+1)%NOC_CORES)))) ){
	//	if (get_cpu_cycles()>tmax){
	//		timeout = true;
	//		noc_dma_clear((unsigned)((get_cpuid()+1)%NOC_CORES));
	//	}
	//}

	//measure
	for (int c = 0; c < 2*NOC_CORES; c++) {
		int i=c/2;
		timeout=false;
		if(i == get_cpuid()){
			bandwidth_results[i][i] = 0;
		}else{
			t1 = get_cpu_cycles();
			tmax = t1 + SEND_TIMEOUT;
			//start to send the block
			noc_send((unsigned) i, ((volatile _SPM void *) NOC_SPM_BASE), block, (size_t) BLOCK_SIZE);

			//check for timeout (this adds a tolerance on the result)
			while( !(timeout || (noc_done((unsigned)i))) ){
				if (get_cpu_cycles()>tmax){
					timeout = true;
					noc_dma_clear((unsigned)i);
				}
			}
			//if the sending is done, get the time
			if (!timeout){
			t2 = get_cpu_cycles();
			//calculate and correct the result with #defined values (from experiments)
			bandwidth_results[get_cpuid()][i] = t2-t1;
			}
		}
	}
	//int noc_done(unsigned dma_id);// 1 The transfer has finished. 0 Otherwise.
	//void noc_send(unsigned dma_id, volatile void _SPM *dst, volatile void _SPM *src, size_t size);
	return;
}

void m_generate_bandwidth_results(){
	c_generate_bandwidth_results();
	return;
}

void s_generate_bandwidth_results(void* arg){
	c_generate_bandwidth_results();
	int ret = 0;
  	corethread_exit(&ret);
	return;
}

void m_clear_correctness_results(){
	for (int i = 0; i < NOC_CORES; i++) {
		for (int j = 0; j < NOC_CORES; j++) {
			correctness_results[i][j] = 0; // 0 not tested, 1 correct, 2 not correct
		}
	}
	return;
}

void m_print_correctness_results(){
	printf("\tto:\nfrom:\t");
	for (int i = 0; i < NOC_CORES; i++) {
		printf("%d\t", i);
	}
	printf("\n");
	for (int i = 0; i < NOC_CORES; i++) {
		printf("%d\t", i);
		for (int j = 0; j < NOC_CORES; j++) {
			if(correctness_results[i][j]==0){
				printf("-\t");
			}else if(correctness_results[i][j]==1){	
				printf("OK\t");
			}else if(correctness_results[i][j]==2){	
				printf("WRONG\t");
			}else{
				printf("?\t");
			}
		}
		printf("\n");
	}
	return;
}


//common
void c_send_random_array(){ //bandwith results need to be cleared
	volatile _SPM unsigned char * block = ((volatile _SPM unsigned char *) NOC_SPM_BASE);
	//Initialize memorycontent with incremental values
	for (int i = 0; i < BLOCK_SIZE; i++) {
		block[i] = random_array[i];
	}
	noc_send((unsigned) d, ((volatile _SPM void *) NOC_SPM_BASE), block, (size_t) BLOCK_SIZE);
	//check for timeout (this adds a tolerance on the result)
	while( !(noc_done((unsigned)d)) ){;}
	//int noc_done(unsigned dma_id);// 1 The transfer has finished. 0 Otherwise.
	return;
}

void m_send_random_array(){
	c_send_random_array();
	return;
}

void s_send_random_array(void * arg){//just changed in arg.
	c_send_random_array();
	int ret = 0;
  	corethread_exit(&ret);
	return;
}

//common
void c_check_correctness(){ //bandwith results need to be cleared
	volatile _SPM unsigned char * block = ((volatile _SPM unsigned char *) NOC_SPM_BASE);
	//Initialize memorycontent with incremental values
	bool correct = true;
	for (int i = 0; i < BLOCK_SIZE; i++) {
		if(block[i] != random_array[i]){
			correct=false;
			break;
		}
	}
	if (correct){
		correctness_results[s][d]=1;
	}else{
		correctness_results[s][d]=2;
	}
	return;
}

void m_check_correctness(){
	c_check_correctness();
	return;
}

void s_check_correctness(void * arg){
	c_check_correctness();
	int ret = 0;
  	corethread_exit(&ret);
	return;
}



//   Main application
int main() {
	//Print the header
	printf("\n-------------------------------------------------------------\n");
	printf("-          C test application for the Argo 2.0 NOC          -\n");
	printf("-                           ;-)                             -\n");
	printf("-------------------------------------------------------------\n");

	//Main loop
	bool loop = true;
	char c = 'v';
	int mode = 0;
	while (loop) {
		printf("\nAvailable operations:\n1 -> Test bandwidth and tramission correctness.\n2 -> Change mode (actual: M%d)\n3 -> operation 3\n4 -> operation 4\ne -> exit\n", mode);

		printf("\nSelect operation: ");
		scanf(" %c", &c);
		while ((c != 'e') && (c != '1') && (c != '2') && (c != '3')	&& (c != '4')) {
			printf("Operation not valid! Select operation: ");
			scanf(" %c", &c);
		};
		switch (c) {
			case '1':
				printf("\nOperation 1: test the bandwidth and the tramission correctness of the current schedule.\n");
				m_clear_bandwidth_results();

				int slave_param = 0;//not used now
				int * retval;
				corethread_t ct;
				for (int i = 0; i < NOC_CORES; i++)
				{
					if (i==NOC_MASTER){
						//The master does the measure
						printf("Bandwidth test: master core (%d) is testing.\n", NOC_MASTER);
						m_generate_bandwidth_results();
					}else{
						printf("Bandwidth test: core %d is testing.\n", i);
						ct = (corethread_t)i;
						if(corethread_create(&ct, &s_generate_bandwidth_results, (void *) &slave_param)){
							//printf("Corethread not created.\n");
						}else{
							//printf("Corethread created.\n");
						}
						
						if(corethread_join(ct, (void**) &retval)){
							//printf("Corethread not joined.\n");
						}else{
							//printf("Corethread joined.\n");
						}

					}
				}
				
				m_clear_correctness_results();
				s = 0;
				d = 0;
				for (s = 0; s < NOC_CORES; s++){
					for (d = 0; d < NOC_CORES; d++)
					{
						if (bandwidth_results[s][d]>0){
							printf("Correctness test: form core %d to core %d.\n", s, d);
							m_generate_random_array();
							//check if it is the master that is sending and receiving
							if(s==NOC_MASTER){
								m_send_random_array(); //pass the destination
							}else{
								ct = (corethread_t)(s);
								if(corethread_create(&ct, &s_send_random_array, ((void *) &slave_param))){
									//printf("Corethread send not created.\n");
								}else{
									//printf("Corethread send created.\n");
								}

								if(corethread_join(ct, (void**) &retval)){
									//printf("Corethread send not joined.\n");
								}else{
									//printf("Corethread send joined.\n");
								}
							}

							if(d==NOC_MASTER){
								m_check_correctness(); //pass the sender id
							}else{
								ct = (corethread_t)(d);
								if(corethread_create(&ct, &s_check_correctness, ((void *) &slave_param))){
									//printf("Corethread receiver not created.\n");
								}else{
									//printf("Corethread receiver created.\n");
								}
								if(corethread_join(ct, (void**) &retval)){
									//printf("Corethread receiver not joined.\n");
								}else{
									//printf("Corethread receiver joined.\n");
								}
								//slave is checking
							}




						}

					}
					
				}

				printf("\nBandwith results (clock cycles needed to transmit %d bytes):\n", BLOCK_SIZE);
				m_print_bandwidth_results();
				printf("\nCorrectness results:\n");
				m_print_correctness_results();
				//m_print_debug();


				break;
			case '2':
				printf("\nOperation 2: change mode (actual: M%d)\n Type new mode: ", mode);
				int new_mode = mode;
				scanf ("%d",&new_mode);
				mode=new_mode;
				noc_set_config(mode);
				printf("Done!\n\n");
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
