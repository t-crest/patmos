#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ONE_16b 0x7FFF
#define BUFFER_SIZE 128
#define Fs 52083 // Hz

/* Vibrato:
     -Buffer Length sets the amount of vibrato: amplitude of sin
     -Vibrato period sets the rate of the vibrato: period of sin
*/

#define VIBR_BUF_LEN 150 // for a delay of up to 10 150*10e3 / 52083 = ms
const int VIBR_P = Fs/4;

#include "audio.h"
#include "audio.c"


//SPM addresses
#define X_ADDR      0x00000000
#define Y_ADDR      ( X_ADDR     + 2 * sizeof(short) )
#define LAST_ADDR   ( Y_ADDR     + 2 * sizeof(short) )
//SPM vars
volatile _SPM short *x               = (volatile _SPM short *)      X_ADDR; // x[2]: input
volatile _SPM short *y               = (volatile _SPM short *)      Y_ADDR; // y[2]: output

int main() {

    setup(0); //for guitar

    // enable input and output
    *audioDacEnReg = 1;
    *audioAdcEnReg = 1;

    setInputBufferSize(BUFFER_SIZE);
    setOutputBufferSize(BUFFER_SIZE);

    //old way:
    storeSinInterpol(vibrSin, vibrFrac, VIBR_P, (VIBRATO_LENGTH*0.5), ((VIBRATO_LENGTH-1)*0.5));

    //initialise last SPM position pointer
    int *ADDR;
    *ADDR = LAST_ADDR;

    int VIBR_ALLOC_AMOUNT = alloc_vibrato_vars(ADDR);

    //CPU cycles stuff
    //int CPUcycles[1000] = {0};


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
