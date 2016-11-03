#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "audio.h"
#include "audio.c"

/*
  IIR comb delay:
    -First, input data is stored in current position of buffer
    -The COMB function used is the same as FIR
    -The difference is that, after computing new y, this y value is replaced by the x value on the iir_buffer
*/


int main() {


    setup(0); //for guitar

    // enable input and output
    *audioDacEnReg = 1;
    *audioAdcEnReg = 1;

    setInputBufferSize(BUFFER_SIZE);
    setOutputBufferSize(BUFFER_SIZE);

    struct IIRdelay del1;
    struct IIRdelay *del1Pnt = &del1;
    struct AudioFX *del1FXPnt = (struct AudioFX *) del1Pnt;
    int DEL_ALLOC_AMOUNT = alloc_delay_vars(del1Pnt, 0);

    //CPU cycles stuff
    //int CPUcycles[1000] = {0};
    //int cpu_pnt = 0;

    while(*keyReg != 3) {
        audioIn(del1FXPnt);
        audio_delay(del1Pnt);
        audioOut(del1FXPnt);
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
