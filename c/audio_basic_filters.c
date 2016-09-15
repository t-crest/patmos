#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <math.h>
#include "audio.h"
#include "audio.c"

#define BUFFER_SIZE 32

#define AUDIO_RECORDING_SIZE 44100*2

#define Fs 52083 // Hz


short x[AUDIO_RECORDING_SIZE][2]; //input
short x_h[2] = {0}; // stored delayed sample
short y[AUDIO_RECORDING_SIZE][2]; //output

int Fc;
float K, b0, b1, a1, c;


int filter_1st(short (*x)[2], short *x_h, short (*y)[2], int SIZE, float b0, float a1, float c) {
  for(int i=0; i<SIZE; i++) {
    //calculate output
    //y[i][0] = (short)(b0 * x[i][0]) + (short)(c * x_h[0]);
    //y[i][1] = (short)(b0 * x[i][1]) + (short)(c * x_h[1]);
    y[i][0] = (b0 * x[i][0]) + (c * x_h[0]);
    y[i][1] = (b0 * x[i][1]) + (c * x_h[1]);

    //calculate next Xh(n-1)
    //x_h[0] = x[i][0] - (short)(a1 * x_h[0]);
    //x_h[1] = x[i][1] - (short)(a1 * x_h[1]);
    x_h[0] = x[i][0] - (a1 * x_h[0]);
    x_h[1] = x[i][1] - (a1 * x_h[1]);
  }
  return 0;
}



int main() {

  setup();

  // enable input and output
  *audioDacEnReg = 1;
  *audioAdcEnReg = 1;

  setInputBufferSize(BUFFER_SIZE);
  setOutputBufferSize(BUFFER_SIZE);

  //input recording
  printf("Press KEY0 to start recording\n");
  while(*keyReg != 14);
  printf("Recording...\n");
  for(int i=0; i<AUDIO_RECORDING_SIZE; i++) {
    getInputBuffer(&x[i][0], &x[i][1]);
  }
  //filtered signals calculation
  printf("Recording Finished. \n");
  printf("Press KEY0 to calculate LPF and KEY1 for HPF\n");
  while(*keyReg == 15);
  if(*keyReg == 13) {
    //1st order HPF:
    printf("Calculating HPF...\n");
    Fc = 5000; // Hz
    K = tan(M_PI * Fc / Fs);
    b0 = 1 / ( K + 1 );
    b1 = -1 / ( K + 1 );
    a1 = ( K - 1 ) / ( K + 1 );
    c  = b1 - ( b0 * a1 );
    filter_1st(x, x_h, y, AUDIO_RECORDING_SIZE, b0, a1, c);
  }
  else {
    if(*keyReg == 14) {
      //1st order LPF:
      printf("Calculating LPF...\n");
      Fc = 600; // Hz
      K = tan(M_PI * Fc / Fs);
      b0 = K / ( K + 1 );
      b1 = K / ( K + 1 );
      a1 = ( K - 1 ) / ( K + 1 );
      c  = b1 - ( b0 * a1 );
      filter_1st(x, x_h, y, AUDIO_RECORDING_SIZE, b0, a1, c);
    }
  }
  printf("Filter calculation finished\n");

  /*
  printf("Recording done. Press KEY0 again to look for input max\n");
  while(*keyReg != 14);
  short maxLeft = 0;
  for(int i=0; i<AUDIO_RECORDING_SIZE; i++) {
    if(abs(audioRecording[i][0]) >= maxLeft) {
      maxLeft = abs(audioRecording[i][0]);
      printf("MAX found: 0x%x at time %f\n", maxLeft, ((float)i/Fs));
    }
  }
  */



  int endBool = 0;
  int playBoolClean, playBoolFilt = 0;
  printf("KEY1: FILTERED, kEY0: DRY\n");
  while(!endBool) {
    switch(*keyReg) {
    case 14:
      //play clean
      if(!playBoolClean) {
        printf("Clean audio playback\n");
        for(int i=0; i<AUDIO_RECORDING_SIZE; i++) {
          setOutputBuffer(x[i][0], x[i][1]);
        }
        playBoolClean = 1;
      }
      break;
    case 13:
      //play HPF
      if(!playBoolFilt) {
        printf("Filtered audio playback\n");
        for(int i=0; i<AUDIO_RECORDING_SIZE; i++) {
          setOutputBuffer(y[i][0], y[i][1]);
        }
        playBoolFilt = 1;
      }
      break;
    case 3:
      endBool = 1;
      break;
    default:
      if ( playBoolClean || playBoolFilt ) {
        printf("KEY1: FILTERED, kEY0: DRY\n");
        playBoolClean = 0;
        playBoolFilt = 0;
      }
    }
  }

  return 0;
}
