#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <math.h>
#include "audio.h"
#include "audio.c"

#define TAP_ORDER 8

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
      x_l[i] = inL / TAP_ORDER;
      x_r[i] = inR / TAP_ORDER;
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
