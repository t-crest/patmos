/*
  Producer/fork/2-consumers benchmark for the S4NOC paper.
  Trim the DELAY of the producer until no tokens are lost.

  Author: Martin Schoeberl and Luca Pezzarossa
*/

#include <stdio.h>
#include <machine/spm.h>
#include "../../libcorethread/corethread.h"

#include "s4noc.h"

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
      //while (!s4noc[TX_FREE]) {;}
      *dead_ptr = DELAY;
      val = *dead_ptr;
      s4noc[SEND_SLOT_PRODU_TO_FORK] = 1;
    }
  }

  finished_producer = 1;

  while (end_flag==0) {
    while (!s4noc[TX_FREE]) {;}
    *dead_ptr = DELAY;
    val = *dead_ptr;
    s4noc[SEND_SLOT_PRODU_TO_FORK] = 0;
  }

  // Join threads
  int ret = 0;
	corethread_exit(&ret);
	return;
}

void fork(void* arg) {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);
  register int tmp;
  int val = 0;

  // Get started
  started_fork=1;

  for (int i=0; i<LEN/BUF_LEN; ++i) {
    for (int j=0; j<BUF_LEN; ++j) {
      while (!s4noc[RX_READY]) {;}
      tmp = s4noc[IN_DATA];
      while (!s4noc[TX_FREE]) {;}
      s4noc[SEND_SLOT_FORK_TO_CONSU1] = tmp;
      while (!s4noc[TX_FREE]) {;}
      s4noc[SEND_SLOT_FORK_TO_CONSU2] = tmp;
    }
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

  printf("Running test:\n");
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
  printf("  Producer has started.\n  [...]\n");

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
  printf("  %d valid packets out of of %d received.\n", result1, LEN);
  printf("  Reception time of %d cycles -> %g cycles per received packet.\n", time1, 1. * time1/LEN);

  printf("Results Consumer-2: \n");
  printf("  %d valid packets out of of %d received.\n", result2, LEN);
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
