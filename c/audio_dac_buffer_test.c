#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <math.h>
#include "audio.h"
#include "audio.c"

short audioArrayL[4] = { 0x01, 0x05, 0x10, 0x1F };
short audioArrayR[4] = { 0x02, 0x06, 0x11, 0x20 };

int main() {

  //set buffer size
  setOutputBufferSize(8);

  //enable DAC
  *audioDacEnReg = 1;

  // audio data
  for(int i=0; i<4; i++) {
    setOutputBuffer(audioArrayL[i], audioArrayR[i]);
  }
  printf("Done first\n");

  printf("IWannaWaitVeryVeryLong\n");

  // audio data
  for(int j=1; j<4; j++) {
    for(int i=0; i<4; i++) {
      setOutputBuffer(audioArrayL[i]+j, audioArrayR[i]+j);
    }
  }

  printf("done second\n");

  return 0;

}
