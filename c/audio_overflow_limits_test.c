#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <math.h>

volatile short a, b, c, d;

int main() {

  a = 0x7FFF;
  b = 0x7FFF;
  volatile int c = a * b;

  printf("%d * %d = %d\n", a, b, c);
  printf("0x%x * 0x%x = 0x%x\n", a, b, c);

  return 0;
}
