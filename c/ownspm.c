/*
    This is a test program for the SPM with ownership.
    It is at the moment intended to be executed from the Boot ROM.

    Author: Martin Schoeberl
    Copyright: DTU, BSD License
*/

#include <machine/patmos.h>
#include <machine/spm.h>
#include "include/bootable.h"
#include "include/patio.h"

// We are using the Argo configuration port
#define BASE ((volatile int _IODEV *)0xE0000000)

int main() {

  volatile _IODEV int *led_ptr  = (volatile _IODEV int *) 0xF0090000;
  volatile _IODEV int *uart_ptr = (volatile _IODEV int *) 0xF0080004;

  volatile int _IODEV *spm_ptr = BASE;

  int i, j;

  WRITECHAR('b');
  WRITECHAR(((char) *spm_ptr)+'0');
  *spm_ptr = 'x';
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
