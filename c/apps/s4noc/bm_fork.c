/*
  Producer/fork/2-consumers benchmark for the S4NOC paper.
  Trim the DELAY of the producer until no tokens are lost.

  Author: Martin Schoeberl and Luca Pezzarossa
*/

#include <stdio.h>
#include <machine/spm.h>
#include "../../libcorethread/corethread.h"

#include "s4noc.h"

#define DELAY 1000

#define PRODUCER_CORE 1
#define FORK_CORE 8
#define CONSUMER1_CORE 3
#define CONSUMER2_CORE 4
#define SEND_SLOT_PRODU_TO_FORK 0
#define SEND_SLOT_FORK_TO_CONSU1 0
#define SEND_SLOT_FORK_TO_CONSU2 1

//#define LEN  20 // in words
//#define BUF_LEN 16 // in words

volatile _UNCACHED int started_producer;
volatile _UNCACHED int started_fork;
volatile _UNCACHED int started_consumer1;
volatile _UNCACHED int started_consumer2;
volatile _UNCACHED int finished_producer;
volatile _UNCACHED int finished_fork;
volatile _UNCACHED int finished_consumer1;
volatile _UNCACHED int finished_consumer2;
volatile _UNCACHED int end_flag;
volatile _UNCACHED int result1;
volatile _UNCACHED int result2;
volatile _UNCACHED int time;
volatile _UNCACHED int time1;
volatile _UNCACHED int time2;

void consumer1(void* arg) {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);
  int sum = 0;

  // Get started
  started_consumer1 = 1;

  //for (int i=0; i<LEN; ++i) {
  for (int i=0; i<LEN/BUF_LEN; ++i) {
    for (int j=0; j<BUF_LEN; ++j) {
      while (!s4noc[RX_READY]) {;}
      sum += s4noc[IN_DATA];
    }
  }
  time1 = *timer_ptr - time;
  result1 = sum;
  finished_consumer1 = 1;
  // Join threads
  int ret = 0;
	corethread_exit(&ret);
	return;
}

void consumer2(void* arg) {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);
  int sum = 0;

  // Get started
  started_consumer2 = 1;

  //for (int i=0; i<LEN; ++i) {
  for (int i=0; i<LEN/BUF_LEN; ++i) {
    for (int j=0; j<BUF_LEN; ++j) {
      while (!s4noc[RX_READY]) {;}
      sum += s4noc[IN_DATA];
    }
  }
  time2 = *timer_ptr - time;
  result2 = sum;
  finished_consumer2 = 1;
  // Join threads
  int ret = 0;
	corethread_exit(&ret);
	return;
}

void producer(void* arg) {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);
  int val = 0;
  
  // Get started
  started_producer = 1;

  // Give the other threads some head start to be ready (0.1s)
  *dead_ptr = 8000000;
  val = *dead_ptr;

  // Start timing
  time = *timer_ptr;

  for (int i=0; i<LEN/BUF_LEN; ++i) {
    for (int j=0; j<BUF_LEN; ++j) {
      while (!s4noc[TX_FREE]) {;}
      *dead_ptr = DELAY/2;
      val = *dead_ptr;
      s4noc[SEND_SLOT_PRODU_TO_FORK] = 1;
      *dead_ptr = DELAY/2;
      val = *dead_ptr;
    }
  }
  
  finished_producer = 1;

  while (end_flag==0) {
    while (!s4noc[TX_FREE]) {;}
    *dead_ptr = DELAY/2;
    val = *dead_ptr;
    s4noc[SEND_SLOT_PRODU_TO_FORK] = 0;
    *dead_ptr = DELAY/2;
    val = *dead_ptr;
  }

  // Join threads
  int ret = 0;
	corethread_exit(&ret);
	return;
}

