#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ONE_16b 0x7FFF

#define BUFFER_SIZE 128

#define Fs 52083 // Hz

#define COMB_FILTER_ORDER_1PLUS 2

/* Vibrato:
     -Buffer Length sets the amount of vibrato: amplitude of sin
     -Vibrato period sets the rate of the vibrato: period of sin
*/

#define FIR_BUFFER_LENGTH 150 // for a delay of up to 10 150*10e3 / 52083 =

#include "audio.h"
#include "audio.c"


// LOCATION IN LOCAL SCRATCHPAD MEMORY
#define ACCUM_ADDR 0x00000000
#define Y_ADDR     ( ACCUM_ADDR  + 2 * sizeof(int) )
#define G_ADDR     ( Y_ADDR      + 2 * sizeof(short) )

#if ( (COMB_FILTER_ORDER_1PLUS % 2) == 0 ) //if it's even
#define DEL_ADDR   ( G_ADDR      + COMB_FILTER_ORDER_1PLUS * sizeof(short) )
#else // if it's odd
#define DEL_ADDR   ( G_ADDR      + COMB_FILTER_ORDER_1PLUS * sizeof(short) + 2 ) //to align with 4-byte word
#endif

#define PNT_ADDR   ( DEL_ADDR    + COMB_FILTER_ORDER_1PLUS * sizeof(int) )
#define V_PNT_ADDR ( PNT_ADDR    + sizeof(int) )

// LOCATION IN EXTERNAL XRAM
#define FIR_BUFFER_ADDR 0x00020000

volatile _SPM int *accum             = (volatile _SPM int *)        ACCUM_ADDR;
volatile _SPM short *y               = (volatile _SPM short *)      Y_ADDR; // y[2]: output
volatile _SPM short *g               = (volatile _SPM short *)      G_ADDR; // g[COMB_FILTER_ORDER_1PLUS]: array of gains [... g2, g1, g0]
volatile _SPM int *del               = (volatile _SPM int *)        DEL_ADDR; // del[COMB_FILTER_ORDER_1PLUS]: array of delays [...d2, d1, 0]
volatile _SPM int *pnt               = (volatile _SPM int *)        PNT_ADDR; //pointer indicates last position of fir_buffer
volatile _SPM int *v_pnt             = (volatile _SPM int *)        V_PNT_ADDR; //pointer for vibrato sin array

//variables in external SRAM
volatile short fir_buffer[FIR_BUFFER_LENGTH][2];
//for sinus:
int sinArray[Fs]; //maximum period: 1 secod
//decide vibrato period here:
const int VIBRATO_PERIOD = Fs/4;
int usedArray[VIBRATO_PERIOD];

int main() {

    setup(1); //for guitar

    // enable input and output
    *audioDacEnReg = 1;
    *audioAdcEnReg = 1;

    setInputBufferSize(BUFFER_SIZE);
    setOutputBufferSize(BUFFER_SIZE);

    /*
    //store sin: 1 second betwen -1 and 1
    storeSin(sinArray, Fs, 0, ONE_16b);
    //calculate interpolated array:
    float arrayDivider = (float)Fs/(float)VIBRATO_PERIOD;
    printf("Array Divider is: %f\n", arrayDivider);
    for(int i=0; i<VIBRATO_PERIOD; i++) {
        //offset = 0.5, amplitude = 0.4
        usedArray[i] = (ONE_16b*0.6) + 0.3*sinArray[(int)floor(i*arrayDivider)];
    }
    */


    storeSin(usedArray, VIBRATO_PERIOD, ((FIR_BUFFER_LENGTH-1)*0.5), ((FIR_BUFFER_LENGTH-1)*0.5));

    //set gains: for VIBRATO: only 1st delayed signal
    g[1] = 0; // g0 = 0;
    g[0] = ONE_16b-1; // g1 = 1;

    //set delays: first, fixed:
    del[1] = 0; // always d0 = 0

    //CPU cycles stuff
    //int CPUcycles[1000] = {0};

    *pnt = FIR_BUFFER_LENGTH - 1; //start on top
    *v_pnt = 0;
    while(*keyReg != 3) {
        //update delay
        del[0] = usedArray[*v_pnt];
        *v_pnt = (*v_pnt + 1) % VIBRATO_PERIOD;
        //first, read sample
        getInputBuffer((short *)&fir_buffer[*pnt][0], (short *)&fir_buffer[*pnt][1]);
        //calculate FIR comb filter
        combFilter_1st(FIR_BUFFER_LENGTH, pnt, fir_buffer, y, accum, g, del);
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
        CPUcycles[*v_pnt] = get_cpu_cycles();
        if(*v_pnt == 1000) {
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
