#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define AP_BUFFER_LENGTH 2000

#include "libaudio/audio.h"
#include "libaudio/audio.c"

// LOCATION IN LOCAL SCRATCHPAD MEMORY
#define X_ADDR     0x00000000
#define Y_ADDR     ( X_ADDR      + 2 * sizeof(short) )
#define G_ADDR     ( Y_ADDR      + 2 * sizeof(short) )
#define PNT_ADDR   ( G_ADDR      + sizeof(short) + 2)
// SPM variables
volatile _SPM short *x               = (volatile _SPM short *)      X_ADDR; // x[2]: input
volatile _SPM short *y               = (volatile _SPM short *)      Y_ADDR; // y[2]: output
volatile _SPM short *g               = (volatile _SPM short *)      G_ADDR; // g: gain
volatile _SPM int *pnt               = (volatile _SPM int *)        PNT_ADDR; //buffer pointer
// Externam SRAM variables
volatile short ap_buffer[AP_BUFFER_LENGTH][2];

/*
  ALL-PASS comb delay:
*/


int main() {

    #if GUITAR == 1
    setup(1); //for guitar
    #else
    setup(0); //for volca
    #endif

    // enable input and output
    *audioDacEnReg = 1;
    *audioAdcEnReg = 1;

    setInputBufferSize(BUFFER_SIZE);
    setOutputBufferSize(BUFFER_SIZE);

    //set gain:
    *g = ONE_16b * 0.5;

    //CPU cycles stuff
    //int CPUcycles[1000] = {0};
    //int cpu_pnt = 0;


    *pnt = AP_BUFFER_LENGTH - 1; // start on top
    while(*keyReg != 3) {
        //first, read sample
        getInputBufferSPM(&x[0], &x[1]);
        //calculate IIR comb filter
        allpass_comb(AP_BUFFER_LENGTH, pnt, ap_buffer, x, y, g);
        //output sample
        setOutputBuffer(y[0], y[1]);
        //update pointer
        if(*pnt == 0) {
            *pnt = AP_BUFFER_LENGTH - 1;
        }
        else {
            *pnt = *pnt - 1;
        }
        /*
        //store CPU Cycles
        CPUcycles[cpu_pnt] = get_cpu_cycles();
        if(cpu_pnt == 1000) {
            break;
        }
        else {
            cpu_pnt++;
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
