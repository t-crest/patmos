#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "audio.h"
#include "audio.c"


/*
   High-Pass / Low-Pass filters:
     -2nd order
*/

int main() {

    setup(0); //for volca

    // enable input and output
    *audioDacEnReg = 1;
    *audioAdcEnReg = 1;

    setInputBufferSize(BUFFER_SIZE);
    setOutputBufferSize(BUFFER_SIZE);

    struct Filter filter1;
    struct Filter *filter1Pnt = &filter1;
    struct AudioFX *filter1FXPnt = (struct AudioFX *) filter1Pnt;
    int FILTER_ALLOC_AMOUNT;

    printf("Press KEY0 to play LPF, KEY1 for HPF, KEY2 for BPF and KEY3 for BRF\n");
    while(*keyReg == 15);
    if(*keyReg == 14) {
        FILTER_ALLOC_AMOUNT = alloc_filter_vars(filter1Pnt, 0, 600, 0.707, 0); //LPF
    }
    if(*keyReg == 13) {
        FILTER_ALLOC_AMOUNT = alloc_filter_vars(filter1Pnt, 0, 5000, 0.707, 1); //HPF
    }
    if(*keyReg == 11) {
        FILTER_ALLOC_AMOUNT = alloc_filter_vars(filter1Pnt, 0, 1000, 300, 2); //BPF
    }
    if(*keyReg == 7) {
        FILTER_ALLOC_AMOUNT = alloc_filter_vars(filter1Pnt, 0, 500, 2000, 3); //BRF
    }
    printf("Done!\n");

    //CPU cycles stuff
    //int CPUcycles[1000] = {0};
    //int cpu_pnt = 0;

    while(*keyReg != 3) {
        audioIn(filter1FXPnt);
        audio_filter(filter1Pnt);
        audioOut(filter1FXPnt);
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
