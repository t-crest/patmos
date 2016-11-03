#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "audio.h"
#include "audio.c"

/*
  Overdrive & Distortion
*/

int main() {

    setup(0); // 1 for guitar

    // enable input and output
    *audioDacEnReg = 1;
    *audioAdcEnReg = 1;

    setInputBufferSize(BUFFER_SIZE);
    setOutputBufferSize(BUFFER_SIZE);

    int type; //to choose between overdrive and distortion

    struct Overdrive od1;
    struct Overdrive *od1Pnt = &od1;
    struct AudioFX *od1FXPnt = (struct AudioFX *) od1Pnt;
    int OD_ALLOC_AMOUNT = alloc_overdrive_vars(od1Pnt, 0);

    struct Distortion dist1;
    struct Distortion *dist1Pnt = &dist1;
    struct AudioFX *dist1FXPnt = (struct AudioFX *) dist1Pnt;
    float distAmount = 0.9;
    int DIST_ALLOC_AMOUNT = alloc_distortion_vars(dist1Pnt, 0, distAmount);

    printf("Press KEY0 for OVERDRIVE and KEY1 for DISTORTION\n");
    while(*keyReg == 15);
    if(*keyReg == 14) {
        type = 0; //overdrive
        printf("Chosen: OVERDRIVE. Play!\n");
    }
    if(*keyReg == 13) {
        type = 1; //distortion
        printf("Chosen: DISTORTION. Play!\n");
    }

    //CPU cycles stuff
    //int CPUcycles[1000] = {0};
    //int cpu_pnt = 0;

    while(*keyReg != 3) {
        if(type==0) {
            audioIn(od1FXPnt);
            audio_overdrive(od1Pnt);
            audioOut(od1FXPnt);
        }
        else { //type==1
            audioIn(dist1FXPnt);
            audio_distortion(dist1Pnt);
            audioOut(dist1FXPnt);
        }
        /*
        //store CPU Cycles
        CPUcycles[cpu_pnt] = get_cpu_cycles();
        cpu_pnt++;
        if(cpu_pnt == 1000) {
            break;
        }
        */
    }
    /*
    //print CPU cycle time
    for(int i=1; i<1000; i++) {
        printf("%d\n", (CPUcycles[i]-CPUcycles[i-1]));
    }
    */
    return 0;
}



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
    /*
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
    */
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

    /*
    while(*keyReg != 3) {
        getInputBufferSPM(&x[0], &x[1]);
        //fuzz(x, y, accum, K, KonePlus, shiftLeft_const);
        overdrive(x, y, accum);
        //distortion(x, y, accum);
        //printf("for input %d, %d, output is %d, %d\n", x[0], x[1], y[0], y[1]);
        setOutputBuffer(y[0], y[1]);

    }
    */
    /*
    //print CPU cycle time
    for(int i=1; i<300; i++) {
    printf("%d\n", (CPUcycles[i]-CPUcycles[i-1]));
    }
    */
