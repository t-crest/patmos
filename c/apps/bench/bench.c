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

// Blink the individual LED of a core
void blink(int period) {

  volatile _IODEV int *led_ptr = (volatile _IODEV int *) PATMOS_IO_LED;
  volatile _IODEV int *us_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+12);

  int time = period*1000/2;
  int next;

  for (;;) {
    next = *us_ptr + time;
    while (*us_ptr-next < 0) *led_ptr = 1;
    next = *us_ptr + time;
    while (*us_ptr-next < 0) *led_ptr = 0;
  }
}

// The main function for the other thread on the another core
void work(void* arg) {
  int val = *((int*)arg);

  blink(val);
}


#define CNT 512

volatile _UNCACHED static int field;
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

int main() {

  bench_mem();

  printf("Hello CMP\n");
  corethread_t worker_id = 1; // The core number
  int parameter = 1000;
  corethread_create( &worker_id, &work, (void *) &parameter);  

  blink(2000);

  // the folowing is not executed in this example
  int* res;
  corethread_join( worker_id, (void *) &res );

  return 0;
}
