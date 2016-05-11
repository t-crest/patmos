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

#define SEND_TIMEOUT_DEFAULT 800000 //in cc

volatile long long unsigned int send_timeout = SEND_TIMEOUT_DEFAULT;

#define REMOTE_IRQ_IDX 19 //bit n.3 -- the forth (shifted 16)
#define LOCAL_IRQ_IDX 18 //bit n.2 -- the third (shifted 16)


volatile _UNCACHED int bandwidth_results[NOC_CORES][NOC_CORES];// bandwidth_results[sender][receiver]//this contains the amount of CC needed to send a block (-1 means: channel not available)
volatile _UNCACHED int correctness_results[NOC_CORES][NOC_CORES];// correctness_results[sender][receiver]//this contains the amount of CC needed to send a block (-1 means: channel not available)
volatile _UNCACHED unsigned char interrupt_status[NOC_CORES][NOC_CORES];//[sender][receiver] 0 menas channel not tested, 1 means: interrupt not received, 2 means: interrupt received but not deasseted, 3 means: everyting fine
volatile _UNCACHED unsigned char interrupt_occ[NOC_CORES][NOC_CORES];//[sender][receiver] 0 menas channel not tested, 1 means: interrupt not received, 2 means: interrupt received but not deasseted, 3 means: everyting fine
volatile _UNCACHED unsigned int interrupt_results[NOC_CORES][NOC_CORES];//[sender][receiver]

volatile _UNCACHED int spm_sizes[NOC_CORES];
//volatile _UNCACHED int master_cpu;
//volatile _UNCACHED int master_cpu;
//volatile _UNCACHED int master_cpu;

volatile _UNCACHED int s, d; //global sender and destnation

volatile _UNCACHED int block_size;
volatile _SPM unsigned char * block_base = (volatile _SPM unsigned char *) NOC_SPM_BASE;


volatile _UNCACHED unsigned char * random_array = NULL;
volatile int mode = 0;

/////////////////////////////////////////////////////////////
//Find the SPM sizes
/////////////////////////////////////////////////////////////
// Find the size in bytes
int c_find_mem_size(volatile unsigned int _SPM * mem_addr){
  int init = *(mem_addr);
  int tmp;
  *(mem_addr) = 0xFFEEDDCC;
  int i = 2;
  int j = 0;
  for(j = 0; j < 28; j++) {
    tmp = *(mem_addr+i);
    *(mem_addr+i) = 0;
    if (*(mem_addr) == 0) {
      *(mem_addr+i) = tmp;
      *(mem_addr) = init;
      return i*4;
    }
    i = i << 1;
    if (*(mem_addr) != 0xFFEEDDCC){
      *(mem_addr+i) = tmp;
      *(mem_addr) = init;
      return -1;
    }
    *(mem_addr+i) = tmp;
  }
  *(mem_addr) = init;
  return -1;
}

void m_find_spm_size(){
	spm_sizes[get_cpuid()] = c_find_mem_size((volatile unsigned int _SPM *) NOC_SPM_BASE);
	return;
}

void s_find_spm_size(void* arg){
	spm_sizes[get_cpuid()] = c_find_mem_size((volatile unsigned int _SPM *) NOC_SPM_BASE);
	int ret = 0;
  	corethread_exit(&ret);
	return;
}

