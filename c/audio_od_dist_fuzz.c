#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ONE_16b 0x7FFF
#define BUFFER_SIZE 128
#define Fs 52083 // Hz

#include "audio.h"
#include "audio.c"

#define X_ADDR     0x00000000
#define Y_ADDR     ( X_ADDR + 2 * sizeof(short) )
#define ACCUM_ADDR ( Y_ADDR + 2 * sizeof(short) )

volatile _SPM short *x   = (volatile _SPM short *) X_ADDR; // input audio
volatile _SPM short *y   = (volatile _SPM short *) Y_ADDR; // output audio
volatile _SPM int *accum = (volatile _SPM int *)   ACCUM_ADDR;


int main() {

    setup(0); // 1 for guitar

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
      x[0] = 0x54E0; // 2/3
      x[1] = 0x5556;
      printf("input is %d, %d (%f, %f)\n", x[0], x[1], ((float)x[0]/pow(2,15)), ((float)x[1]/pow(2,15)));
      overdrive(x, y, accum);
      printf("output is %d, %d (%f, %f)\n", y[0], y[1], ((float)y[0]/pow(2,15)), ((float)y[1]/pow(2,15)));;
      x[0] = 0x5557;
      x[1] = 0x5558;
      printf("input is %d, %d (%f, %f)\n", x[0], x[1], ((float)x[0]/pow(2,15)), ((float)x[1]/pow(2,15)));
      overdrive(x, y, accum);
      printf("output is %d, %d (%f, %f)\n", y[0], y[1], ((float)y[0]/pow(2,15)), ((float)y[1]/pow(2,15)));
    */

    const float amount = 0.9; //works with 0.77
    int K_init = ( (2*amount)/(1-amount) ) * pow(2,15);
    int shiftLeft = 0;
    while(K_init > ONE_16b) {
        shiftLeft++;
        K_init = K_init >> 1;
    }
    const int K = K_init;
    const int KonePlus = (int)( ( (2*amount)/(1-amount) + 1 ) * pow(2,15) ) >> shiftLeft;
    printf("values: amount=%f, shiftLeft=%d, K=%d, KonePlus=%d\n", amount, shiftLeft, K, KonePlus);
    const int shiftLeft_const = shiftLeft;

    /*
      x[0] = 0x0000;
      x[1] = 0x0001;
      fuzz(CH_LENGTH, x, y, K, KonePlus, shiftLeft);
      printf("for inputs %d, %d, results are %d, %d\n", x[0], x[1], y[0], y[1]);
      x[0] = 0x5555;
      x[1] = 0x5566;
      fuzz(CH_LENGTH, x, y, K, KonePlus, shiftLeft);
      printf("for inputs %d, %d, results are %d, %d\n", x[0], x[1], y[0], y[1]);
      x[0] = ONE_16b - 1;
      x[1] = ONE_16b;
      fuzz(CH_LENGTH, x, y, K, KonePlus, shiftLeft);
      printf("for inputs %d, %d, results are %d, %d\n", x[0], x[1], y[0], y[1]);
      x[0] = -(ONE_16b);
      x[1] = -(ONE_16b+1);
      fuzz(CH_LENGTH, x, y, K, KonePlus, shiftLeft);
      printf("for inputs %d, %d, results are %d, %d\n", x[0], x[1], y[0], y[1]);
    */

    //CPU cycles stuff
    //int CPUcycles[300] = {0};
    //int cpu_pnt = 0;


    while(*keyReg != 3) {
        getInputBufferSPM(&x[0], &x[1]);
        //fuzz(x, y, accum, K, KonePlus, shiftLeft_const);
        overdrive(x, y, accum);
        //distortion(x, y, accum);
        //printf("for input %d, %d, output is %d, %d\n", x[0], x[1], y[0], y[1]);
        setOutputBuffer(y[0], y[1]);

    }

    /*
    //print CPU cycle time
    for(int i=1; i<300; i++) {
    printf("%d\n", (CPUcycles[i]-CPUcycles[i-1]));
    }
    */

    return 0;
}
