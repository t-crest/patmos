#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ONE_16b 0x8000 //0x7FFF

#define BUFFER_SIZE 32

#define Fs 52083 // Hz

#define FILTER_ORDER_1PLUS 2

/* Vibrato:
     -Buffer Length sets the amount of vibrato: amplitude of sin
     -Vibrato period sets the rate of the vibrato: period of sin
*/

#define FIR_BUFFER_LENGTH 520 // for a delay of up to 10 52083 / 520 = 10 ms
//#define FIR_BUFFER_LENGTH 20000
#define VIBRATO_PERIOD 20000 //almost half second

#include "audio.h"
#include "audio.c"


// LOCATION IN LOCAL SCRATCHPAD MEMORY
#define ACCUM_ADDR 0x00000000
#define Y_ADDR     ( ACCUM_ADDR  + 2 * sizeof(int) )
#define G_ADDR     ( Y_ADDR      + 2 * sizeof(short) )
#define DEL_ADDR   ( G_ADDR      + FILTER_ORDER_1PLUS * sizeof(short) )
#define PNT_ADDR   ( DEL_ADDR    + FILTER_ORDER_1PLUS * sizeof(int) )
#define V_PNT_ADDR ( PNT_ADDR    + sizeof(int) )

// LOCATION IN EXTERNAL XRAM
#define FIR_BUFFER_ADDR 0x00020000

volatile _SPM int *accum             = (volatile _SPM int *)        ACCUM_ADDR;
volatile _SPM short *y               = (volatile _SPM short *)      Y_ADDR; // y[2]: output
volatile _SPM short *g               = (volatile _SPM short *)      G_ADDR; // g[FILTER_ORDER_1PLUS]: array of gains [... g2, g1, g0]
volatile _SPM int *del               = (volatile _SPM int *)        DEL_ADDR; // del[FILTER_ORDER_1PLUS]: array of delays [...d2, d1, 0]
volatile _SPM int *pnt               = (volatile _SPM int *)        PNT_ADDR; //pointer indicates last position of fir_buffer
volatile _SPM int *v_pnt             = (volatile _SPM int *)        V_PNT_ADDR; //pointer for vibrato sin array

//volatile _SPM short (*fir_buffer)[2] = (volatile _SPM short (*)[2]) FIR_BUFFER_ADDR; // fir_buffer[FIR_BUFFER_LENGTH][2]
volatile short fir_buffer[FIR_BUFFER_LENGTH][2];
int sin_array[VIBRATO_PERIOD];

int storeSin(int *sinArray, int SIZE, int OFFSET, int AMP) {
    for (int i = 0; i < SIZE; i++) {
        sinArray[i] = OFFSET + AMP*sin(2.0*M_PI* i / SIZE);
    }
    printf("sin array storage done\n");

    return 0;
}


int main() {

    setup();

    // enable input and output
    *audioDacEnReg = 1;
    *audioAdcEnReg = 1;

    setInputBufferSize(BUFFER_SIZE);
    setOutputBufferSize(BUFFER_SIZE);

    printf("addresses at scratchpad mem: accum: 0x%x, y: 0x%x, g: 0x%x, del: 0x%x, pnt: 0x%x\n", (int)accum, (int)y, (int)g, (int)del, (int)pnt);
    printf("address of buffer: 0x%x\n", (int)&fir_buffer);


    storeSin(sin_array, VIBRATO_PERIOD, ((FIR_BUFFER_LENGTH-1)*0.5), ((FIR_BUFFER_LENGTH-1)*0.5));

    //set gains: for VIBRATO: only 1st delayed signal
    g[1] = 0; // g0 = 0;
    g[0] = ONE_16b-1; // g1 = 1;

    //set delays: first, fixed:
    del[1] = 0; // always d0 = 0

    *pnt = FIR_BUFFER_LENGTH - 1; //start on top
    while(*keyReg != 3) {
        //update delay
        del[0] = sin_array[*v_pnt];
        *v_pnt = (*v_pnt + 1) % VIBRATO_PERIOD;
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
    }

    return 0;
}
