/*
    This is a minimal C program to exlore WCET analysis.
    foo() is the function to analyze and it is not obvious
    if the multiplication or the addition path is the WCET path.

    Author: Martin Schoeberl
    Copyright: DTU, BSD License
*/

#include <stdio.h>

// foo is the analysis entry point that would be inlined with -O2
int foo(int b, int val, int val2) __attribute__((noinline));
int foo(int b, int val, int val2) {

  int i;

  if (b) {
    for (i=0; i<51; ++i) {
      val = val * val2;
    }
  } else {
    for (i=0; i<73; ++i) {
      val = val + val2;
    }
  }

  return val;
}

// The compiler shall not compute the result
volatile int seed = 3;

int main(int argc, char** argv) {

  int val = seed;
  int val2 = seed+seed;
  int b = seed/4;

  int i = foo(b, val, val2);
//  printf("%d\n", i);

  return i;
}
