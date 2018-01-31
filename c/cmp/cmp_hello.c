/*
    This is a multicore test program that shall be loaded by a bootloader.

    Author: Martin Schoeberl
    Copyright: DTU, BSD License
*/

#include "include/patio.h"
#include "libcorethread/corethread.h"

#define MAX 20

volatile _UNCACHED char data[MAX];

// The main function for the other threads on the another cores
void work(void *arg) {
  int val = *((int *)arg);

  int id = get_cpuid();
  data[id] = id+'0';


}

int main() {

  unsigned i;

  int id = get_cpuid();
  int cnt = get_cpucnt();

  for (i=0; i<MAX; ++i) data[i] = '#';

  for (i=1; i<cnt; ++i) {
    // Is this ok to have a local variable and then pass a pointer to the
    // thread creating function? Yes, as it is only used as index in the
    // create function.
    corethread_t worker_id = i; // The core number
    int parameter = 1; // dummy
    // Why is the core number passed by reference?
    // No reason, should be an int passed by value.
    corethread_create(&worker_id, &work, (void *) &parameter);  
  }

  data[id] = id+'0';

  for (i=0; i<MAX; ++i) UART_DATA = '.';


  // This is a "normal" multicore example where main is executed only
  // on core 0
  for (i=0; i<MAX; ++i) {
    while ((UART_STATUS & 0x01) == 0);
    UART_DATA = data[i];
  }

  for(;;);
}
