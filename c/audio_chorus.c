#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <math.h>
#include "audio.h"
#include "audio.c"

#define BUFFERSIZE 256
#define CHORUS_PERIOD_1 4410 // 1/10 second
#define CHORUS_PERIOD_2 5512 // 1/8 second
#define CHORUS_OFFSET_SAMPLES_1 5000
#define CHORUS_OFFSET_SAMPLES_2 1000

int chorusArray1[CHORUS_PERIOD_1];
int chorusArray2[CHORUS_PERIOD_2];

short inL, inR, outR, outL;

int ch1Ind, ch2Int = 0;

void storeChorusArray(int chorusOffsetSamples1, int *chorusArray1, int SIZE_1, int chorusOffsetSamples2, int *chorusArray2, int SIZE_2) {
  for(int i=0; i<SIZE_1; i++) {
    chorusArray1[i] = chorusOffsetSamples1 + chorusOffsetSamples1*cos(2.0*M_PI* i / SIZE_1);
  }
  for(int i=0; i<SIZE_2; i++) {
    chorusArray2[i] = chorusOffsetSamples2 + chorusOffsetSamples2*sin(2.0*M_PI* i / SIZE_2);
  }
  printf("Chorus array storage done\n");
}


int main() {

  printf("CPU frequency: %d MHz\n", get_cpu_freq()/1000000);

  setup();

  setInputBufferSize(BUFFERSIZE);
  setOutputBufferSize(BUFFERSIZE);

  *audioDacEnReg = 1;
  *audioAdcEnReg = 1;

  //save tremolo array: 1 for sin, 0 for sawtooth
  storeChorusArray(CHORUS_OFFSET_SAMPLES_1, chorusArray1, CHORUS_PERIOD_2, CHORUS_OFFSET_SAMPLES_2, chorusArray2, CHORUS_PERIOD_2);

  short delayBuffer[CHORUS_OFFSET_SAMPLES_1*2][2];
  while(*keyReg != 3) {
    //for now, just one chorus line:
    for(int i=0; i<(CHORUS_OFFSET_SAMPLES_1*2); i++) {
      getInputBuffer(&inL, &inR);
      //store new samples
      delayBuffer[i][0] = inL>>2;
      delayBuffer[i][1] = inR>>2;
      //calculate output
      outL = delayBuffer[i][0] + delayBuffer[i-chorusArray1[ch1Ind]][0];
      outR = delayBuffer[i][1] + delayBuffer[i-chorusArray1[ch1Ind]][1];
      //write output
      setOutputBuffer(outL, outR);
      //increment chorus index
      ch1Ind = (ch1Ind+1) % CHORUS_PERIOD_1;
    }
  }

  return 0;
}
