#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>

#define ALLOC_SIZE  (16*get_cpuid())
#define ALLOC_COUNT (1*get_cpuid())

int main(int argc, char **argv)
{
  if (get_cpuid() == 0) {
    puts("Hello, World!");
    puts("This is Major Tom to Ground Control.");
    puts("Ten-nine-eight-seven-six-five-four-three-two-one.");
  } else {
    for (int i = 0; i < ALLOC_COUNT; i++) {
      volatile char *foo = malloc(ALLOC_SIZE);
      for (int k = 0; k < ALLOC_SIZE; k++) {
        foo[k] = get_cpuid();
      }
      free((char *)foo);
    }
  }
  return 0;
}
