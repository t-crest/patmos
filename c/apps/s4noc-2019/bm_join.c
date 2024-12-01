/*
  2-producers/join/consumer benchmark for the S4NOC paper.
  Trim the DELAY of the producer until no tokens are lost.

  Author: Martin Schoeberl and Luca Pezzarossa
*/

#include <stdio.h>
#include <machine/spm.h>
#include "../../libcorethread/corethread.h"

#include "s4noc.h"

#define PRODUCER1_CORE 1
#define PRODUCER2_CORE 4
#define JOIN_CORE 8
#define CONSUMER_CORE 3
#define SEND_SLOT_PRODU1_TO_JOIN 0
#define SEND_SLOT_PRODU2_TO_JOIN 2
#define SEND_SLOT_JOIN_TO_CONSU 0

//#define LEN  20 // in words
//#define BUF_LEN 16 // in words

volatile _UNCACHED int started_producer1;
volatile _UNCACHED int started_producer2;
volatile _UNCACHED int sync_producer1;
volatile _UNCACHED int started_join;
volatile _UNCACHED int started_consumer;
volatile _UNCACHED int finished_producer1;
volatile _UNCACHED int finished_producer2;
volatile _UNCACHED int finished_join;
volatile _UNCACHED int finished_consumer;
volatile _UNCACHED int end_flag;
volatile _UNCACHED int result;
volatile _UNCACHED int time;
volatile _UNCACHED int time1;
volatile _UNCACHED int time2;

void consumer(void* arg) {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);
  int sum = 0;

  // Get started
  started_consumer = 1;

  //for (int i=0; i<LEN; ++i) {
  for (int i=0; i<LEN/BUF_LEN; ++i) {
    for (int j=0; j<BUF_LEN; ++j) {
      while (!s4noc[RX_READY]) {;}
      sum += s4noc[IN_DATA];
    }
  }
  time = *timer_ptr;
  time1 = time - time1;
  time2 = time - time2;
  result = sum;
  finished_consumer = 1;
  // Join threads
  int ret = 0;
	corethread_exit(&ret);
	return;
}


void producer1(void* arg) {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);
  int val = 0;
  
  // Get started
  started_producer1 = 1;

  // Give the other threads some head start to be ready (0.1s)
  *dead_ptr = 8000000;
  val = *dead_ptr;

  // Get started
  sync_producer1 = 1;
  
  // Start timing
  time1 = *timer_ptr;

  for (int i=0; i<LEN/BUF_LEN; ++i) {
    for (int j=0; j<BUF_LEN; ++j) {
      while (!s4noc[TX_FREE]) {;}
      *dead_ptr = DELAY/2;
      val = *dead_ptr;
      s4noc[SEND_SLOT_PRODU1_TO_JOIN] = 1;  
      *dead_ptr = DELAY/2;
      val = *dead_ptr;
    }
  }
  
  finished_producer1 = 1;

  while (end_flag==0) {
    while (!s4noc[TX_FREE]) {;}
    *dead_ptr = DELAY/2;
    val = *dead_ptr;
    s4noc[SEND_SLOT_PRODU1_TO_JOIN] = 0;
    *dead_ptr = DELAY/2;
    val = *dead_ptr;
  }

  // Join threads
  int ret = 0;
	corethread_exit(&ret);
	return;
}

void producer2(void* arg) {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);
  int val = 0;
  
  // Get started
  started_producer2 = 1;

  // Synchronizing as much as possible with producer 1
  while(sync_producer1 == 0) {;}

  // Start timing
  time2 = *timer_ptr;

  for (int i=0; i<LEN/BUF_LEN; ++i) {
    for (int j=0; j<BUF_LEN; ++j) {
      while (!s4noc[TX_FREE]) {;}
      *dead_ptr = DELAY/2;
      val = *dead_ptr;
      s4noc[SEND_SLOT_PRODU2_TO_JOIN] = 2;  
      *dead_ptr = DELAY/2;
      val = *dead_ptr;
    }
  }
  
  finished_producer2 = 1;

  while (end_flag==0) {
    while (!s4noc[TX_FREE]) {;}
    *dead_ptr = DELAY/2;
    val = *dead_ptr;
    s4noc[SEND_SLOT_PRODU2_TO_JOIN] = 0;
    *dead_ptr = DELAY/2;
    val = *dead_ptr;
  }

  // Join threads
  int ret = 0;
	corethread_exit(&ret);
	return;
}

