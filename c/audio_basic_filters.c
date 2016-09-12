#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <math.h>
#include "audio.h"
#include "audio.c"

#define FILTER_ORDER 1

#define Fs 44100 // Hz
#define Fc 500   // Hz

#define K tan(M_PI * Fc / Fs)

short inL, inR, outL, outR;

float x[2]; // input audio
float x_h[2]; // stored delayed sample
float y[2]; // output audio

int basic_filter_1st_y(float *x, float *x_h, float *y, float b0, float c) {
  for(int i=0; i<2; i++) {
    y[i] = b0 * x[i] + c * x_h[i];
  }
  return 0;
}

int basic_filter_1st_xh(float *x, float *x_h, float a1) {
  for(int i=0; i<2; i++) {
    x_h[i] = x[i] - a1 * x_h[i];
  }
  return 0;
}

int main() {

  //setup();

  //1st order LPF:
  float b0 = K / ( K + 1 );
  float b1 = K / ( K + 1 );
  float a1 = ( K - 1 ) / ( K + 1 );
  float c  = b1 - b0 * a1;

  //  printf("b0: %f, b1: %f, a1: %f, c: %f\n", b0, b1, a1, c);

  setInputBufferSize(32);
  setOutputBufferSize(32);

  // enable input and output
  *audioDacEnReg = 1;
  *audioAdcEnReg = 1;

  //when keyReg 13, decrement, when keyReg 14, increment


  while(*keyReg != 3) {
    getInputBuffer(&inL, &inR);
    x[0] = (float) inL;
    x[1] = (float) inR;
    //printf("input: %f, %f  , stored: %f, %f\n", x[0], x[1], x_h[0], x_h[1]);
    basic_filter_1st_y(&x, &x_h, &y, b0, c);
    basic_filter_1st_xh(&x, &x_h, a1);
    //printf("output: %f, %f  , new stored: %f, %f\n", y[0], y[1], x_h[0], x_h[1]);
    outL = (short) y[0];
    outR = (short) y[1];
    setOutputBuffer(inL, inR);
  }

  return 0;
}
