#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <math.h>

#include "audio.h"
#include "audio.c"

/* WahWah:
     -Addition of original with band-passed signal
     -SIN Modulation of Fc of BandPass filter (B and A coefficients)
*/


int main() {

    setup(0);

    // enable input and output
    *audioDacEnReg = 1;
    *audioAdcEnReg = 1;

    setInputBufferSize(BUFFER_SIZE);
    setOutputBufferSize(BUFFER_SIZE);

    struct WahWah wah1;
    struct WahWah *wah1Pnt = &wah1;
    struct AudioFX *wah1FXPnt = (struct AudioFX *) wah1Pnt;
    int WAH_ALLOC_AMOUNT = alloc_wahwah_vars(wah1Pnt, 0);

    //CPU cycles stuff
    //int CPUcycles[1000] = {0};
    //int cpu_pnt = 0;

    while(*keyReg != 3) {
        audioIn(wah1FXPnt);
        audio_wahwah(wah1Pnt);
        audioOut(wah1FXPnt);
        /*
        CPUcycles[cpu_pnt] = get_cpu_cycles();
        cpu_pnt++;
        if(cpu_pnt == 1000) {
            break;
        }
        */
    }
    /*
    for(int i=1; i<1000; i++) {
        printf("%d\n", (CPUcycles[i]-CPUcycles[i-1]));
    }
    */

    return 0;
}