void join(void* arg) {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);
  volatile _SPM int *spm_base = (volatile _SPM int *) 0x00000000;
  volatile _SPM int *receive_A1 = &spm_base[0];
  volatile _SPM int *receive_A2 = &spm_base[BUF_LEN];
  volatile _SPM int *send_A = &spm_base[2*BUF_LEN];
  volatile _SPM int *receive_B1 = &spm_base[3*BUF_LEN];
  volatile _SPM int *receive_B2 = &spm_base[4*BUF_LEN];
  volatile _SPM int *send_B = &spm_base[5*BUF_LEN];
  volatile _SPM int *receiving1;
  volatile _SPM int *receiving2;
  volatile _SPM int *sending;
  volatile _SPM int *working_input1;
  volatile _SPM int *working_input2;  
  volatile _SPM int *working_output;
  volatile _SPM int *tmp_input1;
  volatile _SPM int *tmp_input2;  
  volatile _SPM int *tmp_output;
  int val = 0;

  receiving1 = receive_A1;
  receiving2 = receive_A2;
  sending = send_A;
  working_input1 = receive_B1;
  working_input2 = receive_B2;
  working_output = send_B;
  
  // Get started
  started_join=1;
  
  // Fill up
  for (int j=0; j<BUF_LEN; ++j) {
    while (!s4noc[RX_READY]) {;}
    receive_A1[j] = s4noc[IN_DATA];
    while (!s4noc[RX_READY]) {;}
    receive_A2[j] = s4noc[IN_DATA];
  }
  // Fill up
  for (int j=0; j<BUF_LEN; ++j) {
    if ((receive_A1[j]==2 && receive_A2[j]==1) || (receive_A1[j]==1 && receive_A2[j]==2)) { // Processing
      send_A[j] = 1;
    } else {
      send_A[j] = 0;
    } 
    while (!s4noc[RX_READY]) {;}
    receive_B1[j] = s4noc[IN_DATA];
    while (!s4noc[RX_READY]) {;}
    receive_B2[j] = s4noc[IN_DATA];

  }
  // Continue
  for (int i=0; i<LEN/BUF_LEN-2; ++i) {
    for (int j=0; j<BUF_LEN; ++j) {
      if ((working_input1[j]==2 && working_input2[j]==1) || (working_input1[j]==1 && working_input2[j]==2)) { // Processing
      working_output[j] = 1;
      } else {
      working_output[j] = 0;
      }  
      while (!s4noc[RX_READY]) {;}
      receiving1[j] = s4noc[IN_DATA];
      while (!s4noc[RX_READY]) {;}
      receiving2[j] = s4noc[IN_DATA];
      while (!s4noc[TX_FREE]) {;}
      s4noc[SEND_SLOT_JOIN_TO_CONSU] = sending[j];
    }
    tmp_output = working_output;
    tmp_input1 = working_input1;
    tmp_input2 = working_input2;
    working_output = sending;
    working_input1 = receiving1;
    working_input2 = receiving2;
    sending = tmp_output;
    receiving1 = tmp_input1;
    receiving2 = tmp_input2;
  }
  // Flush out
  for (int j=0; j<BUF_LEN; ++j) {
    if ((receive_B1[j]==2 && receive_B2[j]==1) || (receive_B1[j]==1 && receive_B2[j]==2)) { // Processing
      send_B[j] = 1;
    } else {
      send_B[j] = 0;
    } 
    while (!s4noc[TX_FREE]) {;}
    s4noc[SEND_SLOT_JOIN_TO_CONSU] = send_A[j];
  }
  // Flush out
  for (int j=0; j<BUF_LEN; ++j) {
    while (!s4noc[TX_FREE]) {;}
    s4noc[SEND_SLOT_JOIN_TO_CONSU] = send_B[j];
  }  
    
  finished_join=1;

  while (end_flag==0) {
    while (!s4noc[TX_FREE]) {;}
    s4noc[SEND_SLOT_JOIN_TO_CONSU] = 0;
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
  result = 0;
  started_producer1 = 0;
  started_producer2 = 0;
  sync_producer1 = 0;
  started_join = 0;
  started_consumer = 0;
  finished_producer1 = 0;
  finished_producer2 = 0;
  finished_join = 0;
  finished_consumer = 0;

  printf("2-producers/join/consumer benchmark for the S4NOC paper:\n");
  printf("  Delay: %d\n", DELAY);
  printf("  Number of cores: %d\n", get_cpucnt());
  printf("  Total packets sent: %d\n", LEN);
  printf("  Buffer size: %d\n", BUF_LEN);

  printf("Runnning test:\n");
  corethread_create(CONSUMER_CORE, &consumer, NULL);
  while(started_consumer == 0) {;}
  printf("  Consumer is ready.\n");

  corethread_create(JOIN_CORE, &join, NULL);
  while(started_join == 0) {;}
  printf("  Join is ready.\n");

  corethread_create(PRODUCER2_CORE, &producer2, NULL);
  corethread_create(PRODUCER1_CORE, &producer1, NULL);
  while(started_producer1 == 0) {;}
  printf("  Producer-1 has started.\n");  
  
  while(started_producer2 == 0) {;}
  printf("  Producer-2 has started.\n");
   
  while(finished_producer1 == 0) {;}
  printf("  Producer-1 has finished.\n");
  
  while(finished_producer2 == 0) {;}
  printf("  Producer-2 has finished.\n");
  
  while(finished_join == 0) {;}
  printf("  Join has finished.\n");

  while(finished_consumer == 0) {;}
  printf("  Consumer has finished.\n");
    
  *dead_ptr = 8000000;
  val = *dead_ptr;
  
  printf("Results: \n");
  printf("  %d valid pakets out of of %d received.\n", result, LEN); 
  printf("  Reception time of %d cycles -> %g cycles per received packet (from Producer-1).\n", time1, 1. * time1/LEN);
  printf("  Reception time of %d cycles -> %g cycles per received packet (from Producer-2).\n", time2, 1. * time2/LEN);
  
  // Join threads
  int *retval;
  end_flag = 1;
  corethread_join(PRODUCER1_CORE, (void **)&retval);
  corethread_join(PRODUCER2_CORE, (void **)&retval);
  corethread_join(JOIN_CORE, (void **)&retval);
  corethread_join(CONSUMER_CORE, (void **)&retval);

  printf("End of program.\n");
  return val;  
}

