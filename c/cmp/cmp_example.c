/*
    This is multicore version of an embedded Hello World program:
    two blinking LEDs executing on two cores.

    Author: Martin Schoeberl
    Copyright: DTU, BSD License
*/

#include <stdio.h>
#include <machine/patmos.h>

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
  int core_id = 1; // The core number
  static int parameter = 1000;
  corethread_create(core_id, &work, (void *) &parameter);  

  blink(2000);

  // the folowing is not executed in this example
  int* res;
  corethread_join( core_id, (void *) &res );

  return 0;
}
