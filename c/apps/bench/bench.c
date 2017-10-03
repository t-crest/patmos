

#include<stdio.h>
#include <machine/patmos.h>


int main() {

  printf("Hello World\n");
  printf("%08x\n", PATMOS_IO_DEADLINE);
}
