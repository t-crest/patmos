#include <stdio.h>
#include <machine/patmos.h>
int main(int argc, char **argv)
{
  if (get_cpuid()==0)
  {
  	puts("Hello, World!");
  }
  return 0;
}
