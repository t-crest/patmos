#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <math.h>
#include "audio.h"
#include "audio.c"

#define ONE_16b 0x7FFF

#define BUFFER_SIZE 32

#define AUDIO_RECORDING_SIZE 44100*2

#define Fs 52083 // Hz


short x[AUDIO_RECORDING_SIZE][2]; //input
short x_h[2] = {0}; // stored delayed sample
short y[AUDIO_RECORDING_SIZE][2]; //output

int Fc;

//to have fixed-point multiplication:
//float K;
//float b0, b1, a1, c;
short K_16;
short b0_16, b1_16, a1_16, c_16;

int filter_1st(short (*x)[2], short *x_h, short (*y)[2], int SIZE, short b0, short a1, short c) {
  for(int i=0; i<SIZE; i++) {
    //calculate output
    y[i][0] = ( (b0 * x[i][0]) >> 15 ) + ( (c * x_h[0]) >> 15 );
    y[i][1] = ( (b0 * x[i][1]) >> 15 ) + ( (c * x_h[1]) >> 15 );

    //calculate next Xh(n-1)
    x_h[0] = x[i][0] - ( (a1 * x_h[0] ) >> 15 );
    x_h[1] = x[i][1] - ( (a1 * x_h[1] ) >> 15 );
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
    //K = tan(M_PI * Fc / Fs);
    //b0 = (  1 / ( K + 1 ) );
    //b1 = ( -1 / ( K + 1 ) );
    //a1 = ( ( K - 1 ) / ( K + 1 ) );
    //c  = b1 - ( b0 * a1 );
    K_16 = (short) ( ONE_16b * tan(M_PI * Fc / Fs) );
    b0_16 = (short) ( (ONE_16b << 15) / ( K_16 + ONE_16b) ); //shift before dividing
    b1_16 = (short) ( ( ((-1) *ONE_16b) << 15 ) / ( K_16 + ONE_16b) );
    a1_16 = (short) ( ((K_16 - ONE_16b) << 15) / (K_16 + ONE_16b) );
    c_16 = b1_16 - ( (b0_16 * a1_16) >> 15 ); // shift after multiplying to take only [29: 14] bits
    //printf("b0: %f, b1: %f, a1: %f, c: %f\n", b0, b1, a1, c);
    //printf("b0: %d, b1: %d, a1: %d, c: %d\n", b0_16, b1_16, a1_16, c_16);
    filter_1st(x, x_h, y, AUDIO_RECORDING_SIZE, b0_16, a1_16, c_16);
  }
  else {
    if(*keyReg == 14) {
      //1st order LPF:
      printf("Calculating LPF...\n");
      Fc = 600; // Hz
      //K = tan(M_PI * Fc / Fs);
      //b0 = K / ( K + 1 );
      //b1 = K / ( K + 1 );
      //a1 = ( K - 1 ) / ( K + 1 );
      //c  = b1 - ( b0 * a1 );
      K_16 = (short) ( ONE_16b * tan(M_PI * Fc / Fs) );
      b0_16 = (short) ( (K_16 << 15) / ( K_16 + ONE_16b) ); //shift before dividing
      b1_16 = (short) ( (K_16 << 15) / ( K_16 + ONE_16b) );
      a1_16 = (short) ( ((K_16 - ONE_16b) << 15) / (K_16 + ONE_16b) );
      c_16 = b1_16 - ( (b0_16 * a1_16) >> 15 ); // shift after multiplying to take only [29: 14] bits
      //printf("b0: %f, b1: %f, a1: %f, c: %f\n", b0, b1, a1, c);
      //printf("b0: %d, b1: %d, a1: %d, c: %d\n", b0_16, b1_16, a1_16, c_16);
      filter_1st(x, x_h, y, AUDIO_RECORDING_SIZE, b0_16, a1_16, c_16);
    }
  }
  printf("Filter calculation finished\n");



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
