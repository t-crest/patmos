#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ONE_16b 0x8000 //0x7FFF

#define BUFFER_SIZE 32
#define AUDIO_RECORDING_SIZE 44100*2
#define FILTER_ORDER_1PLUS 3

#define Fs 52083 // Hz

#include "audio.h"
#include "audio.c"


short x[AUDIO_RECORDING_SIZE][2] = {0}; //input
short y[AUDIO_RECORDING_SIZE][2] = {0}; //output

// LOCATION IN SCRATCHPAD MEMORY
#define ACCUM_ADDR  0x00000000
#define B_ADDR      ( ACCUM_ADDR  + 2 * sizeof(int) )
#define A_ADDR      ( B_ADDR      + FILTER_ORDER_1PLUS * sizeof(short) )
#define X_FILT_ADDR ( A_ADDR      + FILTER_ORDER_1PLUS * sizeof(short) )
#define Y_FILT_ADDR ( X_FILT_ADDR + 2 * FILTER_ORDER_1PLUS * sizeof(short) )
#define PNT_ADDR    ( Y_FILT_ADDR + 2 * FILTER_ORDER_1PLUS * sizeof(short) )
#define SFTLFT_ADDR ( PNT_ADDR    + sizeof(int) )

//to have fixed-point multiplication:
float K;
volatile _SPM int *accum           = (volatile _SPM int *)        ACCUM_ADDR;
volatile _SPM short *B             = (volatile _SPM short *)      B_ADDR; // [b2, b1, b0]
volatile _SPM short *A             = (volatile _SPM short *)      A_ADDR; // [a2, a1,  1]
volatile _SPM short (*x_filter)[2] = (volatile _SPM short (*)[2]) X_FILT_ADDR; // x_filter[FILTER_ORDER_1PLUS][2] = {0};
volatile _SPM short (*y_filter)[2] = (volatile _SPM short (*)[2]) Y_FILT_ADDR; // y_filter[FILTER_ORDER_1PLUS][2] = {0};
volatile _SPM int *pnt             = (volatile _SPM int *)        PNT_ADDR; // pointer indicates last position of x_filter buffer
volatile _SPM int *shiftLeft       = (volatile _SPM int *)        SFTLFT_ADDR; //shift left amount;



