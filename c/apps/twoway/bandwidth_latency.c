/*
    Test program for measuring latency as well as maximum write bandwidth and
    random read bandwidth
*/

#include <machine/patmos.h>
#include <machine/spm.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../libcorethread/corethread.h"

#define BLOCKWIDTH 7

/* Print switch to quickly disable print statements for running in emulator*/
#define DO_PRINTS

#ifdef DO_PRINTS
#define PRINTF(str, ...) printf((str), ##__VA_ARGS__)
#else
#define PRINTF(str, ...)
#endif

// Pointer to the deadline device
volatile _IODEV int *dead_ptr = (volatile _IODEV int *)PATMOS_IO_DEADLINE;
// Measure execution time with the clock cycle timer
volatile _IODEV int *timer_ptr = (volatile _IODEV int *)(PATMOS_IO_TIMER + 4);

volatile _SPM int *mem_ptr = (volatile _IODEV int *)(0xE8000000);

void latency_from_master_lean() {
  int start, end, val;
  for (int i = 0; i < get_cpucnt(); i++) {
    volatile _SPM int *ptr =
        (volatile _IODEV int *)(mem_ptr + (i << BLOCKWIDTH));
    *(ptr) = 10;
  }

  for (int i = 0; i < get_cpucnt(); i++) {
    volatile _SPM int *ptr =
        (volatile _IODEV int *)(mem_ptr + (i << BLOCKWIDTH));
    val = *ptr;
  }
}

void randDelay() {
  for (volatile int i = rand() % 50; i != 0; i--) {
  }
}

void latency_from_master() {
  int start, end, val;

  // 1 for measurement, 1 for constant load cycle between the measurement
  const int const_measurement = 2;

  PRINTF("\n\n---- LATENCY TEST ----\n");

  // External read/write
  const int CNT = 1 << 10;
  const int TRYS = 1;
  PRINTF("Measuring min/max values over %d attempts\n", CNT);
  int min = 100;
  int max = 0;
  PRINTF("\n");
  for (int t = 1; t <= TRYS; t++) {
    PRINTF("-- Try #%d --\n", t);
    for (int i = 0; i < get_cpucnt(); ++i) {
      volatile _SPM int *ptr =
          (volatile _IODEV int *)(mem_ptr + (i << BLOCKWIDTH));
      PRINTF("Core 0 -> Core %d:\n", i);
      // External write
      for (int k = 0; k < CNT; k++) {
        randDelay();
        start = *timer_ptr;
        *(ptr) = 10;
        val = *timer_ptr - start;
        if (min > val)
          min = val;
        if (max < val)
          max = val;
      }
      PRINTF("Write:  Min: %d max: %d\n", min - 1, max - 1);
      min = 100;
      max = 0;

      // External read
      for (int k = 0; k < CNT; k++) {
        randDelay();
        start = *timer_ptr;
        val = *(ptr);
        val = *timer_ptr - start;
        if (min > val)
          min = val;
        if (max < val)
          max = val;
      }
      PRINTF("Read:   Min: %d max: %d\n", min - 1, max - 1);
      PRINTF("\n");
      min = 100;
      max = 0;
    }
  }
}

  // -----------------------------------------------------------------
  // --------- Bandwidth tests ---------------------------------------
  // -----------------------------------------------------------------

#define write                                                                  \
  "swl  [%2] = %5;"                                                            \
  "swl	[%3] = %5;"                                                             \
  "swl	[%4] = %5;"

#define read "lwl	$r11 = [%2];"

#define write2 write write
#define write4 write2 write2
#define write8 write4 write4
#define write16 write8 write8
#define write32 write16 write16
#define write64 write32 write32
#define write128 write64 write64
#define write256 write128 write128

#define read2 read read
#define read4 read2 read2
#define read8 read4 read4
#define read16 read8 read8
#define read32 read16 read16
#define read64 read32 read32
#define read128 read64 read64
#define read256 read128 read128
#define read512 read256 read256

