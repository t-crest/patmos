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
volatile _UNCACHED int sync_valid;
volatile _UNCACHED int sync_time;

void consumer(void* arg) {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);

  _SPM int * sum = (_SPM int *) D_SPM_BASE;
  _SPM int * i = (_SPM int *) D_SPM_BASE + 4;
  _SPM int * j = (_SPM int *) D_SPM_BASE + 8;

  //register int sum = 0;
  *sum = 0;

  // Get started
  started_consumer = 1;

  //for (int i=0; i<LEN; ++i) {
  for (*i=0; *i<LEN/BUF_LEN; ++(*i)) {
    for (*j=0; *j<BUF_LEN; ++(*j)) {
      while (!s4noc[RX_READY]) {;}
      *sum += s4noc[IN_DATA];
    }
  }
  time = *timer_ptr;
  time1 = time - time1;
  time2 = time - time2;
  result = *sum;
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

  // Synchronizing as much as possible with producer 1
  while(sync_valid == 0) {;}
  while(*timer_ptr < sync_time) {;}

  // Start timing
  time1 = *timer_ptr;

  for (int i=0; i<LEN/BUF_LEN; ++i) {
    for (int j=0; j<BUF_LEN; ++j) {
      while (!s4noc[TX_FREE]) {;}
      *dead_ptr = DELAY;
      val = *dead_ptr;
      s4noc[SEND_SLOT_PRODU1_TO_JOIN] = 1;
    }
  }

  finished_producer1 = 1;

  while (end_flag==0) {
    while (!s4noc[TX_FREE]) {;}
    *dead_ptr = DELAY;
    val = *dead_ptr;
    s4noc[SEND_SLOT_PRODU1_TO_JOIN] = 0;
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
  while(sync_valid == 0) {;}
  while(*timer_ptr < sync_time) {;}

  // Start timing
  time2 = *timer_ptr;

  for (int i=0; i<LEN/BUF_LEN; ++i) {
    for (int j=0; j<BUF_LEN; ++j) {
      while (!s4noc[TX_FREE]) {;}
      *dead_ptr = DELAY;
      val = *dead_ptr;
      s4noc[SEND_SLOT_PRODU2_TO_JOIN] = 2;
    }
  }

  finished_producer2 = 1;

  while (end_flag==0) {
    while (!s4noc[TX_FREE]) {;}
    *dead_ptr = DELAY;
    val = *dead_ptr;
    s4noc[SEND_SLOT_PRODU2_TO_JOIN] = 0;
  }

  // Join threads
  int ret = 0;
	corethread_exit(&ret);
	return;
}


void join(void* arg) {

  volatile _SPM int *s4noc = (volatile _SPM int *) (S4NOC_ADDRESS);
  int val = 0;

  _SPM int * tmp = (_SPM int *) D_SPM_BASE;
  _SPM int * tmp1 = (_SPM int *) D_SPM_BASE + 4;
  _SPM int * tmp2 = (_SPM int *) D_SPM_BASE + 8;
  _SPM int * i = (_SPM int *) D_SPM_BASE + 12;
  _SPM int * j = (_SPM int *) D_SPM_BASE + 16;

  // Get started
  started_join=1;

  for (*i=0; *i<LEN/BUF_LEN; ++(*i)) {
    for (*j=0; *j<BUF_LEN; ++(*j)) {
      *tmp = 0;
      while (!s4noc[RX_READY]) {;}
      *tmp1 = s4noc[IN_DATA];
      while (!s4noc[RX_READY]) {;}
      *tmp2 = s4noc[IN_DATA];
      if ((*tmp2==2 && *tmp1==1) || (*tmp2==1 && *tmp1==2)) {
        *tmp = 1;
      }
      while (!s4noc[TX_FREE]) {;}
      s4noc[SEND_SLOT_JOIN_TO_CONSU] = *tmp;
    }
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
  started_join = 0;
  started_consumer = 0;
  finished_producer1 = 0;
  finished_producer2 = 0;
  finished_join = 0;
  finished_consumer = 0;
  sync_valid = 0;
  sync_time = 0;

  printf("Synchronized 2-producers/join/consumer benchmark for the S4NOC paper:\n");
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
  printf("  Producer-2 has started.\n  [...]\n");

  sync_time = *timer_ptr + 80000000;
  *dead_ptr = 800000;
  val = *dead_ptr;
  sync_valid = 1;

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
