/*
  Producer/fork/n-consumers benchmark for the S4NOC paper.
  Trim the DELAY of the producer until no tokens are lost.

  Author: Martin Schoeberl and Luca Pezzarossa
*/

#include <stdio.h>
#include <machine/spm.h>
#include "../../libcorethread/corethread.h"

#define DELAY 10000
#include "s4noc.h"



#define PRODUCER_CORE 1
#define FORK_CORE 8
#define SEND_SLOT_PRODU_TO_FORK 0

// Amount of consumers
#define CONSUMERS 5
#define CONSUMERS_send 5

volatile _UNCACHED int SEND_SLOT_FORK_TO_CONSU[] = {0, 1, 5, 6, 8, 7};
volatile _UNCACHED int CONSUMER_CORE[] =           {3, 4, 5, 6, 7, 2};
volatile _UNCACHED int CONSUMER_ID[] =             {0, 1, 2, 3, 4, 5};

//#define LEN  20 // in words
//#define BUF_LEN 16 // in words

volatile _UNCACHED int started_producer;
volatile _UNCACHED int started_fork;
volatile _UNCACHED int started_consumer[CONSUMERS];
volatile _UNCACHED int finished_producer;
volatile _UNCACHED int finished_fork;
volatile _UNCACHED int finished_consumer[CONSUMERS];

volatile _UNCACHED int end_flag;
volatile _UNCACHED int result[CONSUMERS];
volatile _UNCACHED int base_time;
volatile _UNCACHED int time[CONSUMERS];

void consumer(void* arg) {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);
  int consumer_id = *((int*)arg);
  int sum = 0;

  // Get started
  started_consumer[consumer_id] = 1;

  //for (int i=0; i<LEN; ++i) {
  for (int i=0; i<LEN/BUF_LEN; ++i) {
    for (int j=0; j<BUF_LEN; ++j) {
      while (!s4noc[RX_READY]) {;}
      sum += s4noc[IN_DATA];
    }
  }
  time[consumer_id] = *timer_ptr - base_time;
  result[consumer_id] = sum;
  finished_consumer[consumer_id] = 1;
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
  base_time = *timer_ptr;

  for (int i=0; i<LEN/BUF_LEN; ++i) {
    for (int j=0; j<BUF_LEN; ++j) {
      while (!s4noc[TX_FREE]) {;}
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
      for (int i=0; i<CONSUMERS_send; ++i) {
      //for (int i=CONSUMERS-1; i>=0; --i) {
        while (!s4noc[TX_FREE]) {;}
        s4noc[SEND_SLOT_FORK_TO_CONSU[i]] = tmp;
      }
    }
  }

  finished_fork=1;

  while (end_flag==0) {
    for (int i=0; i<CONSUMERS_send; ++i) {
      while (!s4noc[TX_FREE]) {;}
      s4noc[SEND_SLOT_FORK_TO_CONSU[i]] = 0;
    }
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
  for (int i=0; i<CONSUMERS; ++i) {
    result[i] = 0;
    started_consumer[i] = 0;
    finished_consumer[i] = 0;
  }
  started_producer = 0;
  started_fork = 0;
  finished_producer = 0;
  finished_fork = 0;

  printf("Producer/fork/n-consumers benchmark for the S4NOC paper:\n");
  printf("  Delay: %d\n", DELAY);
  printf("  Number of cores: %d\n", get_cpucnt());
  printf("  Total packets sent: %d\n", LEN);
  printf("  Buffer size: %d\n", BUF_LEN);

  printf("Runnning test:\n");

  for (int k=0; k<CONSUMERS; ++k) {
    corethread_create(CONSUMER_CORE[k], &consumer, (void*) &CONSUMER_ID[k]  );
    *dead_ptr = 8000;
    val = *dead_ptr;
    while(started_consumer[k] == 0) {;}
    printf("  Consumer-%d is ready.\n", k+1);
  }

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

  for (int i=0; i<CONSUMERS; ++i) {
    while(finished_consumer[i] == 0) {;}
    printf("  Consumer-%d has finished.\n", i+1);
  }

  /*
  for (int i=0; i<CONSUMERS; ++i) {
    printf(" %d %d \n", started_consumer[i], finished_consumer[i] );
  }
  */

  *dead_ptr = 8000000;
  val = *dead_ptr;

  for (int i=0; i<CONSUMERS; ++i) {
    printf("Results Consumer-%d: \n", i+1);
    printf("  %d valid pakets out of of %d received.\n", result[i], LEN);
    printf("  Reception time of %d cycles -> %g cycles per received packet.\n", time[i], 1. * time[i]/LEN);
  }

  // Join threads
  int *retval;
  end_flag = 1;
  corethread_join(PRODUCER_CORE, (void **)&retval);
  corethread_join(FORK_CORE, (void **)&retval);
  for (int i=0; i<CONSUMERS; ++i) {
    corethread_join(CONSUMER_CORE[i], (void **)&retval);
  }

  printf("End of program.\n");
  return val;
}
