#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <math.h>


// This program is used to disassemble it and see the difference between
// Floating-Point multiplication and Fixed-Point multiplication in
// assembly language
// also, measuring the CPU cycles before and after

volatile float floatMult(volatile float x, volatile float y) {
  return (x * y);

  /*
  //volatile unsigned long long timesArray[10];
  volatile unsigned long long cycleTime1;
  volatile unsigned long long cycleTime2;
  volatile float result;
  for(int i=0; i<10; i++) {
    cycleTime1 = get_cpu_cycles();
    result = x*y;
    cycleTime2 = get_cpu_cycles();
    timesArray[i] = cycleTime2 - cycleTime1;
    x++;
  }
  if(((int) result) < -200) {
    for(int i=0; i<10; i++) {
      printf("%llu\n", timesArray[i]);
    }
  }
  return result;
  */
}

volatile int fixedMult(volatile int x, volatile int y) {
  return (x * y);

  /*
  volatile unsigned long long timesArray[10];
  volatile unsigned long long cycleTime1;
  volatile unsigned long long cycleTime2;
  volatile int result = -250;
  for(int i=0; i<10; i++) {
    cycleTime1 = get_cpu_cycles();
    result = x*y;
    cycleTime2 = get_cpu_cycles();
    timesArray[i] = cycleTime2 - cycleTime1;
    x++;
  }
  if(result < -200) {
    for(int i=0; i<10; i++) {
      printf("%llu\n", timesArray[i]);
    }
  }

  return result;
  */
}

int main() {
  volatile unsigned long long timesArray[10];
  volatile unsigned long long cycleTime1;
  volatile unsigned long long cycleTime2;

  /*
  // FOR FLOATING POINT MULT:

  volatile float x_f, y_f, res_f;
  x_f = 11.333;
  y_f = -26.056;
  for(int i=0; i<10; i++) {
    cycleTime1 = get_cpu_cycles();
    res_f = floatMult(x_f, y_f);
    cycleTime2 = get_cpu_cycles();
    timesArray[i] = cycleTime2 - cycleTime1;
  }
  printf("result: %f\n", res_f);
  if(res_f < -200) {
    for(int i=0; i<10; i++) {
      printf("%llu\n", timesArray[i]);
    }
  }
  */
  // FOR FIXED POINT MULT:

  volatile int x_i, y_i, res_i;
  x_i = 11;
  y_i = -26;
  for(int i=0; i<10; i++) {
    cycleTime1 = get_cpu_cycles();
    res_i = fixedMult(x_i, y_i);
    cycleTime2 = get_cpu_cycles();
    timesArray[i] = cycleTime2 - cycleTime1;
  }
  printf("result: %d\n", res_i);
  if(res_i < -200) {
    for(int i=0; i<10; i++) {
      printf("%llu\n", timesArray[i]);
    }
  }


  return 0;
}
