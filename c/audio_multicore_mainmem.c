#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "audio.h"
#include "audio.c"



void thread1_blink(void* args) {
    int **intArgs = (int **) args;
    int *ledOnTimeP = intArgs[0];
    int *ledOffTimeP = intArgs[1];
    volatile _UNCACHED int *exit = (volatile _UNCACHED int *) intArgs[2];
    while(*exit == 0) {
        for (int i=0; i<(*ledOnTimeP * 500000); i++) {
            //keep led ON
            *ledReg = 1;
        }
        for (int i=0; i<(*ledOffTimeP * 500000); i++) {
            //keep led OFF
            *ledReg = 0;
        }
    }

    // exit with return value
    int ret = 0;
    corethread_exit(&ret);
    return;
}


int main() {

    printf("Core amount is %d\n", get_cpucnt());

    //create corethread type var
    corethread_t threadOne = (corethread_t) 1;
    //arguments to thread 1 function
    int ledOnTime = 15; //no time unit
    int ledOffTime = 5; //no time unit
    volatile _UNCACHED int *exit;
    *exit = 0; //
    int (*thread1_args[3]);
    thread1_args[0] = &ledOnTime;
    thread1_args[1] = &ledOffTime;
    thread1_args[2] = (int *) exit;
    printf("starting thread...\n");
    //set thread function
    corethread_create(&threadOne, &thread1_blink, (void*) thread1_args);
    //start thread on slave core 1



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
    //exit stuff
    printf("exit here!\n");
    *exit = 1;
    printf("waiting for thread 1 to finish...\n");
    //join with thread 1
    int *retval;
    corethread_join(threadOne, (void **)&retval);
    printf("thread 1 finished!\n");

    return 0;
}
