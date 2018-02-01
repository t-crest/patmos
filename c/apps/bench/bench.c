/*
    Benchmarking the 9-core version of T-CREST.

    Author: Martin Schoeberl
    Copyright: DTU, BSD License
*/

#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>

#include "../../libcorethread/corethread.h"
#include "../../libmp/mp.h"

#define NUM_BUF 2
#define BUF_SIZE 400

// Whatever this contant means, it is needed
const int NOC_MASTER = 0;

// Shared data in main memory for the return value
volatile _UNCACHED static int field;
volatile _UNCACHED static int end_time;

#define CNT 512

unsigned _SPM *data_spm = SPM_BASE;


void do_delay_times() {
  // delay times
  for (int i=0; i<CNT; ++i) {
    data_spm[i] = rand() & 0xff;
  }
}

void bench_mem() {

  // Pointer to the deadline device
  volatile _IODEV int *dead_ptr = (volatile _IODEV int *) PATMOS_IO_DEADLINE;
  // Measure execution time with the clock cycle timer
  volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);


  // use function
  int start, end;
  int val;
  int acc, add;

  acc = 0;

  start = *timer_ptr;
  val = *timer_ptr-start;
  printf("Empty measurement %d\n", val);

  start = *timer_ptr;
  field = 0;
  val = *timer_ptr-start;
  --val;
  // result is 23, 1 for the measurement, 1 for a constant load
  // between the two measurements. So a write is 21 clock cycles.
  printf("Single write %d\n", val);

  start = *timer_ptr;
  add = field;
  val = *timer_ptr-start;
  acc += add;
  --val;
  // result is 23, 1 for the measurement
  // between the two measurements. So a read is 22 clock cycles.
  printf("Single read %d\n", val);

  printf("Access in a loop:\n");
  for (int i=0; i<CNT; ++i) {
    // field = 1;
    add = field;
    acc += add;
    start = *timer_ptr;
    // field = 0;
    add = field;
    val = *timer_ptr-start;
    // now 22 clock cycles
    acc += add;
    --val;
    printf("%d ", val);
  }
  printf("\n");

  int min = 100;
  int max = 0;

  do_delay_times();
  printf("Access with a random delay\n");
  for (int i=0; i<CNT; ++i) {
    // field = 1;
    add = field;
    acc += add;
    *dead_ptr = data_spm[i];
    val = *dead_ptr; // delay by a random value
    start = *timer_ptr;
    // field = 0;
    add = field;
    val = *timer_ptr-start;
    acc += add;
    --val;
    printf("%d ", val);
    if (min>val) min = val;
    if (max<val) max = val;
  }
  printf("\n");

  printf("Min: %d max: %d\n", min, max);
  printf("Acc result %d\n", acc);
}

// The main function for the other thread on the another core
void work(void* arg) {

  // Measure execution time with the clock cycle timer
  volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);

  int val, data;

  qpd_t *channel = mp_create_qport(1, SINK, BUF_SIZE, NUM_BUF);
  mp_init_ports();
  for (;;) {
    mp_recv(channel, 0);
    val = *timer_ptr;
    data = *(volatile int _SPM *) channel->read_buf;
    mp_ack(channel, 0);

    // Return time stamp and the change value in the shared variable
    end_time = val;
    field = data + 1;
  }
}

void bench_noc() {

  // Pointer to the deadline device
  volatile _IODEV int *dead_ptr = (volatile _IODEV int *) PATMOS_IO_DEADLINE;
  // Measure execution time with the clock cycle timer
  volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);


  printf("Hello NoC\n");
  printf("We use %d bytes buffers\n", BUF_SIZE);
  int core_id = 1; // The core number
  corethread_create(core_id, &work, NULL); 

  int start, val;

  int data = 42;
  // create a channel
  qpd_t *channel = mp_create_qport(1, SOURCE, BUF_SIZE, NUM_BUF);
  // init
  mp_init_ports();
  start = *timer_ptr;
  // write data into the send buffer
  *(volatile int _SPM *) channel->write_buf = data;
  start = *timer_ptr;
  // send the buffer
  mp_send(channel, 0);
  printf("Data sent\n");
  printf("Returned data is: %d\n", field);
  printf("Took %d cycles\n", end_time - start - 1);

  int min = 999999;
  int max = 0;

  printf("NoC in a loop:\n");
  for (int i=0; i<CNT; ++i) {
    start = *timer_ptr;
    *(volatile int _SPM *) channel->write_buf = i;
    start = *timer_ptr;
    mp_send(channel, 0);
    *dead_ptr = 10000; // some delay to see the result
    val = *dead_ptr;
    val = end_time - start - 1;
//    printf("%d ", val);
    if (min>val) min = val;
    if (max<val) max = val;
  }
  printf("\n");
  printf("Min: %d max: %d\n", min, max);

  min = 999999;
  max = 0;
  do_delay_times();

  printf("NoC in a loop with random delay:\n");
  for (int i=0; i<CNT; ++i) {
    start = *timer_ptr;
    *(volatile int _SPM *) channel->write_buf = i;
    *dead_ptr = data_spm[i];
    val = *dead_ptr; // delay by a random value
    start = *timer_ptr;
    mp_send(channel, 0);
    *dead_ptr = 3000; // some delay to see the result
    val = *dead_ptr;
    val = end_time - start;
    // printf("%d ", val);
    if (min>val) min = val;
    if (max<val) max = val;
  }
  printf("\n");
  printf("Min: %d max: %d\n", min, max);

  // not really as the worker runs forever
  int* res;
  corethread_join( core_id, (void *) &res );
}

int main() {

  bench_mem();
  bench_noc();

  return 0;
}
