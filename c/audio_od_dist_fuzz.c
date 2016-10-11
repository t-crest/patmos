#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ONE_16b 0x8000 //0x7FFF

#define OD_THRESHOLD 0x2AAB // ONE_16b * 1/3

#define BUFFER_SIZE 32

#define Fs 52083 // Hz

#include "audio.h"
#include "audio.c"

#define X_ADDR 0x00000000
#define Y_ADDR ( X_ADDR + 2 * sizeof(short) )

volatile _SPM short *x = (volatile _SPM short *) X_ADDR; // input audio
volatile _SPM short *y = (volatile _SPM short *) Y_ADDR; // output audio

int main() {

  setup();

  // enable input and output
  *audioDacEnReg = 1;
  *audioAdcEnReg = 1;

  setInputBufferSize(BUFFER_SIZE);
  setOutputBufferSize(BUFFER_SIZE);

  printf("Play!\n");

  /*
  while(*keyReg != 3) {
      getInputBufferSPM(&x[0], &x[1]);
      overdrive(x, y, OD_THRESHOLD);
      setOutputBuffer(y[0], y[1]);
  }
  */

  x[0] = 0x4000; //0.5
  x[1] = 0x4030; // 0.5014648
  overdrive(x, y, OD_THRESHOLD);

  return 0;
}
