#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <math.h>
#include "audio.h"
#include "audio.c"

int main() {

  short inL = 0;
  short inR = 0;

  //buffer sizes
  setInputBufferSize(8);
  setOutputBufferSize(8);

  //enables
  *audioAdcEnReg = 1;
  *audioDacEnReg = 1;

  for(int i=0; i<12; i++) {
    getInputBuffer(&inL, &inR);
    setOutputBuffer(inL, inR);
  }

  printf("done\n");

  return 0;

}
