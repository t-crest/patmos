/*
    This is multicore verson of an embedded Hello World program: a blinking LED.

    Author: Martin Schoeberl
    Copyright: DTU, BSD License
*/

#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>

#include "libnoc/noc.h"
#include "libcorethread/corethread.h"

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

int main() {

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
