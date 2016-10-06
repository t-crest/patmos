#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <math.h>

#define ONE_16b 0x8000 //0x7FFF

#define BUFFER_SIZE 32

#define Fs 52083 // Hz

#define FILTER_ORDER_1PLUS 3

#include "audio.h"
#include "audio.c"

/* WahWah:
     -Addition of original with band-passed signal
     -SIN Modulation of Fc of BandPass filter (B and A coefficients)
*/

#define WAHWAH_PERIOD 35000
#define WAHWAH_FC_CEN 500
#define WAHWAH_FC_AMP 200
#define WAHWAH_FB     30

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
#define WAHPNT_ADDR ( OUTREG_ADDR + 2 * sizeof(int) )

//to have fixed-point multiplication:
volatile _SPM int *accum           = (volatile _SPM int *)        ACCUM_ADDR;
volatile _SPM short *B             = (volatile _SPM short *)      B_ADDR; // [b2, b1, b0]
volatile _SPM short *A             = (volatile _SPM short *)      A_ADDR; // [a2, a1,  1]
volatile _SPM short (*x_filter)[2] = (volatile _SPM short (*)[2]) X_FILT_ADDR; // x_filter[FILTER_ORDER_1PLUS][2] = {0};
volatile _SPM short (*y_filter)[2] = (volatile _SPM short (*)[2]) Y_FILT_ADDR; // y_filter[FILTER_ORDER_1PLUS][2] = {0};
volatile _SPM int *pnt             = (volatile _SPM int *)        PNT_ADDR; // pointer indicates last position of x_filter buffer
volatile _SPM int *shiftLeft       = (volatile _SPM int *)        SFTLFT_ADDR; //shift left amount;
volatile _SPM int *outputReg       = (volatile _SPM int *)        OUTREG_ADDR; //stores the output data
volatile _SPM int *wah_pnt         = (volatile _SPM int *)        WAHPNT_ADDR; // pointer for modulation array
// array of center frequencies
int FcArray[WAHWAH_PERIOD];
// array of coefficients
short A_array[WAHWAH_PERIOD][FILTER_ORDER_1PLUS];
short B_array[WAHWAH_PERIOD][FILTER_ORDER_1PLUS];


int main() {

    setup();

    // enable input and output
    *audioDacEnReg = 1;
    *audioAdcEnReg = 1;

    setInputBufferSize(BUFFER_SIZE);
    setOutputBufferSize(BUFFER_SIZE);

    //shift left is fixed!!!
    *shiftLeft = 1;


    printf("calculating Fc modulation array...\n");
    //calculate sin array of FCs
    storeSin(FcArray, WAHWAH_PERIOD, WAHWAH_FC_CEN, WAHWAH_FC_AMP);

    // calculate all-pass filter coefficients
    printf("calculating modulation coefficients...\n");
    for(int i=0; i<WAHWAH_PERIOD; i++) {
        filter_coeff_bp_br(B, A, FcArray[i], WAHWAH_FB, shiftLeft, 1);
        B_array[i][2] = B[2];
        B_array[i][1] = B[1];
        B_array[i][0] = B[0];
        A_array[i][1] = A[1];
        A_array[i][0] = A[0];
    }
    printf("calculation of modulation coefficients finished!\n");


    //CPU cycles stuff
    //int CPUcycles[1000] = {0};
    //int cpu_pnt = 0;

    *wah_pnt = 0;
    //first, fill filter buffer
    for(*pnt=0; *pnt<(FILTER_ORDER_1PLUS-1); *pnt++) {
        getInputBufferSPM(&x_filter[*pnt][0], &x_filter[*pnt][1]);
    }
    //when filter buffer is full, compute each output sample
    while(*keyReg != 3) {
        //MOD
        B[2] = B_array[*wah_pnt][2]; //b0
        B[1] = B_array[*wah_pnt][1]; //b1
        // b2 doesnt need to be updated: always 1
        A[1] = A_array[*wah_pnt][1]; //a1
        A[0] = A_array[*wah_pnt][0]; //a2
        *wah_pnt = (*wah_pnt+1) % WAHWAH_PERIOD;
        //increment pointer
        *pnt = (*pnt+1) % FILTER_ORDER_1PLUS;
        //first, read last sample
        getInputBufferSPM(&x_filter[*pnt][0], &x_filter[*pnt][1]);
        //then, calculate filter
        filterIIR(pnt, x_filter, y_filter, accum, B, A, *shiftLeft);
        //set output
        outputReg[0] = ( x_filter[*pnt][0] - y_filter[*pnt][0] ); // >> 1;
        outputReg[1] = ( x_filter[*pnt][1] - y_filter[*pnt][1] ); // >> 1;
        //mix with original: gains are 50-50
        outputReg[0] = (outputReg[0] + x_filter[*pnt][0]) >> 1;
        outputReg[1] = (outputReg[1] + x_filter[*pnt][1]) >> 1;
        setOutputBuffer((short)outputReg[0], (short)outputReg[1]);

        /*
        CPUcycles[cpu_pnt] = get_cpu_cycles();
        cpu_pnt++;
        if(cpu_pnt == 1000) {
            break;
        }
        */
    }

    /*
    for(int i=1; i<1000; i++) {
        printf("%d\n", (CPUcycles[i]-CPUcycles[i-1]));
    }
    */

    return 0;
}
