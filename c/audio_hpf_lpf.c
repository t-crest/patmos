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

    struct HpfLpf hpfLpf1;
    struct HpfLpf *hpfLpf1Pnt = &hpfLpf1;
    struct AudioFX *hpfLpf1FXPnt = (struct AudioFX *) hpfLpf1Pnt;
    int HPFLPF_ALLOC_AMOUNT;

    printf("Press KEY0 to play LPF and KEY1 for HPF\n");
    while(*keyReg == 15);
    if(*keyReg == 14) {
        HPFLPF_ALLOC_AMOUNT = alloc_hpfLpf_vars(hpfLpf1Pnt, 0, 600, 0.707, 0); //LPF
    }
    if(*keyReg == 13) {
        HPFLPF_ALLOC_AMOUNT = alloc_hpfLpf_vars(hpfLpf1Pnt, 0, 5000, 0.707, 1); //HPF
    }
    printf("Done!\n");

    //CPU cycles stuff
    //int CPUcycles[1000] = {0};
    //int cpu_pnt = 0;

    while(*keyReg != 3) {
        audioIn(hpfLpf1FXPnt);
        audio_hpfLpf(hpfLpf1Pnt);
        audioOut(hpfLpf1FXPnt);
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