int calc_filter_coeff(volatile _SPM short *B, volatile _SPM short *A, float K, int Fc, float Q, int type) {
  K = tan(M_PI * Fc / Fs);// K is same for all
  float Bfl[FILTER_ORDER_1PLUS] = {0};
  float Afl[FILTER_ORDER_1PLUS] = {0};
  float common_factor;
  if(type == 0) { //LPF
    if(FILTER_ORDER_1PLUS == 2) { //1st order
      printf("Calculating LPF for 1st order...\n");
      Bfl[1] = K/(K+1); //b0
      Bfl[0] = K/(K+1); //b1
      Afl[0] = (K-1)/(K+1); //a1
    }
    else {
      if(FILTER_ORDER_1PLUS == 3) { //2nd order
        printf("Calculating LPF for 2nd order...\n");
        common_factor = 1/(pow(K,2)*Q + K + Q);
        Bfl[2] = pow(K,2)*Q*common_factor; //b0
        Bfl[1] = 2*pow(K,2)*Q*common_factor; //b1
        Bfl[0] = pow(K,2)*Q*common_factor; //b2
        Afl[1] = 2*Q*(pow(K,2)-1)*common_factor; //a1
        Afl[0] = (pow(K,2)*Q - K + Q)*common_factor; //a2
      }
    }
  }
  else {
    if(type == 1) { // HPF
      if(FILTER_ORDER_1PLUS == 2) { //1st order
        printf("Calculating HPF for 1st order...\n");
        Bfl[1] =  1/(K+1); //b0
        Bfl[0] = -1/(K+1); //b1
        Afl[0] = (K-1)/(K+1); //a1
      }
      else {
        if(FILTER_ORDER_1PLUS == 3) { //2nd order
          printf("Calculating HPF for 2nd order...\n");
          common_factor = 1/(pow(K,2)*Q + K + Q);
          Bfl[2] = Q*common_factor; //b0
          Bfl[1] = -2*Q*common_factor; //b1
          Bfl[0] = Q*common_factor; //b2
          Afl[1] = 2*Q*(pow(K,2)-1)*common_factor; //a1
          Afl[0] = (pow(K,2)*Q - K + Q)*common_factor; //a2
        }
      }
    }
  }

  //check for overflow if coefficients
  float maxVal = 0;
  for(int i=0; i<FILTER_ORDER_1PLUS; i++) {
    if( (fabs(Bfl[i]) > 1) && (fabs(Bfl[i]) > maxVal) ) {
      maxVal = fabs(Bfl[i]);
    }
    if( (fabs(Afl[i]) > 1) && (fabs(Afl[i]) > maxVal) ) {
      maxVal = fabs(Afl[i]);
    }
  }
  //if coefficients were too high, scale down
  if(maxVal > 1) {
    printf("Greatest coefficient found is %f, ", maxVal);
  }
  while(maxVal > 1) { //loop until maxVal < 1
    *shiftLeft = *shiftLeft + 1; //here we shift right, but the IIR result will be shifted left
    maxVal--;
  }
  printf("shift left amount is %d\n", *shiftLeft);
  // now all coefficients should be between 0 and 1
  for(int i=0; i<FILTER_ORDER_1PLUS; i++) {
    B[i] = (short) ( (int) (ONE_16b * Bfl[i]) >> *shiftLeft );
    A[i] = (short) ( (int) (ONE_16b * Afl[i]) >> *shiftLeft );
  }
  if(FILTER_ORDER_1PLUS == 2) {
    printf("done! K: %f, b0: %d, b1: %d, a0, %d, a1: %d\n", K, B[1], B[0], A[1], A[0]);
  }
  if(FILTER_ORDER_1PLUS == 3) {
    printf("done! K: %f, common_factor: %f, b0: %d, b1: %d, b2 : %d, a0: %d, a1: %d, a2: %d\n", K, common_factor, B[2], B[1], B[0], A[2], A[1], A[0]);
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

  //shift left amount starts at 0
  *shiftLeft = 0;

  printf("addresses at scratchpad mem: accum: 0x%x, B: 0x%x, A: 0x%x, x_filter: 0x%x, y_filter: 0x%x, pnt: 0x%x, shiftLeft: 0x%x\n", (int)accum, (int)B, (int)A, (int)x_filter, (int)y_filter, (int)pnt, (int)shiftLeft);

  printf("Press KEY0 for real-time playing and KEY1 for recording\n");
  while(*keyReg == 15);
  if(*keyReg == 14) {
    while(*keyReg == 14);
    printf("Press KEY0 to play LPF and KEY1 for HPF\n");
    while(*keyReg == 15);
    if(*keyReg == 14) {
      calc_filter_coeff(B, A, K, 600, 0.707, 0); // 0 for LFP
    }
    if(*keyReg == 13) {
      calc_filter_coeff(B, A, K, 5000, 0.707, 1); // 1 for HFP
    }

    //first, fill filter buffer
    for(*pnt=0; *pnt<(FILTER_ORDER_1PLUS-1); *pnt++) {
      getInputBufferSPM(&x_filter[*pnt][0], &x_filter[*pnt][1]);
    }
    //when filter buffer is full, compute each output sample
    while(*keyReg != 3) {
      //increment pointer
      *pnt = (*pnt+1) % FILTER_ORDER_1PLUS;
      //first, read last sample
      getInputBufferSPM(&x_filter[*pnt][0], &x_filter[*pnt][1]);
      //then, calculate filter
      filterIIR(pnt, x_filter, y_filter, accum, B, A, *shiftLeft);
      //set output
      setOutputBuffer(y_filter[*pnt][0], y_filter[*pnt][1]);
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
      calc_filter_coeff(B, A, K, 5000, 0.707, 1); // 1 for HFP
    }
    if(*keyReg == 14) {
      calc_filter_coeff(B, A, K, 600, 0.707, 0); // 0 for LFP
    }

    int x_pnt = 0;

    //first, fill filter buffer
    for(*pnt=0; *pnt<(FILTER_ORDER_1PLUS-1); *pnt++) {
      x_filter[*pnt][0] = x[*pnt][0];
      x_filter[*pnt][1] = x[*pnt][1];
    }
    x_pnt = *pnt;
    //when filter buffer is full, compute each output sample
    for(x_pnt=(FILTER_ORDER_1PLUS-1); x_pnt<AUDIO_RECORDING_SIZE; x_pnt++) {
      //increment pointer
      *pnt = (*pnt+1) % FILTER_ORDER_1PLUS;
      //first, read last sample
      x_filter[*pnt][0] = x[x_pnt][0];
      x_filter[*pnt][1] = x[x_pnt][1];
      //then, calculate filter
      filterIIR(pnt, x_filter, y_filter, accum, B, A, *shiftLeft);
      //set output
      y[x_pnt][0] = y_filter[*pnt][0];
      y[x_pnt][1] = y_filter[*pnt][1];
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
