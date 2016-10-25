#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <math.h>

#define ONE_16b 0x7FFF
#define BUFFER_SIZE 128
#define Fs 52083 // Hz

#include "audio.h"
#include "audio.c"

// LOCATION OF VARS AT SPM:
#define X_ADDR      0x00000000
#define Y_ADDR      ( X_ADDR   + 2 * sizeof(short) )
#define PNT_ADDR    ( Y_ADDR   + 2 * sizeof(short) )

//variables in local SPM:
volatile _SPM short *x    = (volatile _SPM short *) X_ADDR; //input audio
volatile _SPM short *y    = (volatile _SPM short *) Y_ADDR; //output audio
volatile _SPM int   *pnt  = (volatile _SPM int *)   PNT_ADDR; //pointer to array

//location in external SRAM:
int sinArray[Fs]; //maximum period: 1 second
//decide tremolo period here:
const int TREMOLO_PERIOD = Fs/4;
int usedArray[TREMOLO_PERIOD];


int main() {

  setup(1);

  setInputBufferSize(BUFFER_SIZE);
  setOutputBufferSize(BUFFER_SIZE);

  *audioDacEnReg = 1;
  *audioAdcEnReg = 1;


  //store sin: 1 second betwen -1 and 1
  storeSin(sinArray, Fs, 0, ONE_16b);

  //calculate interpolated array:
  float arrayDivider = (float)Fs/(float)TREMOLO_PERIOD;
  printf("Array Divider is: %f\n", arrayDivider);
  for(int i=0; i<TREMOLO_PERIOD; i++) {
      //offset = 0.5, amplitude = 0.4
      usedArray[i] = (ONE_16b*0.6) + 0.3*sinArray[(int)floor(i*arrayDivider)];
  }

  *pnt = 0;
  while(*keyReg != 3) {
      //update pointer
      *pnt = (*pnt + 1) % TREMOLO_PERIOD;
      getInputBufferSPM(&x[0], &x[1]);
      y[0] = (x[0] * usedArray[*pnt]) >> 15;
      y[1] = (x[1] * usedArray[*pnt]) >> 15;
      setOutputBuffer(y[0], y[1]);
  }

  return 0;
}
