/*
    This is a test program for SPM with ownership.
    It is at the moment intended to be executed from the Boot ROM.

    Author: Martin Schoeberl
    Copyright: DTU, BSD License
*/

#include <machine/patmos.h>
#include <machine/spm.h>
#include "include/bootable.h"
#include "include/patio.h"

// We using the Argo configuration port
#define BASE ((volatile int _IODEV *)0xE0000000)

int main() {

  // we use _SPM for the IO devices, and _IODEV for the shared SPM. Strange ;-)
  volatile _SPM int *led_ptr  = (volatile _SPM int *) 0xF0090000;
  volatile _SPM int *uart_ptr = (volatile _SPM int *) 0xF0080004;

  volatile int _IODEV *spm_ptr = BASE;

  int i, j;

  WRITECHAR('b');
  WRITECHAR((char) *spm_ptr);
  WRITE("STRING", 6);
  for (int i=0; i<10; ++i) {
    *uart_ptr = '1';
    for (j=2000; j!=0; --j)
      ;
//        *led_ptr = 1;


    *uart_ptr = '0';
    for (j=2000; j!=0; --j)
      ;
//        *led_ptr = 0;

  }
}
