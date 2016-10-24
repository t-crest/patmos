#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ONE_16b 0x7FFF

#define BUFFER_SIZE 32

#define Fs 52083 // Hz

#define AP_BUFFER_LENGTH 200

#include "audio.h"
#include "audio.c"

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

    setup(0); //for volca

    // enable input and output
    *audioDacEnReg = 1;
    *audioAdcEnReg = 1;

    setInputBufferSize(BUFFER_SIZE);
    setOutputBufferSize(BUFFER_SIZE);

    //set gain:
    *g = ONE_16b * 0.5;

    //CPU cycles stuff
    //int CPUcycles[300] = {0};
    //int cpu_pnt = 0;


    *pnt = AP_BUFFER_LENGTH - 1; // start on top
    int printa = 0;
    while(*keyReg != 3) {
        if( (*pnt == 100) || (*pnt == 101) ) {
            printa = 1;
        }
        else {
            printa = 0;
        }
        //first, read sample
        getInputBuffer((short *)&x[0], (short *)&x[1]);
        //calculate IIR comb filter
        allpass_comb(AP_BUFFER_LENGTH, pnt, ap_buffer, x, y, g, printa);
        if(printa == 1) {
            printf("for inputs %d, %d at pnt %d, outputs are %d, %d\n", x[0], x[1], *pnt, y[0], y[1]);
        }
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
        if(cpu_pnt == 300) {
            break;
        }
        else {
            cpu_pnt++;
        }
        */
    }
    /*
    //print CPU cycle time
    for(int i=1; i<300; i++) {
        printf("%d\n", (CPUcycles[i]-CPUcycles[i-1]));
    }
    */
    return 0;
}
