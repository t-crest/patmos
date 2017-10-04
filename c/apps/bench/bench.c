

#include<stdio.h>
#include <machine/patmos.h>

volatile static int field;

int main() {

  // Pointer to the deadline device
  volatile _IODEV int *dead_ptr = (volatile _IODEV int *) PATMOS_IO_DEADLINE;
  // Measure execution time with the clock cycle timer
  volatile _IODEV int *timer_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+4);
  // use function
  int start, end;
  int val;

  start = *timer_ptr;
  val = *timer_ptr-start;
  printf("Empty measurement %d\n", val);

  start = *timer_ptr;
  field = 0;
  val = *timer_ptr-start;
  // result is 23, 1 for the measurement, 1 for a constant load
  // between the two measurements. So a write is 21 clock cycles.
  printf("Single write %d\n", val);

  for (int i=0; i<100; ++i) {
    start = *timer_ptr;
    field = 0;
    val = *timer_ptr-start;
    // now 22 clock cycles
    printf("More writes %d\n", val);
  }


}
