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

    setup(0);

    // enable input and output
    *audioDacEnReg = 1;
    *audioAdcEnReg = 1;

    setInputBufferSize(BUFFER_SIZE);
    setOutputBufferSize(BUFFER_SIZE);

    struct Filter filter1;
    struct Filter *filter1Pnt = &filter1;
    struct AudioFX *filter1FXPnt = (struct AudioFX *) filter1Pnt;
    int FILTER_ALLOC_AMOUNT;

    struct Filter32 filter321;
    struct Filter32 *filter321Pnt = &filter321;
    struct AudioFX *filter321FXPnt = (struct AudioFX *) filter321Pnt;
    int FILTER32_ALLOC_AMOUNT;

    printf("Press KEY0 to play LPF, KEY1 for HPF, KEY2 for BPF and KEY3 for BRF\n");
    while(*keyReg == 15);
    if(*keyReg == 14) {
        FILTER_ALLOC_AMOUNT = alloc_filter_vars(filter1Pnt, 0, 600, 0.707, 0); //LPF
        FILTER32_ALLOC_AMOUNT = alloc_filter32_vars(filter321Pnt, 0, 600, 0.707, 0); //LPF
    }
    if(*keyReg == 13) {
        FILTER_ALLOC_AMOUNT = alloc_filter_vars(filter1Pnt, 0, 5000, 0.707, 1); //HPF
        FILTER32_ALLOC_AMOUNT = alloc_filter32_vars(filter321Pnt, 0, 5000, 0.707, 1); //HPF
    }
    if(*keyReg == 11) {
        FILTER_ALLOC_AMOUNT = alloc_filter_vars(filter1Pnt, 0, 1000, 300, 2); //BPF
        FILTER32_ALLOC_AMOUNT = alloc_filter32_vars(filter321Pnt, 0, 1000, 300, 2); //BPF
    }
    if(*keyReg == 7) {
        FILTER_ALLOC_AMOUNT = alloc_filter_vars(filter1Pnt, 0, 500, 2000, 3); //BRF
        FILTER32_ALLOC_AMOUNT = alloc_filter32_vars(filter321Pnt, 0, 500, 2000, 3); //BRF
    }
    printf("Done!\n");

    //CPU cycles stuff
    //int CPUcycles[1000] = {0};
    //int cpu_pnt = 0;

    int type = 0;
    printf("press KEY0 for 16-bit tremolo, KEY1 for 32-bit tremolo\n");
    while(*keyReg != 3) {
        if (type == 0) {
            audioIn(filter1FXPnt);
            audio_filter(filter1Pnt);
            audioOut(filter1FXPnt);
        }
        else {
            audioIn(filter321FXPnt);
            audio_filter32(filter321Pnt);
            audioOut(filter321FXPnt);
        }
        if( (*keyReg == 14) && (type==1) ) {
            printf("16-bit\n");
            type = 0;
        }
        if( (*keyReg == 13) && (type==0) ) {
            printf("32-bit\n");
            type = 1;
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
    //print CPU cycle time
    for(int i=1; i<1000; i++) {
        printf("%d\n", (CPUcycles[i]-CPUcycles[i-1]));
    }
    */

    return 0;
}
