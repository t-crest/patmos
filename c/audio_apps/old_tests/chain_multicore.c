#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libaudio/audio.h"
#include "libaudio/audio.c"

/*
  Chain:
    -Delay
    -Vibrato
*/

int main() {

    #if GUITAR == 1
    setup(1); //for guitar
    #else
    setup(0); //for volca
    #endif

    // enable input and output
    *audioDacEnReg = 1;
    *audioAdcEnReg = 1;

    setInputBufferSize(BUFFER_SIZE);
    setOutputBufferSize(BUFFER_SIZE);

    struct IIRdelay del1;
    struct IIRdelay *del1Pnt = &del1;
    struct AudioFX *del1FXPnt = (struct AudioFX *) del1Pnt;
    int DEL_ALLOC_AMOUNT = alloc_delay_vars(del1Pnt, 0);

    struct Filter filt1;
    struct Filter *filt1Pnt = &filt1;
    struct AudioFX *filt1FXPnt = (struct AudioFX *) filt1Pnt;
    int FILT_ALLOC_AMOUNT = alloc_filter_vars(filt1Pnt, 0, 600, 0.707, 0); //LPF

    //CPU cycles stuff
    //int CPUcycles[1000] = {0};
    //int cpu_pnt = 0;

    while(*keyReg != 3) {
        audioIn(del1FXPnt);
        audio_delay(del1Pnt);
        audioChainCore(del1FXPnt, filt1FXPnt);
        audio_filter(filt1Pnt);
        audioOut(filt1FXPnt);
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
