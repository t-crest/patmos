#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "audio.h"
#include "audio.c"

/*
 * @file		Audio_InOut.c
 * @author	Daniel Sanz Ausin s142290 & Fabian Goerge s150957
 * @brief	This program takes the input auido data and outputs it again
 */


int main() {

    setup(0); //guitar?

    // enable input and output
    *audioDacEnReg = 1;
    *audioAdcEnReg = 1;

    setInputBufferSize(32);
    setOutputBufferSize(32);

    //AudioFX struct: contains no effect
    struct AudioFX audio1FX;
    struct AudioFX *audio1FXPnt = &audio1FX;
    int AUDIO_ALLOC_AMOUNT = alloc_dry_vars(audio1FXPnt, 0);

    while(*keyReg != 3) {
        audioIn(audio1FXPnt);
        audio_dry(audio1FXPnt);
        audioOut(audio1FXPnt);
    }

    return 0;
}
