#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ONE_16b 0x8000 //0x7FFF

#define BUFFER_SIZE 32

#define Fs 52083 // Hz

#define FILTER_ORDER_1PLUS 2

//#define FIR_BUFFER_LENGTH 520 // for a delay of up to 10 52083 / 520 = 10 ms
#define FIR_BUFFER_LENGTH 20000

#include "audio.h"
#include "audio.c"


// LOCATION IN LOCAL SCRATCHPAD MEMORY
#define ACCUM_ADDR 0x00000000
#define Y_ADDR     ( ACCUM_ADDR  + 2 * sizeof(int) )
#define G_ADDR     ( Y_ADDR      + 2 * sizeof(short) )
#define DEL_ADDR   ( G_ADDR      + FILTER_ORDER_1PLUS * sizeof(short) )
#define PNT_ADDR   ( DEL_ADDR    + FILTER_ORDER_1PLUS * sizeof(int) )

// LOCATION IN EXTERNAL XRAM
#define FIR_BUFFER_ADDR 0x00020000

volatile _SPM int *accum             = (volatile _SPM int *)        ACCUM_ADDR;
volatile _SPM short *y               = (volatile _SPM short *)      Y_ADDR; // y[2]: output
volatile _SPM short *g               = (volatile _SPM short *)      G_ADDR; // g[FILTER_ORDER_1PLUS]: array of gains [... g2, g1, g0]
volatile _SPM int *del               = (volatile _SPM int *)        DEL_ADDR; // del[FILTER_ORDER_1PLUS]: array of delays [...d2, d1, 0]
volatile _SPM int *pnt               = (volatile _SPM int *)        PNT_ADDR; //pointer indicates last position of fir_buffer

//volatile _SPM short (*fir_buffer)[2] = (volatile _SPM short (*)[2]) FIR_BUFFER_ADDR; // fir_buffer[FIR_BUFFER_LENGTH][2]
volatile _SPM short fir_buffer[FIR_BUFFER_LENGTH][2];

int fir_comb(volatile _SPM int *pnt, volatile _SPM short (*fir_buffer)[2], volatile _SPM short *y, volatile _SPM int *accum, volatile _SPM short *g, volatile _SPM int *del) {

  y[0] = fir_buffer[*pnt][0];
  y[1] = fir_buffer[*pnt][1];
  return 0;
}


int main() {

  setup();

  // enable input and output
  *audioDacEnReg = 1;
  *audioAdcEnReg = 1;

  setInputBufferSize(BUFFER_SIZE);
  setOutputBufferSize(BUFFER_SIZE);

  //set gains: for VIBRATO: only 1st delayed signal
  //g[1] = 0; // g0 = 0;
  //g[0] = ONE_16b-1; // g1 = 1;
  g[1] = ONE_16b*0.5; //g0
  g[0] = ONE_16b*0.5; //g1

  //set delays: first, fixed:
  del[1] = 0; // always d0 = 0
  del[0] = FIR_BUFFER_LENGTH - 1; // d1

  *pnt = 0;
  while(*keyReg != 3) {
    *pnt = (*pnt+1) % FIR_BUFFER_LENGTH;
    //first, read sample
    getInputBufferSPM(&fir_buffer[*pnt][0], &fir_buffer[*pnt][1]);
    fir_comb(pnt, fir_buffer, y, accum, g, del);
    setOutputBuffer(y[0], y[1]);
  }

  return 0;
}
