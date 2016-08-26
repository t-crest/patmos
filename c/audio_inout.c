#include <machine/spm.h>
#include <stdio.h>
#include "audio.h"
#include "audio.c"

/*
 * @file		Audio_InOut.c
 * @author	Daniel Sanz Ausin s142290 & Fabian Goerge s150957
 * @brief	This program takes the input auido data and outputs it again
 */

int main() {
  setup();

  short inL = 0;
  short inR = 0;
  *audioDacEnReg = 1;
  *audioAdcEnReg = 1;

  while(*keyReg != 3) {
    getInputAudio(&inL,&inR);
    //printf("InL: %i InR: %i\n",inL,inR);
    setOutputAudio(inL,inR);
  }
  return 0;
}
