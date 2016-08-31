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
  printf("done first\n");

  printf("IWannaWaitVeryVeryLong\n");

  // audio data
  for(int j=0; j<3; j++) {
    for(int i=0; i<4; i++) {
      setOutputBuffer(audioArrayL[i]+3, audioArrayR[i]+3);
    }
  }
  printf("done second\n");

  return 0;

}
