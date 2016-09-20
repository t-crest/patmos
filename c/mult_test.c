#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <math.h>


// This program is used to disassemble it and see the difference between
// Floating-Point multiplication and Fixed-Point multiplication in
// assembly language
// also, measuring the CPU cycles before and after

float floatMult(volatile float x, volatile float y) {
  unsigned long long cycleTime1 = get_cpu_cycles();
  volatile float result = x*y;
  unsigned long long cycleTime2 = get_cpu_cycles();

  if(((int)result) < -200) {
    printf("%llu, %llu\n", cycleTime1, cycleTime2);
    printf("%llu\n", (cycleTime2-cycleTime1));
  }

  return result;
}

int fixedMult(volatile int x, volatile int y) {
  unsigned long long cycleTime1 = get_cpu_cycles();
  volatile int result = x*y;
  unsigned long long cycleTime2 = get_cpu_cycles();

  if(result < -200) {
    printf("%llu, %llu\n", cycleTime1, cycleTime2);
    printf("%llu\n", (cycleTime2-cycleTime1));
  }

  return result;
}

int main() {
  volatile float x = 11.333;
  volatile float y = -26.056;
  volatile float res = floatMult(x, y);
  //volatile int res = fixedMult((volatile int)x, (volatile int)y);

  return 0;
}
