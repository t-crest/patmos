#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libaudio/audio.h"
#include "libaudio/audio.c"



void thread1_delay(void* args) {
    volatile _UNCACHED short **inArgs = (volatile _UNCACHED short **) args;
    volatile _UNCACHED short *left      = inArgs[0];
    volatile _UNCACHED short *right     = inArgs[1];
    volatile _UNCACHED short *syncToken = inArgs[2];
    volatile _UNCACHED short *exit      = inArgs[3];

    struct IIRdelay del1;
    struct IIRdelay *del1Pnt = &del1;
    int DEL_ALLOC_AMOUNT = alloc_delay_vars(del1Pnt, 0);


    while(*exit == 0) {
        //wait for sync (except if exit)
        while(*syncToken == 0) {
            if(*exit != 0) {
                break;
            }
        }
        //write input
        del1Pnt->x[0] = *left;
        del1Pnt->x[1] = *right;
        //compute delay
        audio_delay(del1Pnt);
        //write output
        *left  = del1Pnt->y[0];
        *right = del1Pnt->y[1];
        //pass token
        *syncToken = 0;
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
    short left, right, syncToken, exit = 0;
    volatile _UNCACHED short *leftP      = (volatile _UNCACHED short *) &left;
    volatile _UNCACHED short *rightP     = (volatile _UNCACHED short *) &right;
    volatile _UNCACHED short *syncTokenP = (volatile _UNCACHED short *) &syncToken;
    volatile _UNCACHED short *exitP      = (volatile _UNCACHED short *) &exit;
    volatile _UNCACHED short (*thread1_args[4]);
    thread1_args[0] = leftP;
    thread1_args[1] = rightP;
    thread1_args[2] = syncTokenP;
    thread1_args[3] = exitP;
    printf("starting thread...\n");
    //set thread function
    corethread_create(&threadOne, &thread1_delay, (void*) thread1_args);
    //start thread on slave core 1

    //AudioFX struct: contains no effect
    struct AudioFX audio1FX;
    struct AudioFX *audio1FXPnt = &audio1FX;
    int AUDIO_ALLOC_AMOUNT = alloc_dry_vars(audio1FXPnt, 0);

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

    //CPU cycles stuff
    //int CPUcycles[1000] = {0};
    //int cpu_pnt = 0;

    while(*keyReg != 3) {
        //put input in x
        audioIn(audio1FXPnt);
        //write from x to memory
        *leftP  = audio1FXPnt->x[0];
        *rightP = audio1FXPnt->x[1];
        //pass token
        *syncTokenP = 1;
        //wait for thread 1 to finish computation
        while(*syncTokenP == 1);
        //printf("printf synto is %d\n", syncToken);
        audio1FXPnt->y[0] = *leftP;
        audio1FXPnt->y[1] = *rightP;
        //put y in output
        audioOut(audio1FXPnt);
        /*
        //store CPU Cycles
        CPUcycles[cpu_pnt] = get_cpu_cycles();
        cpu_pnt++;
        if(cpu_pnt == 1000) {
            break;
        }
        */
    }

    //exit stuff
    printf("exit here!\n");
    exit = 1;
    printf("waiting for thread 1 to finish...\n");
    //join with thread 1
    int *retval;
    corethread_join(threadOne, (void **)&retval);
    printf("thread 1 finished!\n");
    /*
    for(int i=1; i<1000; i++) {
        printf("%d\n", (CPUcycles[i]-CPUcycles[i-1]));
    }
    */
    return 0;
}
