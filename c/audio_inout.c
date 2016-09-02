#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <math.h>
#include "audio.h"
#include "audio.c"

/*
 * @file		Audio_InOut.c
 * @author	Daniel Sanz Ausin s142290 & Fabian Goerge s150957
 * @brief	This program takes the input auido data and outputs it again
 */

int main() {
  setup();

  setOutputBufferSize(256);

  short inL = 0;
  short inR = 0;

  // enable input and output
  *audioDacEnReg = 1;
  *audioAdcEnReg = 1;

  while(*keyReg != 3) {
    getInputAudio(&inL,&inR);
    //printf("InL: %i InR: %i\n",inL,inR);
    setOutputBuffer(inL,inR);
  }
  return 0;
}
