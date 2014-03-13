/*
    The blinking example implemented with interrupts.

    Author: Wolfgang Puffitsch
    Copyright: DTU, BSD License
*/

#include <machine/patmos.h>
#include <machine/exceptions.h>

#include <stdio.h>
#include <stdlib.h>

// definitions for I/O devices
#define LEDS (*((volatile _IODEV unsigned *)0xf0000900))
#define TIMER_USEC_HI (*((volatile _IODEV unsigned *)0xf0000208))
#define TIMER_USEC_LO (*((volatile _IODEV unsigned *)0xf000020c))

// the blinking frequency in microseconds
#define PERIOD 1000000

void intr_handler(void) __attribute__((naked));

// variable keep track of time and generate interrupts without drift
static unsigned long long start_time;

// get the current time in microseconds
static unsigned long long get_usecs(void) {
  unsigned lo = TIMER_USEC_LO;
  unsigned hi = TIMER_USEC_HI;
  return (unsigned long long)hi << 32 | lo;
}

// arm the timer to trigger an interrupt at a specific time
static void arm_usec_timer(unsigned long long time) {
  TIMER_USEC_LO = (unsigned)time;
  TIMER_USEC_HI = (unsigned)(time >> 32);
}

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
  start_time = get_usecs() + PERIOD/2;
  arm_usec_timer(start_time);

  // generate output
  LEDS = 0;
  putc('0', stderr);

  // loop forever
  for(;;);
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
