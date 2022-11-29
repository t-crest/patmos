/*
    This is a minimal C program to exlore WCET analysis.
    foo() is the function to analyze and it is not obvious
    if the multiplication or the addition path is the WCET path.

    Author: Martin Schoeberl
    Copyright: DTU, BSD License
*/

#include <stdio.h>

volatile int n = 123;

// foo is the analysis entry point that would be inlined with -O2
int foo(int b, int val) __attribute__((noinline));
int foo(int b, int val) {

  int i;

  if (b) {
    for (i=0; i<51; ++i) {
      val = val * n;
    }
  } else {
    for (i=0; i<73; ++i) {
      val = val + n;
    }
  }

  return val;
}

// The compiler shall not compute the result
volatile int seed = 3;

int main(int argc, char** argv) {

  int val = seed;
  int b = seed/4;

  int i = foo(b, val);
  printf("%d\n", i);

  return i;
}
