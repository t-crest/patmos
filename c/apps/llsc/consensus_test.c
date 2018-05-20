/*
    Test program for the LL/SC SPM.

    An implementation of a consensus protocol. For more info,
    https://en.wikipedia.org/wiki/Consensus_(computer_science)

    Authors: Davide Laezza - Roberts Fanning - Wenhao Li
*/

#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>

#include "libcorethread/corethread.h"

// Initial value for the shared address
#define INIT (-1)

// Whatever this contant means, it is needed
const int NOC_MASTER = 0;

// Timer pointer for random delay
volatile _IODEV int* us_ptr = (volatile _IODEV int *) (PATMOS_IO_TIMER+12);

/*
    Shared scratchpad memory address, whatever the actual type.
    Needs to be a macro otherwise the compiler complains about addresses not
    being constant at compile-time.
*/
#define SHARED_SPM (0xE8000000)
volatile _SPM int* sspm = (volatile _SPM int *) (SHARED_SPM);
volatile _SPM int* shared_addr = (volatile _SPM int *) (SHARED_SPM + 32);

// This function busy waits for the specified period, in microseconds
void wait_period(unsigned int period) {
    int next = *us_ptr + period;
    while (*us_ptr - next < 0);
}


/*
    The choice of the consensus protocol. In a first-come-first-serve
    approach, the shared variable is read, and the own value is only written
    if the default value is read. LL/SC prevents the occurring of lost updates
*/
void choose(void* args) {

    // Random period creation
    unsigned int seed = 0x7FFF - ((unsigned int) get_cpu_usecs());
    unsigned int period = rand_r(&seed) & 0xFFFF;

    // The value that this thraead would decide for
    int my_value = (int) args;

    // The value in the shared address
    int value = *shared_addr;

    // This increases the chances of a lost update occurring
    wait_period(period);

    // Writing only if the default value is read.
    if (value == INIT) {
        *shared_addr = my_value;
    }

    /*
        Returning the value at the shared address, regardless of
        the write results
    */
    int ret = *shared_addr;
    corethread_exit((void*) ret);
}

/*
    The main function. Writes the default value at the shared address, then
    spawns the chooser threads and waits for them to completes. Finally,
    it checks whether they all decided for the same value.
*/
int main() {
  int a = *shared_addr;                     // So that INIT can be written
  int slaves_count = get_cpucnt() - 1;
  int k;
  int* choices = (int*) malloc(sizeof(int) * slaves_count);
  int broken = 0;                           // Flag for the final check

  *shared_addr = INIT;                      // Writing INIT

  // Spawning chooser threads
  for (k = 0; k < slaves_count; ++k) {
    corethread_create(k + 1, &choose, (void*) k);
  }

  // Joining chooser threads
  for (k = 0; k < slaves_count; ++k) {
    corethread_join(k + 1, (void**) &choices[k]);
  }

  // Checking for all the values to be equal
  for (k = 0; k < slaves_count - 1; ++k) {

      // Different values found
      if (choices[k] != choices[k + 1]) {
          printf("Consensus not reached: found [");

          // Printing the results
          for (k = 0; k < slaves_count - 1; ++k) {
              printf("%d, ", choices[k]);
          }
          printf("%d]\n", choices[slaves_count - 1]);

          broken = 1;
          break;
      }
  }

  // Search flag not set, success!
  if (!broken) {
      printf("Consensus reached on %d\n", choices[0]);
  }

  free(choices);
  return 0;
}
