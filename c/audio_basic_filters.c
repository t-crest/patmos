#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "audio.h"
#include "audio.c"

#define ONE_16b 0x8000 //0x7FFF

#define BUFFER_SIZE 32
#define AUDIO_RECORDING_SIZE 44100*2
//#define AUDIO_RECORDING_SIZE 4
#define FILTER_ORDER_1PLUS 2

#define Fs 52083 // Hz


short x[AUDIO_RECORDING_SIZE][2] = {0}; //input
short y[AUDIO_RECORDING_SIZE][2] = {0}; //output

int Fc;

// LOCATION IN SCRATCHPAD MEMORY
#define ACCUM_ADDR  0x0000
#define B_ADDR      ( ACCUM_ADDR  + 2 * sizeof(int) )
//#define A_ADDR      ( B_ADDR      + 2 * sizeof(short) )
//#define X_FILT_ADDR ( A_ADDR      + 2 * sizeof(short) )
//#define Y_FILT_ADDR ( X_FILT_ADDR + 2 * FILTER_ORDER_1PLUS * sizeof(short) )

//to have fixed-point multiplication:
volatile int *accum = (volatile int *) ACCUM_ADDR;
short K_16;
volatile short *B = (volatile short *) B_ADDR; // [b1, b0]
short *A; // = (short *) A_ADDR; // [a1,  0]

short (*x_filter)[2]; // = (short (*)[2]) X_FILT_ADDR; // x_filter[FILTER_ORDER_1PLUS][2] = {0};
short (*y_filter)[2]; // = (short (*)[2]) Y_FILT_ADDR; // y_filter[FILTER_ORDER_1PLUS][2] = {0};


int filterIIR(short (*x)[2], short (*y)[2], short *B, short *A, int RES_SHIFT) {
  accum[0] = 0;
  accum[1] = 0;
  for(int i=0; i<FILTER_ORDER_1PLUS; i++) {
    // SIGNED SHIFT (arithmetical): losing a 6-bit resolution
    accum[0] += (B[i]*x[i][0]) >> 6;
    accum[0] -= (A[i]*y[i][0]) >> 6;
    accum[1] += (B[i]*x[i][1]) >> 6;
    accum[1] -= (A[i]*y[i][1]) >> 6;
  }
  //accumulator limits: [ (2^(30-6-1))-1 , -(2^(30-6-1)) ]
  //accumulator limits: [ 0x7FFFFF, 0x800000 ]
  // digital saturation
  for(int i=0; i<2; i++) {
    if (accum[i] > 0x7FFFFF) {
      accum[i] = 0x7FFFFF;
    }
    else {
      if (accum[i] < -0x800000) {
        accum[i] = -0x800000;
      }
    }
  }
  y[FILTER_ORDER_1PLUS-1][0] = accum[0] >> (9+RES_SHIFT);
  y[FILTER_ORDER_1PLUS-1][1] = accum[1] >> (9+RES_SHIFT);
  return 0;
}



