#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <math.h>

#define ONE_16b 0x8000 //0x7FFF

#define BUFFER_SIZE 32

#define Fs 52083 // Hz

#define FILTER_ORDER_1PLUS 3 //FOR LPF filter of noise
#define COMB_FILTER_ORDER_1PLUS 3 // FOR CHORUS comb filter


/* Chorus:
     -Implemented as a 2nd order FIR comb filter
     -Modulation of copied signals are noise signals, low-pass filtered for enhancement
*/

#define FIR_BUFFER_LENGTH 2083 // for a delay of up to 25 52083 / 2083 = 25 ms
#define CHORUS_1_LENGTH 20000
#define CHORUS_2_LENGTH 14000

#include "audio.h"
#include "audio.c"


// LOCATION IN LOCAL SCRATCHPAD MEMORY
#define ACCUM_ADDR  0x00000000
#define Y_ADDR      ( ACCUM_ADDR  + 2 * sizeof(int) )
#define G_ADDR      ( Y_ADDR      + 2 * sizeof(short) )

#if ( (COMB_FILTER_ORDER_1PLUS % 2) == 0 ) //if it's even
#define DEL_ADDR    ( G_ADDR      + COMB_FILTER_ORDER_1PLUS * sizeof(short) )
#else // if it's odd
#define DEL_ADDR    ( G_ADDR      + COMB_FILTER_ORDER_1PLUS * sizeof(short) + 2 ) //to align with 4-byte word
#endif

#define PNT_ADDR    ( DEL_ADDR    + COMB_FILTER_ORDER_1PLUS * sizeof(int) )

#define CH_ACC_ADDR ( PNT_ADDR    + sizeof(int) )
#define B_ADDR      ( CH_ACC_ADDR + 2 * sizeof(int) )

#if ( (FILTER_ORDER_1PLUS % 2) == 0 ) //if it's even
#define A_ADDR      ( B_ADDR      + FILTER_ORDER_1PLUS * sizeof(short) )
#define CH_X_ADDR   ( A_ADDR      + FILTER_ORDER_1PLUS * sizeof(short) )
#else  // if it's odd
#define A_ADDR      ( B_ADDR      + FILTER_ORDER_1PLUS * sizeof(short) + 2 ) //to align with 4-byte word
#define CH_X_ADDR   ( A_ADDR      + FILTER_ORDER_1PLUS * sizeof(short) + 2 ) //to align with 4-byte word
#endif

#if ( (((COMB_FILTER_ORDER_1PLUS-1) * FILTER_ORDER_1PLUS) % 2) == 0 ) //if it's even
#define CH_Y_ADDR   ( CH_X_ADDR   + (COMB_FILTER_ORDER_1PLUS-1) * FILTER_ORDER_1PLUS * sizeof(short) )
#define CH_PNT_ADDR ( CH_Y_ADDR   + (COMB_FILTER_ORDER_1PLUS-1) * FILTER_ORDER_1PLUS * sizeof(short) )
#else // if it's odd: align with 4-byte word
#define CH_Y_ADDR   ( CH_X_ADDR   + (COMB_FILTER_ORDER_1PLUS-1) * FILTER_ORDER_1PLUS * sizeof(short) + 2 )
#define CH_PNT_ADDR ( CH_Y_ADDR   + (COMB_FILTER_ORDER_1PLUS-1) * FILTER_ORDER_1PLUS * sizeof(short) + 2 )
#endif

#define SFTLFT_ADDR ( CH_PNT_ADDR + sizeof(int) )




