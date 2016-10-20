#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ONE_16b 0x8000 //0x7FFF

#define OD_THRESHOLD 0x2AAB // set experimentally (usually 0x2AAB = ONE_16b * 1/3)

#define BUFFER_SIZE 32

#define Fs 52083 // Hz

#include "audio.h"
#include "audio.c"

#define X_ADDR 0x00000000
#define Y_ADDR ( X_ADDR + 2 * sizeof(short) )

const int CH_LENGTH = 2;
const int MACLAURIN_ORDER_1MINUS = 5;

volatile _SPM short *x = (volatile _SPM short *) X_ADDR; // input audio
volatile _SPM short *y = (volatile _SPM short *) Y_ADDR; // output audio


int main() {

    setup(1); //for guitar

  // enable input and output
  *audioDacEnReg = 1;
  *audioAdcEnReg = 1;

  setInputBufferSize(BUFFER_SIZE);
  setOutputBufferSize(BUFFER_SIZE);

  printf("Play!\n");

  /*
  printf("some examples...\n");

  x[0] = 0x4000; //0.5
  x[1] = 0x3500; //0.4140625
  printf("inputs: x[0]=%d, x[1]=%d\n", x[0], x[1]);
  distortion(CH_LENGTH, MACLAURIN_ORDER_1MINUS, x, y);
  printf("RESULTA: \n");
  printf("y[0] = %d\n", y[0]);
  printf("y[1] = %d\n", y[1]);
  x[0] = -1 * 0x3000; //-0.375
  x[1] = -1 * 0x2A00; //-0.328125
  printf("inputs: x[0]=%d, x[1]=%d\n", x[0], x[1]);
  distortion(CH_LENGTH, MACLAURIN_ORDER_1MINUS, x, y);
  printf("RESULTADOS: \n");
  printf("y[0] = %d\n", y[0]);
  printf("y[1] = %d\n", y[1]);
  */

  /*
  printf("overdrive test...\n");
  x[0] = -0x5555; // 2/3
  x[1] = -0x5556;
  printf("input is %d, %d\n", x[0], x[1]);
  overdrive(CH_LENGTH, x, y, OD_THRESHOLD);
  printf("output is %d, %d\n", y[0], y[1]);
  x[0] = -0x5557;
  x[1] = -0x5558;
  printf("input is %d, %d\n", x[0], x[1]);
  overdrive(CH_LENGTH, x, y, OD_THRESHOLD);
  printf("output is %d, %d\n", y[0], y[1]);
  */

  const float amount = 0.8;
  const int K = ( (2*amount)/(1-amount) ) * pow(2,15);
  const int KonePlus = ( (2*amount)/(1-amount) + 1 ) * pow(2,15);
  printf("values: amount=%f, K=%d, KonePlus=%d\n", amount, K, KonePlus);

  //CPU cycles stuff
  int CPUcycles[300] = {0};
  int cpu_pnt = 0;

  while(*keyReg != 3) {
      getInputBufferSPM(&x[0], &x[1]);
      fuzz(CH_LENGTH, x, y, K, KonePlus);
      //overdrive(CH_LENGTH, x, y, OD_THRESHOLD);
      //distortion(CH_LENGTH, MACLAURIN_ORDER_1MINUS, x, y);
      //printf("for input %d, %d, output is %d, %d\n", x[0], x[1], y[0], y[1]);
      setOutputBuffer(y[0], y[1]);

      //store CPU Cycles
      CPUcycles[cpu_pnt] = get_cpu_cycles();
      cpu_pnt++;
      if(cpu_pnt == 300) {
          break;
      }

  }

  //print CPU cycle time
  for(int i=1; i<300; i++) {
      printf("%d\n", (CPUcycles[i]-CPUcycles[i-1]));
  }

  return 0;
}
