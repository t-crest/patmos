#include <stdio.h>
#include <time.h>
#include <machine/patmos.h>

volatile static int dummy;

int main(int argc, char **argv)
{
  clock_t tim;
  int i, j, k;
  
  printf("I am CPU %d\n", get_cpuid());
  for (i=0; i<10; ++i) {
    tim = clock();
    printf("Hello time %llu\n", tim);
    for (j=0; j<1000000; ++j) k = dummy;
  }
  return 0;
}
