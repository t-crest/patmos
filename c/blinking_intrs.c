/*
    The blinking example implemented with interrupts.

    Author: Wolfgang Puffitsch
    Copyright: DTU, BSD License
*/

#include <machine/patmos.h>
#include <machine/exceptions.h>
#include <machine/rtc.h>

#include <stdio.h>
#include <stdlib.h>

// definitions for I/O devices
#define LEDS (*((volatile _IODEV unsigned *)0xf0090000))
#define SLEEP (*((volatile _IODEV unsigned *)0xf0010010))

// the blinking frequency in microseconds
#define PERIOD 1000000

void intr_handler(void) __attribute__((naked));

// variable keep track of time and generate interrupts without drift
static unsigned long long start_time;

int main(void) {
  // register exception handler
  exc_register(17, &intr_handler);

  // unmask interrupts
  intr_unmask_all();
  // clear pending flags
  intr_clear_all_pending();
  // enable interrupts
  intr_enable();

  // arm timer
  start_time = get_cpu_usecs() + PERIOD/2;
  arm_usec_timer(start_time);

  // generate output
  LEDS = 0;
  putc('0', stderr);

  // loop forever
  for(;;) {
    SLEEP = 0;
  }
}

// interrupt handler
void intr_handler(void) {
  exc_prologue();

  // arm timer for next interrupt
  start_time += PERIOD/2;
  arm_usec_timer(start_time);

  // generate output
  LEDS ^= 1;
  putc('0' + (LEDS & 1), stderr);

  exc_epilogue();
}
