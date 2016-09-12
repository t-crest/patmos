#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <math.h>
#include "audio.h"
#include "audio.c"

#define TAP_ORDER 8
#define FIR_COEF 1/TAP_ORDER

short inL, inR;

short x_l[TAP_ORDER] = {0};
short x_r[TAP_ORDER] = {0};

short y_l;
short y_r;

int main() {

  //setup();

  setInputBufferSize(32);
  setOutputBufferSize(32);

  // enable input and output
  *audioDacEnReg = 1;
  *audioAdcEnReg = 1;

  while(*keyReg != 3) {
    for(int i=0; i<TAP_ORDER; i++) {
      //store input in array
      getInputBuffer(&inL, &inR);
      // already multiply FIR coefficient
      x_l[i] = inL * FIR_COEF;
      x_r[i] = inR * FIR_COEF;

      //printf("input: %d, %d\n", inL, inR);
      //printf("stored: %d, %d\n", x_l[i], x_r[i]);

      //add output samples
      y_l = 0;
      y_r = 0;
      for(int j=0; j<TAP_ORDER; j++) {
        y_l += x_l[j];
        y_r += x_r[j];
      }
      // write output
      //setOutputBuffer(inL,inR);
      setOutputBuffer(y_l, y_r);
    }
  }
  return 0;
}
