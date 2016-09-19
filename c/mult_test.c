#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <math.h>


// This program is used to disassemble it and see the difference between
// Floating-Point multiplication and Fixed-Point multiplication in
// assembly language


float floatMult(float x, float y) {
  float result = x*y;
  return result;
}

int fixedMult(int x, int y) {
  int result = x*y;
  return result;
}

int main() {
  float x = 11.333;
  float y = -26.056;
  float res = floatMult(x, y);
  //int res = fixedMult((int)x, (int)y);
}
