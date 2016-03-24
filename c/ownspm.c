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
#define COM_CONF ((volatile int _IODEV *)0xE0000000)
#define COM_SPM ((volatile int _IODEV *)0xE8000000)

#define DSPM ((volatile int _SPM *)0x00000000)

int main() {

  volatile _IODEV int *led_ptr  = (volatile _IODEV int *) 0xF0090000;
  volatile _IODEV int *uart_ptr = (volatile _IODEV int *) 0xF0080004;

  volatile int _IODEV *spm_ptr = COM_CONF;
  // Using the Argo SPM port to get the core id
  volatile int _IODEV *id_ptr = COM_SPM;

  int i, j;
  char ch;

  WRITECHAR('b');
  WRITECHAR(((char) *spm_ptr)+'0');
  *spm_ptr = 'x';
  WRITECHAR((char) *spm_ptr);

  int id = *id_ptr;
  WRITECHAR(id+'0');
  WRITECHAR('-');

  for (int i=0; i<8; ++i) {
    WRITECHAR(((char) *(spm_ptr+i))+'0');
  }
  *(spm_ptr+0) = 'A';
  *(spm_ptr+1) = 'B';
  *(spm_ptr+2) = 'C';
  *(spm_ptr+3) = 'D';
  for (int i=0; i<8; ++i) {
    WRITECHAR(((char) *(spm_ptr+i))+'0');
  }

  *(spm_ptr) = id;


  // WRITE("STRING", 6);

  for (int i=0; i<8; ++i) {
    WRITECHAR(((char) *(spm_ptr+i))+'0');
  }

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