int main() {

  //memory allocation for variables
  accum = calloc(2, sizeof(int));
  B = calloc(2, sizeof(short));
  A = calloc(2, sizeof(short));
  x_filter = calloc(FILTER_ORDER_1PLUS, sizeof(short[2]));
  y_filter = calloc(FILTER_ORDER_1PLUS, sizeof(short[2]));


  setup();

  // enable input and output
  *audioDacEnReg = 1;
  *audioAdcEnReg = 1;

  setInputBufferSize(BUFFER_SIZE);
  setOutputBufferSize(BUFFER_SIZE);


  printf("addresses at scratchpad mem: accum: %d, B: %d\n", accum, B);

  printf("Press KEY0 for real-time playing and KEY1 for recording\n");
  while(*keyReg == 15);
  if(*keyReg == 14) {
    while(*keyReg == 14);
    printf("Press KEY0 to play LPF and KEY1 for HPF\n");
    while(*keyReg == 15);
    if(*keyReg == 14) {
      //LPF
      printf("Calculating LPF...\n");
      Fc = 600; // Hz
      K_16 = (short) ( ONE_16b * tan(M_PI * Fc / Fs) );
      B[1] = (short) ( (K_16 << 15) / ( K_16 + ONE_16b) ); // b0
      B[0] = (short) ( (K_16 << 15) / ( K_16 + ONE_16b) ); // b1
      A[0] = (short) ( ((K_16 - ONE_16b) << 15) / (K_16 + ONE_16b) ); // a1
      printf("K: %d, b0: %d, b1: %d, a1: %d\n", K_16, B[1], B[0], A[0]);

      //first, fill filter buffer
      for(int i=0; i<(FILTER_ORDER_1PLUS-1); i++) {
        getInputBuffer(&x_filter[i][0], &x_filter[i][1]);
      }
      //when filter buffer is full, compute each output sample
      while(*keyReg != 3) {
        //first, read last sample
        getInputBuffer(&x_filter[FILTER_ORDER_1PLUS-1][0], &x_filter[FILTER_ORDER_1PLUS-1][1]);
        //then, calculate filter
        filterIIR(x_filter, y_filter, B, A, 0);
        //set output
        setOutputBuffer(y_filter[FILTER_ORDER_1PLUS-1][0], y_filter[FILTER_ORDER_1PLUS-1][1]);
        //finally, shift buffers left
        for(int j=0; j<FILTER_ORDER_1PLUS-1; j++) {
          x_filter[j][0] = x_filter[j+1][0];
          x_filter[j][1] = x_filter[j+1][1];
          y_filter[j][0] = y_filter[j+1][0];
          y_filter[j][1] = y_filter[j+1][1];
        }
      }
    }
    if(*keyReg == 13) {
      //HPF
      printf("Calculating HPF...\n");
      Fc = 5000; // Hz
      K_16 = (short) ( ONE_16b * tan(M_PI * Fc / Fs) );
      B[1] = (short) ( (ONE_16b << 15) / ( K_16 + ONE_16b) ); // b0
      B[0] = (short) ( ( ((-1) *ONE_16b) << 15 ) / ( K_16 + ONE_16b) ); // b1
      A[0] = (short) ( ((K_16 - ONE_16b) << 15) / (K_16 + ONE_16b) ); // a1
      printf("K: %d, b0: %d, b1: %d, a1: %d\n", K_16, B[1], B[0], A[0]);

      //first, fill filter buffer
      for(int i=0; i<(FILTER_ORDER_1PLUS-1); i++) {
        getInputBuffer(&x_filter[i][0], &x_filter[i][1]);
      }
      //when filter buffer is full, compute each output sample
      while(*keyReg != 3) {
        //first, read last sample
        getInputBuffer(&x_filter[FILTER_ORDER_1PLUS-1][0], &x_filter[FILTER_ORDER_1PLUS-1][1]);
        //then, calculate filter
        filterIIR(x_filter, y_filter, B, A, 0);
        //set output
        setOutputBuffer(y_filter[FILTER_ORDER_1PLUS-1][0], y_filter[FILTER_ORDER_1PLUS-1][1]);
        //finally, shift buffers left
        for(int j=0; j<FILTER_ORDER_1PLUS-1; j++) {
          x_filter[j][0] = x_filter[j+1][0];
          x_filter[j][1] = x_filter[j+1][1];
          y_filter[j][0] = y_filter[j+1][0];
          y_filter[j][1] = y_filter[j+1][1];
        }
      }
    }
  }
  if(*keyReg == 13) {
    printf("Recording...\n");
    for(int i=0; i<AUDIO_RECORDING_SIZE; i++) {
      getInputBuffer(&x[i][0], &x[i][1]);
    }
    //filtered signals calculation
    printf("Recording Finished. \n");
    while(*keyReg != 15);
    printf("Press KEY0 to calculate LPF and KEY1 for HPF\n");
    while(*keyReg == 15);
    if(*keyReg == 13) {
      //1st order HPF:
      printf("Calculating HPF...\n");
      Fc = 5000; // Hz
      K_16 = (short) ( ONE_16b * tan(M_PI * Fc / Fs) );
      B[1] = (short) ( (ONE_16b << 15) / ( K_16 + ONE_16b) ); // b0
      B[0] = (short) ( ( ((-1) *ONE_16b) << 15 ) / ( K_16 + ONE_16b) ); // b1
      A[0] = (short) ( ((K_16 - ONE_16b) << 15) / (K_16 + ONE_16b) ); // a1
      printf("K: %d, b0: %d, b1: %d, a1: %d\n", K_16, B[1], B[0], A[0]);

      int x_pnt = 0;

      //first, fill filter buffer
      for(x_pnt=0; x_pnt<(FILTER_ORDER_1PLUS-1); x_pnt++) {
        x_filter[x_pnt][0] = x[x_pnt][0];
        x_filter[x_pnt][1] = x[x_pnt][1];
      }
      //when filter buffer is full, compute each output sample
      for(x_pnt=(FILTER_ORDER_1PLUS-1); x_pnt<AUDIO_RECORDING_SIZE; x_pnt++) {
        //first, read last sample
        x_filter[FILTER_ORDER_1PLUS-1][0] = x[x_pnt][0];
        x_filter[FILTER_ORDER_1PLUS-1][1] = x[x_pnt][1];
        //then, calculate filter
        filterIIR(x_filter, y_filter, B, A, 0);
        //set output
        y[x_pnt][0] = y_filter[FILTER_ORDER_1PLUS-1][0];
        y[x_pnt][1] = y_filter[FILTER_ORDER_1PLUS-1][1];
        //finally, shift buffers left
        for(int j=0; j<FILTER_ORDER_1PLUS-1; j++) {
          x_filter[j][0] = x_filter[j+1][0];
          x_filter[j][1] = x_filter[j+1][1];
          y_filter[j][0] = y_filter[j+1][0];
          y_filter[j][1] = y_filter[j+1][1];
        }
      }
    }
    else {
      if(*keyReg == 14) {
        //1st order LPF:
        printf("Calculating LPF...\n");
        Fc = 600; // Hz
        K_16 = (short) ( ONE_16b * tan(M_PI * Fc / Fs) );
        B[1] = (short) ( (K_16 << 15) / ( K_16 + ONE_16b) ); // b0
        B[0] = (short) ( (K_16 << 15) / ( K_16 + ONE_16b) ); // b1
        A[0] = (short) ( ((K_16 - ONE_16b) << 15) / (K_16 + ONE_16b) ); // a1
        printf("K: %d, b0: %d, b1: %d, a1: %d\n", K_16, B[1], B[0], A[0]);

        int x_pnt = 0;

        //first, fill filter buffer
        for(x_pnt=0; x_pnt<(FILTER_ORDER_1PLUS-1); x_pnt++) {
          x_filter[x_pnt][0] = x[x_pnt][0];
          x_filter[x_pnt][1] = x[x_pnt][1];
        }
        //when filter buffer is full, compute each output sample
        for(x_pnt=(FILTER_ORDER_1PLUS-1); x_pnt<AUDIO_RECORDING_SIZE; x_pnt++) {
          //first, read last sample
          x_filter[FILTER_ORDER_1PLUS-1][0] = x[x_pnt][0];
          x_filter[FILTER_ORDER_1PLUS-1][1] = x[x_pnt][1];
          //then, calculate filter
          filterIIR(x_filter, y_filter, B, A, 0);
          //set output
          y[x_pnt][0] = y_filter[FILTER_ORDER_1PLUS-1][0];
          y[x_pnt][1] = y_filter[FILTER_ORDER_1PLUS-1][1];
          //finally, shift buffers left
          for(int j=0; j<FILTER_ORDER_1PLUS-1; j++) {
            x_filter[j][0] = x_filter[j+1][0];
            x_filter[j][1] = x_filter[j+1][1];
            y_filter[j][0] = y_filter[j+1][0];
            y_filter[j][1] = y_filter[j+1][1];
          }
        }
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
  }
  return 0;
}
