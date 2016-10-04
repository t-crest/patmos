#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <math.h>


// This program is used to disassemble it and see the difference between
// Floating-Point multiplication and Fixed-Point multiplication in
// assembly language
// also, measuring the CPU cycles before and after

volatile int floatMultLoop(volatile float x, volatile float y) {
    volatile float result;
    //CPU cycles stuff
    int CPUcycles[10];
    for(int i=0; i<10; i++) {
        CPUcycles[i] = get_cpu_cycles();
        result = x * y;
    }
    printf("FLOAT result: %f\n", result);
    if(result < -200) {
        for(int i=1; i<10; i++) {
            printf("%d\n", (CPUcycles[i]-CPUcycles[i-1]));
        }
    }
    return result;
}

volatile int fixedMultLoop(volatile int x, volatile int y) {
    volatile int result;
    //CPU cycles stuff
    int CPUcycles[10];
    for(int i=0; i<10; i++) {
        CPUcycles[i] = get_cpu_cycles();
        result = x * y;
    }
    printf("INT result: %d\n", result);
    if(result < -200) {
        for(int i=1; i<10; i++) {
            printf("%d\n", (CPUcycles[i]-CPUcycles[i-1]));
        }
    }
    return result;
}

int main() {
  volatile unsigned long long cycleTime1;
  volatile unsigned long long cycleTime2;

  // FOR FLOATING POINT MULT:

  volatile float x_f, y_f, res_f;
  x_f = 11.333;
  y_f = -26.056;
  res_f = floatMultLoop(x_f, y_f);

  // FOR FIXED POINT MULT:

  volatile int x_i, y_i, res_i;
  x_i = 11;
  y_i = -26;
  res_i = fixedMultLoop(x_i, y_i);


  return 0;
}
