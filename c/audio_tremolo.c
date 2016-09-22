#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <math.h>
#include "audio.h"
#include "audio.c"


#define TREMOLO_PERIOD 22050 // 1/4 second: 44100/4

float tremoloArray[TREMOLO_PERIOD];
short inL, inR, outR, outL;

void storeTremoloArray(float *tremoloArray, int SIZE, int isSin) {
  if (isSin == 1) {
    printf("Selected tremolo type: Sin\n");
  }
  else {
    printf("Selected tremolo type: Sawtooth\n");
  }

  printf("Storing tremolo array...\n");
  for (int i = 0; i < SIZE; i++) {
    //if sin is selected:
    if (isSin == 1) {
      tremoloArray[i] = 0.5 + 0.5*cos(2.0*M_PI* i / SIZE);
    }
    //otherwise, sawtooth
    else {
      tremoloArray[i] = ((float)SIZE-(float)i-1)/((float)SIZE-1);
    }
  }
  printf("Tremolo array storage done\n");
}


int main() {

  printf("CPU frequency: %d MHz\n", get_cpu_freq()/1000000);

  setup();

  setInputBufferSize(32);
  setOutputBufferSize(32);

  *audioDacEnReg = 1;
  *audioAdcEnReg = 1;

  //save tremolo array: 1 for sin, 0 for sawtooth
  storeTremoloArray(tremoloArray, TREMOLO_PERIOD, 1);

  while(*keyReg != 3) {
    for(int i=0; i<TREMOLO_PERIOD; i++) {
      getInputBuffer(&inL, &inR);
      outL = inL * tremoloArray[i];
      outR = inR * tremoloArray[i];
      /*
      //a click every TREMOLO_PERIOD:
      if(i==0) {
       outL  = 16384;
       outR  = 16384;
      }
      else {
        outL = 0;
        outR = 0;
      }
      */
      setOutputBuffer(outL, outR);
    }
  }

  return 0;
}
