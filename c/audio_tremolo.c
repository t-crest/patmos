#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <math.h>

#include "audio.h"
#include "audio.c"

/*
  Tremolo:
    - Sin modulation of signal amplitude (volume)
*/

int main() {

    setup(1);

    // enable input and output
    *audioDacEnReg = 1;
    *audioAdcEnReg = 1;

    setInputBufferSize(BUFFER_SIZE);
    setOutputBufferSize(BUFFER_SIZE);

    struct Tremolo trem1;
    struct Tremolo *trem1Pnt = &trem1;
    struct AudioFX *trem1FXPnt = (struct AudioFX *) trem1Pnt;
    int TREM_ALLOC_AMOUNT = alloc_tremolo_vars(trem1Pnt, 0);

    struct Tremolo32 trem321;
    struct Tremolo32 *trem321Pnt = &trem321;
    struct AudioFX *trem321FXPnt = (struct AudioFX *) trem321Pnt;
    int TREM32_ALLOC_AMOUNT = alloc_tremolo32_vars(trem321Pnt, 0);

    //CPU cycles stuff
    //int CPUcycles[100] = {0};
    //int cpu_pnt = 0;

    int type = 0;
    printf("press KEY0 for 16-bit tremolo, KEY1 for 32-bit tremolo\n");
    while(*keyReg != 3) {
        if (type == 0) {
            audioIn(trem1FXPnt);
            audio_tremolo(trem1Pnt);
            audioOut(trem1FXPnt);
        }
        else {
            audioIn(trem321FXPnt);
            audio_tremolo32(trem321Pnt);
            audioOut(trem321FXPnt);
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