volatile _SPM int *accum             = (volatile _SPM int *)        ACCUM_ADDR;
volatile _SPM short *y               = (volatile _SPM short *)      Y_ADDR; // y[2]: output
volatile _SPM short *g               = (volatile _SPM short *)      G_ADDR; // g[FILTER_ORDER_1PLUS]: array of gains [... g2, g1, g0]
volatile _SPM int *del               = (volatile _SPM int *)        DEL_ADDR; // del[FILTER_ORDER_1PLUS]: array of delays [...d2, d1, 0]
volatile _SPM int *pnt               = (volatile _SPM int *)        PNT_ADDR; //pointer indicates last position of fir_buffer
//stuff for LPF noise signal
float K;
volatile _SPM int *ch_accum                            = (volatile _SPM int *)                                CH_ACC_ADDR;
volatile _SPM short *B                                 = (volatile _SPM short *)                              B_ADDR; // [b2, b1, b0]
volatile _SPM short *A                                 = (volatile _SPM short *)                              A_ADDR; // [a2, a1,  1]
volatile _SPM short (*ch_x)[COMB_FILTER_ORDER_1PLUS-1] = (volatile _SPM short (*)[COMB_FILTER_ORDER_1PLUS-1]) CH_X_ADDR;
volatile _SPM short (*ch_y)[COMB_FILTER_ORDER_1PLUS-1] = (volatile _SPM short (*)[COMB_FILTER_ORDER_1PLUS-1]) CH_Y_ADDR;
volatile _SPM int *ch_pnt                              = (volatile _SPM int *)                                CH_PNT_ADDR;
volatile _SPM int *shiftLeft                           = (volatile _SPM int *)                                SFTLFT_ADDR; //shift left amount;

//volatile _SPM short (*fir_buffer)[2] = (volatile _SPM short (*)[2]) FIR_BUFFER_ADDR; // fir_buffer[FIR_BUFFER_LENGTH][2]
volatile short fir_buffer[FIR_BUFFER_LENGTH][2];


int main() {

    setup();

    // enable input and output
    *audioDacEnReg = 1;
    *audioAdcEnReg = 1;

    setInputBufferSize(BUFFER_SIZE);
    setOutputBufferSize(BUFFER_SIZE);

    *shiftLeft = 0;

    printf("Addresses: accum: %d, y: %d, g: %d, del: %d, pnt: %d, ch_accum: %d, B: %d, A: %d, ch_x: %d, ch_y: %d, ch_pnt: %d, shiftLeft: %d\n", (int)accum, (int)y, (int)g, (int)del, (int)pnt, (int)ch_accum, (int)B, (int)A, (int)ch_x, (int)ch_y, (int)ch_pnt, (int)shiftLeft);

    //FILTER COEFFICIENTS FOR NOISE SIGNAL
    calc_filter_coeff(B, A, K, 600, 0.707, shiftLeft, 0); //0 for LPF

    //gains
    g[2] = ONE_16b * 0.8; //g0
    g[1] = ONE_16b * 0.6; //g1
    g[0] = ONE_16b * 0.6; //g2

    //delays
    del[2] = 0; //always d0 = 0

    //CPU cycles stuff
    //int CPUcycles[1000] = {0};
    //int cpu_pnt = 0;


    *pnt = FIR_BUFFER_LENGTH - 1; //start on top
    *ch_pnt = 0;
    while(*keyReg != 3) {
        //UPDATE DELAYS: NOISE + LPF
        for(int i=0; i<(COMB_FILTER_ORDER_1PLUS-1); i++) {
            ch_x[*ch_pnt][i] = rand() % FIR_BUFFER_LENGTH;
            //printf("ch_x[%d][%d]=%d\n", *ch_pnt, i, ch_x[*ch_pnt][i]);
            filterIIR(ch_pnt, ch_x, ch_y, ch_accum, B, A, *shiftLeft);
            //printf("ch_y[%d][%d]=%d\n", *ch_pnt, i, ch_y[*ch_pnt][i]);
            del[i] = ch_y[*ch_pnt][i];
        }
        *ch_pnt = (*ch_pnt+1) % FILTER_ORDER_1PLUS;
        //printf("del[1]: %d, del[0]: %d\n", del[1], del[0]);

        //first, read sample
        getInputBuffer((short *)&fir_buffer[*pnt][0], (short *)&fir_buffer[*pnt][1]);
        //calculate FIR comb filter
        fir_comb(pnt, fir_buffer, y, accum, g, del);
        //output sample
        setOutputBuffer(y[0], y[1]);
        //update pointer
        if(*pnt == 0) {
            *pnt = FIR_BUFFER_LENGTH - 1;
        }
        else {
            *pnt = *pnt - 1;
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
    for(int i=1; i<1000; i++) {
        printf("%d\n", (CPUcycles[i]-CPUcycles[i-1]));
    }
    */

    return 0;
}
