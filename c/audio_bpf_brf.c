#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <math.h>

#define ONE_16b 0x7FFF

#define BUFFER_SIZE 128

#define Fs 52083 // Hz

#define FILTER_ORDER_1PLUS 3

#include "audio.h"
#include "audio.c"

/* Band-Pass/Band-Reject Filters:
     -Based on 2nd order all-pass filters
     -The implementation has a similar structure to a comb filter of 1st order:
      the direct audio is added/subtracted from the all-passed audio
     -The all-pass branch can be implemented as an IIR filter
*/


// LOCATION IN SCRATCHPAD MEMORY
#define ACCUM_ADDR  0x00000000
#define B_ADDR      ( ACCUM_ADDR  + 2 * sizeof(int) )

#if ( (FILTER_ORDER_1PLUS % 2) == 0 ) //if it's even
#define A_ADDR      ( B_ADDR      + FILTER_ORDER_1PLUS * sizeof(short) )
#define X_FILT_ADDR ( A_ADDR      + FILTER_ORDER_1PLUS * sizeof(short) )
#else //if it's odd, align with 4-byte word:
#define A_ADDR      ( B_ADDR      + FILTER_ORDER_1PLUS * sizeof(short) + 2 )
#define X_FILT_ADDR ( A_ADDR      + FILTER_ORDER_1PLUS * sizeof(short) + 2 )
#endif

#define Y_FILT_ADDR ( X_FILT_ADDR + 2 * FILTER_ORDER_1PLUS * sizeof(short) )
#define PNT_ADDR    ( Y_FILT_ADDR + 2 * FILTER_ORDER_1PLUS * sizeof(short) )
#define SFTLFT_ADDR ( PNT_ADDR    + sizeof(int) )
#define OUTREG_ADDR ( SFTLFT_ADDR + sizeof(int) )

//to have fixed-point multiplication:
volatile _SPM int *accum           = (volatile _SPM int *)        ACCUM_ADDR;
volatile _SPM short *B             = (volatile _SPM short *)      B_ADDR; // [b2, b1, b0]
volatile _SPM short *A             = (volatile _SPM short *)      A_ADDR; // [a2, a1,  1]
volatile _SPM short (*x_filter)[2] = (volatile _SPM short (*)[2]) X_FILT_ADDR; // x_filter[FILTER_ORDER_1PLUS][2] = {0};
volatile _SPM short (*y_filter)[2] = (volatile _SPM short (*)[2]) Y_FILT_ADDR; // y_filter[FILTER_ORDER_1PLUS][2] = {0};
volatile _SPM int *pnt             = (volatile _SPM int *)        PNT_ADDR; // pointer indicates last position of x_filter buffer
volatile _SPM int *shiftLeft       = (volatile _SPM int *)        SFTLFT_ADDR; //shift left amount;
volatile _SPM int *outputReg       = (volatile _SPM int *)        OUTREG_ADDR; //stores the output data

int main() {

    setup(0); //for volca

    // enable input and output
    *audioDacEnReg = 1;
    *audioAdcEnReg = 1;

    setInputBufferSize(BUFFER_SIZE);
    setOutputBufferSize(BUFFER_SIZE);

    //shift left amount starts at 0
    *shiftLeft = 0;

    printf("addresses at scratchpad mem: accum: 0x%x, B: 0x%x, A: 0x%x, x_filter: 0x%x, y_filter: 0x%x, pnt: 0x%x, shiftLeft: 0x%x, outputReg: 0x%x\n", (int)accum, (int)B, (int)A, (int)x_filter, (int)y_filter, (int)pnt, (int)shiftLeft, (int)outputReg);

    if(FILTER_ORDER_1PLUS == 2) { //1st order
        printf("Press KEY0 for high-pass filter, KEY1 for low-pass filter\n");
    }
    if(FILTER_ORDER_1PLUS == 3) { // 2nd order
        printf("Press KEY0 for band-pass filter, KEY1 for band-reject filter\n");
    }
    int isBandPass;
    while(*keyReg == 15);
    if(*keyReg == 14) { // BAND-PASS
        // calculate all-pass filter coefficients
        filter_coeff_bp_br(FILTER_ORDER_1PLUS, B, A, 1000, 300, shiftLeft, 0);
        isBandPass = 1;
        if(FILTER_ORDER_1PLUS == 3) { // 2nd order
            printf("Band-Pass ready, PLAY!\n");
        }
        else { //1st order
            printf("High-Pass ready, PLAY!\n");
        }
    }
    if(*keyReg == 13) { // BAND-REJECT
        // calculate all-pass filter coefficients
        filter_coeff_bp_br(FILTER_ORDER_1PLUS, B, A, 500, 2000, shiftLeft, 0);
        isBandPass = 0;
        if(FILTER_ORDER_1PLUS == 3) { // 2nd order
            printf("Band-Reject ready, PLAY!\n");
        }
        else { // 1st order
            printf("Low-Pass ready, PLAY!\n");
        }
    }

    //CPU cycles stuff
    //int CPUcycles[300] = {0};
    //int cpu_pnt = 0;

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
        if(FILTER_ORDER_1PLUS == 2) { //1st order
            filterIIR_1st(*pnt, x_filter, y_filter, accum, B, A, *shiftLeft);
        }
        else { //2nd order
            filterIIR_2nd(*pnt, x_filter, y_filter, accum, B, A, *shiftLeft);
        }
        //set output
        if(isBandPass == 1) {
            outputReg[0] = ( x_filter[*pnt][0] - y_filter[*pnt][0] ) >> 1;
            outputReg[1] = ( x_filter[*pnt][1] - y_filter[*pnt][1] ) >> 1;
        }
        else {
            outputReg[0] = ( x_filter[*pnt][0] + y_filter[*pnt][0] ) >> 1;
            outputReg[1] = ( x_filter[*pnt][1] + y_filter[*pnt][1] ) >> 1;
        }
        setOutputBuffer((short)outputReg[0], (short)outputReg[1]);
        /*
        //store CPU Cycles
        CPUcycles[cpu_pnt] = get_cpu_cycles();
        cpu_pnt++;
        if(cpu_pnt == 300) {
            break;
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
