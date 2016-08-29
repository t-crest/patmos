#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <math.h>
#include "audio.h"
#include "audio.c"

short audioArrayL[4] = { 1, 5, 16, 31 };
short audioArrayR[4] = { 2, 6, 17, 32 };

int main() {

  //enable DAC
  *audioDacEnReg = 1;

  // audio data
  for(int i=0; i<4; i++) {
    setOutputBuffer(audioArrayL[i], audioArrayR[i]);
  }
  printf("done\n");

  return 0;

}
