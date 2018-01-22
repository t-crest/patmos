/*
    This is a multicore test program that shall be loaded by a bootloader.

    Author: Martin Schoeberl
    Copyright: DTU, BSD License
*/

#include "include/patio.h"
#include "libcorethread/corethread.h"

#define MAX 20

volatile _UNCACHED char data[MAX];

// The main function for the other thread on the another core
void work(void* arg) {
  int val = *((int*)arg);

  int id = get_cpuid();
  data[id] = id+'0';


}

int main() {

  unsigned i;

  int id = get_cpuid();
  int cnt = get_cpucnt();

  for (i=0; i<MAX; ++i) data[i] = '#';

  for (i=1; i<cnt; ++i) {
    corethread_t worker_id = i; // The core number
    int parameter = 1000;
    corethread_create( &worker_id, &work, (void *) &parameter);  
  }

  data[id] = id+'0';

  // Only core 0 is connected to the serial port
  if (id == 0) {
    for (i=0; i<MAX; ++i) {
      while ((UART_STATUS & 0x01) == 0);
      UART_DATA = data[i];
    }
  }

  for(;;);
}