void m_collect_platform_info(){
	int slave_param = 0;
	int * retval;
	corethread_t ct;
	for (int i = 0; i < NOC_CORES; i++)
	{
		if (i==NOC_MASTER){
			m_find_spm_size();
		}else{
			ct = (corethread_t)i;
			if(corethread_create(&ct, &s_find_spm_size, (void *) &slave_param)){
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
	return;
}

int m_get_max_spm_sizes(){
	int max = spm_sizes[0];
	for (int i = 1; i < NOC_CORES; i++){
		if(spm_sizes[i]>max){
			max = spm_sizes[i];
		}
	}
	return max;
}

int m_get_min_spm_sizes(){
	int min = spm_sizes[0];
	for (int i = 1; i < NOC_CORES; i++){
		if(spm_sizes[i]<min){
			min = spm_sizes[i];
		}
	}
	return min;
}

/////////////////////////////////////////////////////////////
//Printing functions
/////////////////////////////////////////////////////////////
void m_print_test_parameters(){
	printf("The tests are executed with the following parameters:\n\n");
	printf("Send timeout: %llu clock cycles - (default: %d clock cycles)\n", send_timeout, SEND_TIMEOUT_DEFAULT);
	printf("Data block base: 0x%08X - (default: 0x%08X)\n", (unsigned int) block_base, (unsigned int) NOC_SPM_BASE);
	printf("Data block size: %u bytes - (default: %u bytes)\n", block_size, m_get_min_spm_sizes());
	printf("Mode of operation: %d\n", mode);
	return;
}

void m_print_bandwidth_results(){
	printf("\nBandwith results:\n\n");
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
	
		printf("\nNotes: The table shows the clock cycles needed to transmit %d bytes\nbetween every couple of cores. 'N/A' means that the channel does not\nexists or the trasmission timed-out.\n\n", block_size);
	
	return;
}

void m_print_correctness_results(){
	printf("\nCorrectness results:\n\n");
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
	
		printf("\nNotes: The table shows if the transmission of a random generated data\nblock of %d bytes is correctly performed.\n\n", block_size);
	
	return;
}

void m_print_spm_sizes(){
	printf("CPU #:\t");
	for (int i = 0; i < NOC_CORES; i++) {
		printf("%d\t", i);
	}
	printf("\n");
	printf("Size:\t");
	for (int i = 0; i < NOC_CORES; i++) {
		printf("%d\t", spm_sizes[i]);
	}
	printf("\n");
	return;
}

void m_print_interrupt_results(){
	printf("\nInterrupt results:\n\n");
	printf("\tto:\nfrom:\t");
	for (int i = 0; i < NOC_CORES; i++) {
		printf("%d\t", i);
	}
	printf("\n");
	for (int i = 0; i < NOC_CORES; i++) {
		printf("%d\t", i);
		for (int j = 0; j < NOC_CORES; j++) {
			if(i!=j){
				printf("%04X\t", interrupt_results[i][j]);
			}else{	
				printf("-\t");
			}
		}
			printf("\n");
	}
	
		printf("\nNotes: The table shows, in hexadecimal the last value read from\nthe data irq fifo. This should be the word-based\naddress of the last received word.\n");
	
	return;
}

void m_print_interrupt_status(){
	printf("\nInterrupt status:\n\n");
	printf("\tto:\nfrom:\t");
	for (int i = 0; i < NOC_CORES; i++) {
		printf("%d\t", i);
	}
	printf("\n");
	for (int i = 0; i < NOC_CORES; i++) {
		printf("%d\t", i);
		for (int j = 0; j < NOC_CORES; j++) {
			if(interrupt_status[i][j]==0){
				printf("-\t");
			}else if(interrupt_status[i][j]==1){	
				printf("NO IRQ\t");
			}else if(interrupt_status[i][j]==2){
				printf("NO DEA.\t");
			}else{
				printf("DEA.\t");
			}
		}
			printf("\n");
	}
	
	printf("\nNotes: The table shows the data interrupt status. 'DEA.' means that the\ninterrupt was deasserted. 'NO DEA.' means the opposite.\n'NO IRQ' menas that the interrupt was never asseted.\n\n");
	
	return;
}

void m_print_interrupt_occ(){
	printf("\nInterrupt fifo elements:\n\n");
	printf("\tto:\nfrom:\t");
	for (int i = 0; i < NOC_CORES; i++) {
		printf("%d\t", i);
	}
	printf("\n");
	for (int i = 0; i < NOC_CORES; i++) {
		printf("%d\t", i);
		for (int j = 0; j < NOC_CORES; j++) {
			if(interrupt_status[i][j]<=1){
				printf("-\t");
			}else{
				printf("%d\t", interrupt_occ[i][j]);
			}
		}
		printf("\n");
	}
	printf("\nNotes: The table shows how many elements where in the data fifo before\nthe interrupt de-asserted (1 is the expected value.\n\n");
	return;
}

/////////////////////////////////////////////////////////////
//Arrays initializations functions
/////////////////////////////////////////////////////////////
void m_clear_interrupt_status(){
	for (int i = 0; i < NOC_CORES; i++) {
		for (int j = 0; j < NOC_CORES; j++) {
			interrupt_status[i][j] = 0;
		}
	}
	return;
}

void m_clear_interrupt_occ(){
	for (int i = 0; i < NOC_CORES; i++) {
		for (int j = 0; j < NOC_CORES; j++) {
			interrupt_occ[i][j] = 0;
		}
	}
	return;
}

void m_clear_interrupt_results(){
	for (int i = 0; i < NOC_CORES; i++) {
		for (int j = 0; j < NOC_CORES; j++) {
			interrupt_results[i][j] = 0;
		}
	}
	return;
}

void m_clear_bandwidth_results(){
	for (int i = 0; i < NOC_CORES; i++) {
		for (int j = 0; j < NOC_CORES; j++) {
			bandwidth_results[i][j] = -1;
		}
	}
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

void m_generate_random_array(){
	for (int i=0; i<block_size; i++) {
		random_array[i] = ((unsigned char) (rand() & 0x000000FF));
	}
}


/////////////////////////////////////////////////////////////
//Functions related to the generation of bandwidth results
/////////////////////////////////////////////////////////////
void c_generate_bandwidth_results(){ //bandwith results need to be cleared
	volatile _SPM unsigned char * block = ((volatile _SPM unsigned char *) block_base);
	//Initialize memorycontent with incremental values
	for (int i = 0; i < block_size; i++) {
		block[i] = (unsigned char)(i & 0x000000FF);
	}
	long long unsigned int t1, t2, tmax;
	bool timeout = false;
	//measure
	//I need to put nocsend in the cache therefore every measure is repeated twice
	for (int c = 0; c < 2*NOC_CORES; c++) {
		int i=c/2;
		timeout=false;
		if(i == get_cpuid()){
			bandwidth_results[i][i] = 0;
		}else{
			t1 = get_cpu_cycles();
			tmax = t1 + send_timeout;
			//start to send the block
			noc_write((unsigned) i, ((volatile _SPM void *) block_base), ((volatile _SPM void *)block), (size_t) block_size, 0);

			//check for timeout (this adds a tolerance on the result)
			while( !(timeout || (noc_dma_done((unsigned)i))) ){
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


/////////////////////////////////////////////////////////////
//Functions related to the generation of correctness results
//and data interrupts results
/////////////////////////////////////////////////////////////
void c_send_random_array(){ //bandwith results need to be cleared
	volatile _SPM unsigned char * block = ((volatile _SPM unsigned char *) block_base);
	//Initialize memorycontent with incremental values
	for (int i = 0; i < block_size; i++) {
		block[i] = random_array[i];
	}
	noc_write((unsigned) d, ((volatile _SPM void *) block_base), ((volatile _SPM void *) block), (size_t) block_size, 1);
	//check for timeout (this adds a tolerance on the result)
	while( !(noc_dma_done((unsigned)d)) ){;}
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

void c_check_correctness(){ //bandwith results need to be cleared
	volatile _SPM unsigned char * block = ((volatile _SPM unsigned char *) block_base);
	//Initialize memorycontent with incremental values
	bool correct = true;
	for (int i = 0; i < block_size; i++) {
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
	
	//Check if the local IRQ addr is correct ()
	if ((intr_get_pending() & (1 << LOCAL_IRQ_IDX)) != 0){
	
		for (int i = 1; i <= 256; i++)
		{
			interrupt_occ[s][d] = (unsigned char)i;
			//read fifo
			interrupt_results[s][d] = (unsigned int)(*(NOC_IRQ_BASE+1));
			//clear the pending
			intr_clear_pending(LOCAL_IRQ_IDX);
			if ((intr_get_pending() & (1 << LOCAL_IRQ_IDX)) == 0){
				break;
			}
		}
		if ((intr_get_pending() & (1 << LOCAL_IRQ_IDX)) == 0){
			//deasserted
			interrupt_status[s][d]=	3;
		}else{
			//not deasserted
			interrupt_status[s][d]=	2;
		}
	}else{
		//no interrupt
		interrupt_status[s][d]=1;
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

void m_bandwidth_correctness_test(){
	int slave_param = 0;//not used now
	int * retval;
	corethread_t ct;
	for (int i = 0; i < NOC_CORES; i++)
	{
		if (i==NOC_MASTER){
			//The master does the measure
			//////printf("Bandwidth test: master core (%d) is testing.\n", NOC_MASTER);
			m_generate_bandwidth_results();
		}else{
			//////printf("Bandwidth test: core %d is testing.\n", i);
			ct = (corethread_t)i;
			if(corethread_create(&ct, &s_generate_bandwidth_results, (void *) &slave_param)){
				//printf("Corethread not created.\n");
			}
			if(corethread_join(ct, (void**) &retval)){
				//printf("Corethread not joined.\n");
			}
		}
	}

	m_clear_correctness_results();
	m_clear_interrupt_status();
	m_clear_interrupt_results();
	m_clear_interrupt_occ();
	s = 0;
	d = 0;
	for (s = 0; s < NOC_CORES; s++){
		for (d = 0; d < NOC_CORES; d++)
		{
			if (bandwidth_results[s][d]>0){
				//////printf("Correctness test: form core %d to core %d.\n", s, d);
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
	return;
}

/////////////////////////////////////////////////////////////
//User interface main 
/////////////////////////////////////////////////////////////
int main() {
	long long unsigned int t1, t2;
	//Print the header
	printf("\n-------------------------------------------------------------------------------\n");
	printf("-                   C test application for the Argo 2.0 NOC                   -\n");
	printf("-                                     ;-)                                     -\n");
	printf("-------------------------------------------------------------------------------\n\n");

	m_collect_platform_info();

	block_size = m_get_min_spm_sizes();

	//allocate main memory for the random array,
	random_array = (volatile _UNCACHED unsigned char *) malloc(m_get_max_spm_sizes()* sizeof(unsigned char));
	if (random_array == NULL){
		printf("Dynamic memory allocation faild.\n");
		return -1;
	}

	//Main loop
	bool loop = true;
	char c = 'v';
	while (loop) {
		printf("\n-------------------------------------------------------------------------------");
		printf("\nAvailable operations:\n1 -> Test bandwidth, tramission correctness, and data interrupts.\n2 -> Change mode (actual: M%d)\n3 -> Print test parameters\n4 -> Change test parameters\n5 -> Print platform info\n6 -> Perform and test NOC intilization\ne -> exit\n", mode);

		printf("\nSelect operation: ");
		scanf(" %c", &c);
		while ((c != 'e') && (c != '1') && (c != '2') && (c != '3')	&& (c != '4') && (c != '5') && (c != '6')) {
			printf("Operation not valid! Select operation: ");
			scanf(" %c", &c);
		};
		printf("\n-------------------------------------------------------------------------------");
		switch (c) {
			case '1':
				printf("\nOperation 1: Test bandwidth, tramission correctness, and data interrupts.\n\n");
				m_clear_bandwidth_results();
				m_bandwidth_correctness_test();
				m_print_test_parameters();
				m_print_bandwidth_results();
				m_print_correctness_results();
				m_print_interrupt_status();
				m_print_interrupt_occ();
				m_print_interrupt_results();

				//m_print_debug();

				break;
			case '2':
				printf("\nOperation 2: Change mode (actual: M%d)\n Type new mode: ", mode);
				int new_mode = mode;
				scanf ("%d",&new_mode);
				mode=new_mode;
				noc_sched_set(mode);
				printf("Done!\n\n");
				break;
			case '3':
				printf("\nOperation 3: Print test parameters\n\n");
				m_print_test_parameters();
				break;
			case '4':
				printf("\nOperation 4: Change test parameters\n\n");
				break;

			case '5':
				printf("\nOperation 5: Print platform info\n\n");
				printf("SPM max size: %d\n", m_get_max_spm_sizes());
				printf("SPM min size: %d\n", m_get_min_spm_sizes());
				m_print_spm_sizes();
				break;
			case '6':
				printf("\nOperation 6: Perform and test NOC initialization\n\n");
				t1 = get_cpu_cycles();
				noc_init();
				t2 = get_cpu_cycles();
				printf("%llu clock cycles\n", t2-t1);
				t1 = get_cpu_cycles();
				noc_init();
				t2 = get_cpu_cycles();
				printf("%llu clock cycles\n", t2-t1);
				break;
			case 'e':
				loop = false;
				break;

		}
	}
	printf("\n\nGoodbye!\n");

	free((void *)random_array);
	random_array = NULL;
	return 0;
}
