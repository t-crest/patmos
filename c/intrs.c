/*
    An example application for exception and interrupt handling.

    Author: Wolfgang Puffitsch
    Copyright: DTU, BSD License
*/

#include <machine/patmos.h>
#include <machine/exceptions.h>

#include <stdio.h>
#include <stdlib.h>

#define LEDS (*((volatile _IODEV unsigned *)PATMOS_IO_LED))

void fault_handler(void);
void trap_handler(void) __attribute__((naked));
void intr_handler(void) __attribute__((naked));

#define N 10000

int main(void) {
  // register exception handlers
  for (unsigned i = 0; i < 32; i++) {
	exc_register(i, &fault_handler);
  }
  exc_register(8, &trap_handler);
  exc_register(18, &intr_handler);
  exc_register(19, &intr_handler);
  exc_register(20, &intr_handler);
  exc_register(21, &intr_handler);

  // unmask interrupts
  intr_unmask_all();
  // clear pending flags
  intr_clear_all_pending();
  // enable interrupts
  intr_enable();

  // go to user mode
  EXC_STATUS &= ~0x2;

  // a that prints "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_" N times and does some self-checking
  volatile unsigned starts = 0;
  volatile unsigned ends = 0;
  volatile unsigned sent = 0;

  for (unsigned k = 0; k < N; k++) {
    starts++;
    for (unsigned i = 0; i < 32; i++) {
      putchar('@'+i);
      sent+=i;
    }
    putchar('\n');
    ends++;

    if (sent != 496*(k+1) || starts != ends) {
      LEDS = 0x55;
      abort();
    }
  }

  // disabling interrupts again only works in privileged mode
  /* intr_disable(); */

  // call exception vector number 8
  trap(8);

  // trigger illegal operation fault
  asm volatile(".word 0xffffffff"); // illegal operation
  // trigger illegal memory access fault, never reached
  (*((volatile _IODEV unsigned *)0xffffffff)) = 0;

  return 0;
}

// a basic handler for faults
void fault_handler(void) {
  unsigned source = exc_get_source();
  LEDS = source;

  const char *msg = "FAULT";
  switch(source) {
  case 0: msg = "Illegal operation"; break;
  case 1: msg = "Illegal memory access"; break;
  }
  puts(msg);

  // cannot recover from a fault
  abort();
}

// a basic trap
void trap_handler(void) {
  exc_prologue();

  puts("TRAP");

  exc_epilogue();
}

// a basic interrupt handler
void intr_handler(void) {
  exc_prologue();

  LEDS += exc_get_source() & 0xf;

  exc_epilogue();
}
