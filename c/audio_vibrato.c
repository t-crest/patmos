#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "audio.h"
#include "audio.c"

/* Vibrato:
     -Buffer Length sets the amount of vibrato: amplitude of sin
     -Vibrato period sets the rate of the vibrato: period of sin
*/

int main() {

    setup(0); //for guitar

    // enable input and output
    *audioDacEnReg = 1;
    *audioAdcEnReg = 1;

    setInputBufferSize(BUFFER_SIZE);
    setOutputBufferSize(BUFFER_SIZE);

    struct Vibrato vibr1;
    struct Vibrato *vibr1Pnt = &vibr1;
    struct AudioFX *vibr1FXPnt = (struct AudioFX *) vibr1Pnt;
    int VIBR_ALLOC_AMOUNT = alloc_vibrato_vars(vibr1Pnt, 0);

    //CPU cycles stuff
    //int CPUcycles[100] = {0};
    //int cpu_pnt = 0;

    while(*keyReg != 3) {
        audioIn(vibr1FXPnt);
        audio_vibrato(vibr1Pnt);
        audioOut(vibr1FXPnt);
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
    //print CPU cycle time
    for(int i=1; i<1000; i++) {
        printf("%d\n", (CPUcycles[i]-CPUcycles[i-1]));
    }
    */

    return 0;
}