void write_bandwidth_2x2(void *arg) {
  int sum = 0;
  volatile int start, stop, val;
  start = *timer_ptr; // Use timer ptr so we can refer to it in inline assembly
                      // by name

  // Set targets according to schedule
  int target1, target2, target3;
  switch (get_cpuid()) {
  case 0: {
    target1 = 3;
    target2 = 2;
    target3 = 1;
    break;
  }
  case 1: {
    target1 = 3;
    target2 = 0;
    target3 = 2;
    break;
  }
  case 2: {
    target1 = 1;
    target2 = 0;
    target3 = 3;
    break;
  }
  case 3: {
    target1 = 0;
    target2 = 1;
    target3 = 2;
    break;
  }
  }

  volatile _SPM int *ptr1 =
      (volatile _IODEV int *)(mem_ptr + (target1 << BLOCKWIDTH));
  volatile _SPM int *ptr2 =
      (volatile _IODEV int *)(mem_ptr + (target2 << BLOCKWIDTH));
  volatile _SPM int *ptr3 =
      (volatile _IODEV int *)(mem_ptr + (target3 << BLOCKWIDTH));

  // 256 times, write a word to each of the other 3 cores in the 2x2 network
  const int n = 256 * 3;
  int v = 10; // Value to be written

  // Timing is done inline to assure that the timer ptr is not spilled to stack
  asm("li	$r10 = timer_ptr;"
      "lwc	%0 = [$r10];" // start = *timer_ptr;
      "nop;"
      "lwl	%0 = [%0];"
      // We do not want the compiler to reload the volatile pointers each
      // iteration, so we manually state the sequence of writes
      write256

      "lwc	%1 = [$r10];"
      "nop;"
      "lwl	%1 = [%1];" // stop = *timer_ptr
      : "=r"(start), "=r"(stop)
      : "r"(ptr1), "r"(ptr2), "r"(ptr3), "r"(v)
      : "$r10" /* temporary timer ptr reg*/);

  // Return result
  val = stop - start;
  double wordsPerCycle = n / (double)val;
  *((double *)arg) = wordsPerCycle;

  return;
}

void read_bandwidth_2x2(void *arg) {
  int sum = 0;
  volatile int start, stop, val;
  start = *timer_ptr; // Use timer ptr so we can refer to it in inline assembly
                      // by name

  // Set targets according to schedule
  int target;
  switch (get_cpuid()) {
  case 0: {
    target = 2;
    break;
  }
  case 1: {
    target = 3;
    break;
  }
  case 2: {
    target = 0;
    break;
  }
  case 3: {
    target = 1;
    break;
  }
  }

  volatile _SPM int *ptr =
      (volatile _IODEV int *)(mem_ptr + (target << BLOCKWIDTH));

  // 256 times, read a word from each of the other 3 cores in the 2x2 network
  const int n = 256;
  int v = 10; // Value to be written

  start = *timer_ptr;
  asm(
      // Cant inline timer measurement because for some reason, this
      // breaks the following read statement

      "li	$r10 = timer_ptr;"
      "lwc	%0 = [$r10];" // start = *timer_ptr;
      "nop;"
      "lwl	%0 = [%0];"
      "nop;"

      read256

      "lwc	%1 = [$r10];"
      "nop;"
      "lwl	%1 = [%1];" // stop = *timer_ptr

      : "=r"(start), "=r"(stop)
      : "r"(ptr)
      : "$r10", "$r11");
  stop = *timer_ptr;

  // Return result
  val = stop - start;
  double wordsPerCycle = n / (double)val;
  *((double *)arg) = wordsPerCycle;

  return;
}