void fork(void* arg) {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);
  volatile _SPM int *spm_base = (volatile _SPM int *) 0x00000000;
  volatile _SPM int *receive_A = &spm_base[0];
  volatile _SPM int *send_A1 = &spm_base[BUF_LEN];
  volatile _SPM int *send_A2 = &spm_base[2*BUF_LEN];
  volatile _SPM int *receive_B = &spm_base[3*BUF_LEN];
  volatile _SPM int *send_B1 = &spm_base[4*BUF_LEN];
  volatile _SPM int *send_B2 = &spm_base[5*BUF_LEN];
  volatile _SPM int *receiving;
  volatile _SPM int *sending1;
  volatile _SPM int *sending2;
  volatile _SPM int *working_input;
  volatile _SPM int *working_output1;
  volatile _SPM int *working_output2;
  volatile _SPM int *tmp_input;
  volatile _SPM int *tmp_output1;
  volatile _SPM int *tmp_output2;
  int val = 0;

  receiving = receive_A;
  sending1 = send_A1;
  sending2 = send_A2;
  working_input = receive_B;
  working_output1 = send_B1;
  working_output2 = send_B2;
  
  // Get started
  started_fork=1;
  
  // Fill up
  for (int j=0; j<BUF_LEN; ++j) {
    while (!s4noc[RX_READY]) {;}
    receive_A[j] = s4noc[IN_DATA];
  }
  // Fill up
  for (int j=0; j<BUF_LEN; ++j) {
    send_A1[j] = receive_A[j]; // Processing
    send_A2[j] = receive_A[j]; // Processing
    while (!s4noc[RX_READY]) {;}
    receive_B[j] = s4noc[IN_DATA];
  }
  // Continue
  for (int i=0; i<LEN/BUF_LEN-2; ++i) {
    for (int j=0; j<BUF_LEN; ++j) {
      working_output1[j] = working_input[j]; // Processing
      working_output2[j] = working_input[j]; // Processing
      while (!s4noc[RX_READY]) {;}
      receiving[j] = s4noc[IN_DATA];
      while (!s4noc[TX_FREE]) {;}
      s4noc[SEND_SLOT_FORK_TO_CONSU1] = sending1[j];
      while (!s4noc[TX_FREE]) {;}
      s4noc[SEND_SLOT_FORK_TO_CONSU2] = sending2[j];
    }
    tmp_output1 = working_output1;
    tmp_output2 = working_output2;
    tmp_input = working_input;
    working_output1 = sending1;
    working_output2 = sending2;
    working_input = receiving;
    sending1 = tmp_output1;
    sending2 = tmp_output2;
    receiving = tmp_input;
  }
  // Flush out
  for (int j=0; j<BUF_LEN; ++j) {
    send_B1[j] = receive_B[j]; // Processing
    send_B2[j] = receive_B[j]; // Processing
    while (!s4noc[TX_FREE]) {;}
    s4noc[SEND_SLOT_FORK_TO_CONSU1] = send_A1[j];
    while (!s4noc[TX_FREE]) {;}
    s4noc[SEND_SLOT_FORK_TO_CONSU2] = send_A2[j];

  }
  // Flush out
  for (int j=0; j<BUF_LEN; ++j) {
    while (!s4noc[TX_FREE]) {;}
    s4noc[SEND_SLOT_FORK_TO_CONSU1] = send_B1[j];
    while (!s4noc[TX_FREE]) {;}
    s4noc[SEND_SLOT_FORK_TO_CONSU2] = send_B2[j];
  }  
    
  finished_fork=1;

  while (end_flag==0) {
    while (!s4noc[TX_FREE]) {;}
    s4noc[SEND_SLOT_FORK_TO_CONSU1] = 0;
    while (!s4noc[TX_FREE]) {;}
    s4noc[SEND_SLOT_FORK_TO_CONSU2] = 0;
  }
  
  // Join threads
  int ret = 0;
	corethread_exit(&ret);
	return;
}

// The main acts as producer
int main() {

  int val = 0;

  end_flag = 0;
  result1 = 0;
  result2 = 0;
  started_producer = 0;
  started_fork = 0;
  started_consumer1 = 0;
  started_consumer2 = 0;
  finished_producer = 0;
  finished_fork = 0;
  finished_consumer1 = 0;
  finished_consumer2 = 0;

  printf("Producer/fork/2-consumers benchmark for the S4NOC paper:\n");
  printf("  Delay: %d\n", DELAY);
  printf("  Number of cores: %d\n", get_cpucnt());
  printf("  Total packets sent: %d\n", LEN);
  printf("  Buffer size: %d\n", BUF_LEN);

  printf("Runnning test:\n");
  corethread_create(CONSUMER1_CORE, &consumer1, NULL);
  while(started_consumer1 == 0) {;}
  printf("  Consumer-1 is ready.\n");
  
  corethread_create(CONSUMER2_CORE, &consumer2, NULL);
  while(started_consumer2 == 0) {;}
  printf("  Consumer-2 is ready.\n");

  corethread_create(FORK_CORE, &fork, NULL);
  while(started_fork == 0) {;}
  printf("  Fork is ready.\n");

  corethread_create(PRODUCER_CORE, &producer, NULL);
  while(started_producer == 0) {;}
  printf("  Producer has started.\n");
  
  while(finished_producer == 0) {;}
  printf("  Producer has finished.\n");
  
  while(finished_fork == 0) {;}
  printf("  Fork has finished.\n");

  while(finished_consumer1 == 0) {;}
  printf("  Consumer-1 has finished.\n");

  while(finished_consumer2 == 0) {;}
  printf("  Consumer-2 has finished.\n");
   
  *dead_ptr = 8000000;
  val = *dead_ptr;
  
  printf("Results Consumer-1: \n");
  printf("  %d valid pakets out of of %d received.\n", result1, LEN); 
  printf("  Reception time of %d cycles -> %g cycles per received packet.\n", time1, 1. * time1/LEN);
  
  printf("Results Consumer-2: \n");
  printf("  %d valid pakets out of of %d received.\n", result2, LEN); 
  printf("  Reception time of %d cycles -> %g cycles per received packet.\n", time2, 1. * time2/LEN);
  
  // Join threads
  int *retval;
  end_flag = 1;
  corethread_join(PRODUCER_CORE, (void **)&retval);
  corethread_join(FORK_CORE, (void **)&retval);
  corethread_join(CONSUMER1_CORE, (void **)&retval);
  corethread_join(CONSUMER2_CORE, (void **)&retval);  

  printf("End of program.\n");
  return val;  
}















