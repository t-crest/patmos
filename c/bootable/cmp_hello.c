/*
    This is amulticore test program that can be compiled as bootable
    into the ROM.

    Author: Martin Schoeberl
    Copyright: DTU, BSD License
*/

#include "include/bootable.h"

#define MAX 20

volatile _UNCACHED char data[MAX];

// main() is executed by all cores in parallel
int main() {

  unsigned i;

  int id = get_cpuid();
  int cnt = get_cpucnt();

  for (i=0; i<MAX; ++i) data[i] = '#';

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
