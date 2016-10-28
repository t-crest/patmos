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

#define FIR_BUFFER_LENGTH 150 // for a delay of up to 10 150*10e3 / 52083 = ms

#include "audio.h"
#include "audio.c"



#define X_ADDR      LAST_SPM_POS
#define Y_ADDR      ( X_ADDR     + 2 * sizeof(short) )

volatile _SPM short *x               = (volatile _SPM short *)      X_ADDR; // x[2]: input
volatile _SPM short *y               = (volatile _SPM short *)      Y_ADDR; // y[2]: output

//variables in external SRAM
volatile short fir_buffer[FIR_BUFFER_LENGTH][2];
//decide vibrato period here:
const int VIBRATO_PERIOD = Fs/4;
//for sinus:
int usedSin[VIBRATO_PERIOD];
short fracArray[VIBRATO_PERIOD];

int main() {

    setup(0); //for guitar

    // enable input and output
    *audioDacEnReg = 1;
    *audioAdcEnReg = 1;

    setInputBufferSize(BUFFER_SIZE);
    setOutputBufferSize(BUFFER_SIZE);

    //old way:
    storeSinInterpol(usedSin, fracArray, VIBRATO_PERIOD, (FIR_BUFFER_LENGTH*0.5), ((FIR_BUFFER_LENGTH-4)*0.5));

    //CPU cycles stuff
    //int CPUcycles[1000] = {0};

    //initialise vibrato variables
    *pnt = FIR_BUFFER_LENGTH - 1; //start on top
    *v_pnt = 0;
    while(*keyReg != 3) {
        getInputBufferSPM(&x[0], &x[1]);
        audio_vibrato(VIBRATO_PERIOD, x, y);
        setOutputBuffer(y[0], y[1]);
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