void read_bandwidth_3x3(void *arg) {
  int sum = 0;
  volatile int start, stop, val;
  start = *timer_ptr; // Use timer ptr so we can refer to it in inline assembly
                      // by name

  // Set targets according to schedule
  int target;
  switch (get_cpuid()) {
  case 0: {
    target = 7;
    break;
  }
  case 1: {
    target = 8;
    break;
  }
  case 2: {
    target = 6;
    break;
  }
  case 3: {
    target = 1;
    break;
  }
  case 4: {
    target = 2;
    break;
  }
  case 5: {
    target = 0;
    break;
  }
  case 6: {
    target = 4;
    break;
  }
  case 7: {
    target = 5;
    break;
  }
  case 8: {
    target = 3;
    break;
  }
  }

  volatile _SPM int *ptr =
      (volatile _IODEV int *)(mem_ptr + (target << BLOCKWIDTH));

  // 256 times, read a word from each of the other 3 cores in the 2x2 network
  const int n = 256;
  int v = 10; // Value to be written

  start = *timer_ptr;
  asm(
      // Cant inline timer measurement because for some reason, this
      // breaks the following read statement

      "li	$r10 = timer_ptr;"
      "lwc	%0 = [$r10];" // start = *timer_ptr;
      "nop;"
      "lwl	%0 = [%0];"
      "nop;"

      read256

      "lwc	%1 = [$r10];"
      "nop;"
      "lwl	%1 = [%1];" // stop = *timer_ptr

      : "=r"(start), "=r"(stop)
      : "r"(ptr)
      : "$r10", "$r11");
  stop = *timer_ptr;

  // Return result
  val = stop - start;
  double wordsPerCycle = n / (double)val;
  *((double *)arg) = wordsPerCycle;

  return;
}

void write_maximum_bandwidth() {
  PRINTF("\n-- MAXIMUM WRITE BANDWITH --\n");
  PRINTF("[wpc] = Words per cycle\n");
  PRINTF("\n");

  double retvals[get_cpucnt()];
  for (int i = 1; i < get_cpucnt(); i++) {
    corethread_create(i, &write_bandwidth_2x2, (void *)&retvals[i]);
  }
  write_bandwidth_2x2((void *)&retvals[0]);
  for (int i = 1; i < get_cpucnt(); i++) {
    void *res;
    corethread_join(i, &res);
  }

  double sumBandwidth = 0;
  for (int i = 0; i < get_cpucnt(); i++) {
    sumBandwidth += retvals[i];
  }

  PRINTF("Single core bandwidths:\n");
  for (int i = 0; i < get_cpucnt(); i++) {
    PRINTF("\tCore %d: %f wpc\n", i, retvals[i]);
  }
  PRINTF("\nCummulative write bandwidth of network: %f wpc\n", sumBandwidth);
}

void read_random_bandwidth() {
  PRINTF("\n-- READ BANDWITH --\n");
  PRINTF("best-case read bandwidth test \n");
  PRINTF("[wpc] = Words per cycle\n");
  PRINTF("\n");

  double retvals[get_cpucnt()];
  for (int i = 1; i < get_cpucnt(); i++) {
    // corethread_create(i, &read_bandwidth_2x2, (void *)&retvals[i]);
    corethread_create(i, &read_bandwidth_3x3, (void *)&retvals[i]);
  }
  // read_bandwidth_2x2((void *)&retvals[0]);
  read_bandwidth_3x3((void *)&retvals[0]);

  for (int i = 1; i < get_cpucnt(); i++) {
    void *res;
    corethread_join(i, &res);
  }

  double sumBandwidth = 0;
  for (int i = 0; i < get_cpucnt(); i++) {
    sumBandwidth += retvals[i];
  }

  // PRINTF("Single core bandwidths:\n");
  for (int i = 0; i < get_cpucnt(); i++) {
    PRINTF("\tCore %d: %f wpc\n", i, retvals[i]);
  }
  PRINTF("\nCummulative read bandwidth of network: %f wpc\n", sumBandwidth);
}

int main() {
  latency_from_master();
  // write_maximum_bandwidth();
  // read_random_bandwidth();
}
