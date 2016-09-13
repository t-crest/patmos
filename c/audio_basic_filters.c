#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <math.h>
#include "audio.h"
#include "audio.c"

#define BUFFER_SIZE 32

#define Fs 44100 // Hz
#define Fc 1000   // Hz

#define K tan(M_PI * Fc / Fs)

short x[BUFFER_SIZE][2] = {0}; // input audio
short x_h[2] = {0}; // stored delayed sample
short y[BUFFER_SIZE][2] = {0}; // output audio



int filter_1st(short (*x)[2], short *x_h, short (*y)[2], int SIZE, float b0, float a1, float c) {
  for(int i=0; i<SIZE; i++) {
    //calculate output
    y[i][0] = (short)(b0 * x[i][0]) + (short)(c * x_h[0]);
    y[i][1] = (short)(b0 * x[i][1]) + (short)(c * x_h[1]);
    //y[i][0] = (b0 * x[i][0]) + (c * x_h[0]);
    //y[i][1] = (b0 * x[i][1]) + (c * x_h[1]);

    //calculate next Xh(n-1)
    x_h[0] = x[i][0] - (short)(a1 * x_h[0]);
    x_h[1] = x[i][1] - (short)(a1 * x_h[1]);
    //x_h[0] = x[i][0] - (a1 * x_h[0]);
    //x_h[1] = x[i][1] - (a1 * x_h[1]);
  }
  return 0;
}



int main() {

  setup();

  //1st order LPF:
  float b0 = 1 / ( K + 1 );
  float b1 = -1 / ( K + 1 );
  float a1 = ( K - 1 ) / ( K + 1 );
  float c  = b1 - b0 * a1;

  printf("b0: %f, b1: %f, a1: %f, c: %f\n", b0, b1, a1, c);

  setInputBufferSize(BUFFER_SIZE);
  setOutputBufferSize(BUFFER_SIZE);

  // enable input and output
  *audioDacEnReg = 1;
  *audioAdcEnReg = 1;

  //when keyReg 13, decrement, when keyReg 14, increment

  while(*keyReg != 3) {
    for(int i=0; i<BUFFER_SIZE; i++) {
      getInputBuffer(&x[i][0], &x[i][1]);
    }
    filter_1st(x, x_h, y, BUFFER_SIZE, b0, a1, c);
    for(int i=0; i<BUFFER_SIZE; i++) {
      setOutputBuffer(y[i][0], y[i][1]);
    }
  }
  return 0;
}
