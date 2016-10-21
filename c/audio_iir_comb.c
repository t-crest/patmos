#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ONE_16b 0x7FFF

#define BUFFER_SIZE 32

#define Fs 52083 // Hz

const int COMB_FILTER_ORDER_1PLUS = 2;
const int IIR_BUFFER_LENGTH = 13020; //0.25 seconds
const int CH_LENGTH = 2;

#include "audio.h"
#include "audio.c"

// LOCATION IN LOCAL SCRATCHPAD MEMORY
#define Y_ADDR     0x00000000
#define G_ADDR     ( Y_ADDR      + 2 * sizeof(short) )
#if ( (COMB_FILTER_ORDER_1PLUS % 2) == 0 ) //if it's even
#define DEL_ADDR   ( G_ADDR      + COMB_FILTER_ORDER_1PLUS * sizeof(short) )
#else // if it's odd
#define DEL_ADDR   ( G_ADDR      + COMB_FILTER_ORDER_1PLUS * sizeof(short) + 2 ) //to align with 4-byte word
#endif
#define PNT_ADDR   ( DEL_ADDR    + COMB_FILTER_ORDER_1PLUS * sizeof(int) )
// SPM variables
volatile _SPM short *y               = (volatile _SPM short *)      Y_ADDR; // y[2]: output
volatile _SPM short *g               = (volatile _SPM short *)      G_ADDR; // g[COMB_FILTER_ORDER_1PLUS]: array of gains [... g2, g1, g0]
volatile _SPM int *del               = (volatile _SPM int *)        DEL_ADDR; // del[COMB_FILTER_ORDER_1PLUS]: array of delays [...d2, d1, 0]
volatile _SPM int *pnt               = (volatile _SPM int *)        PNT_ADDR; //pointer indicates last position of fir_buffer
// Externam SRAM variables
volatile short iir_buffer[IIR_BUFFER_LENGTH][CH_LENGTH];

/*
  IIR comb delay:
    -First, input data is stored in current position of buffer
    -The COMB function used is the same as FIR
    -The difference is that, after computing new y, this y value is replaced by the x value on the iir_buffer
*/

int main() {

    setup(0); //for volca

    // enable input and output
    *audioDacEnReg = 1;
    *audioAdcEnReg = 1;

    setInputBufferSize(BUFFER_SIZE);
    setOutputBufferSize(BUFFER_SIZE);

    //set gains: for comb delay:
    g[1] = ONE_16b;       // g0 = 1
    g[0] = ONE_16b * 0.5; // g1 = 0.7
    printf("gains are: %d, %d\n", g[1], g[0]);

    //set delays:
    del[1] = 0; // always d0 = 0
    del[0] = IIR_BUFFER_LENGTH - 1; // d1 = as long as delay buffer

    //CPU cycles stuff
    int CPUcycles[100] = {0};
    int cpu_pnt = 0;

    *pnt = IIR_BUFFER_LENGTH - 1; // start on top
    while(*keyReg != 3) {
        //first, read sample
        getInputBuffer((short *)&iir_buffer[*pnt][0], (short *)&iir_buffer[*pnt][1]);
        //calculate IIR comb filter
        iir_comb(CH_LENGTH, IIR_BUFFER_LENGTH, COMB_FILTER_ORDER_1PLUS, pnt, iir_buffer, y, g, del);
        //output sample
        setOutputBuffer(y[0], y[1]);
        //replace content on buffer
        iir_buffer[*pnt][0] = y[0];
        iir_buffer[*pnt][1] = y[1];
        //update pointer
        if(*pnt == 0) {
            *pnt = IIR_BUFFER_LENGTH - 1;
        }
        else {
            *pnt = *pnt - 1;
        }

        //store CPU Cycles
        CPUcycles[cpu_pnt] = get_cpu_cycles();
        if(cpu_pnt == 100) {
            break;
        }
        else {
            cpu_pnt++;
        }

    }

    //print CPU cycle time
    for(int i=1; i<100; i++) {
        printf("%d\n", (CPUcycles[i]-CPUcycles[i-1]));
    }

    return 0;
}
