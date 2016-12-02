/*
#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
*/

#include "libaudio/audio.h"
#include "libaudio/audio.c"

//master core
const int NOC_MASTER = 0;
//how many cores take part in the audio system
const int AUDIO_CORES = 2;
//how many effects are on the system in total
const int FX_AMOUNT = 4;


// FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | P (S) | IN_TYPE | OUT_TYPE | FROM_ID | TO_ID //
const int FX_SCHED[FX_AMOUNT][10] = {
    {0, 0, 0, 8, 8, 1, 0, 0, -1,  1},
    {1, 0, 1, 8, 8, 8, 0, 1,  0,  0},
    {2, 1, 0, 8, 8, 1, 1, 1,  0,  1},
    {3, 0, 0, 8, 8, 1, 1, 0,  1, -1}
};

void threadFunc(void* args) {
    volatile _UNCACHED int **inArgs = (volatile _UNCACHED int **) args;
    volatile _UNCACHED int *exitP      = inArgs[0];
    volatile _UNCACHED int *allocsDoneP = inArgs[1];

    volatile _UNCACHED int *audioValuesP = inArgs[3];

    /*
      AUDIO STUFF HERE
    */

    int cpuid = get_cpuid();

    //init and allocate
    struct AudioFX audio1a;
    struct AudioFX *audio1aP = &audio1a;
    //from NoC, to NoC, is not 1st, is notlast
    alloc_audio_vars(audio1aP, DRY, NOC, NOC, 8, 8, 1, NO_FIRST, NO_LAST);

    audio_connect_from_core(0, audio1aP); // effect audio1a from corresponding channelID
    audio_connect_to_core(audio1aP, 1); // effect audio1a to corresponding channelID

    // wait until all cores are ready
    allocsDoneP[cpuid] = 1;
    for(int i=0; i<AUDIO_CORES; i++) {
        while(allocsDoneP[i] == 0);
    }


    // Initialize the communication channels
    int nocret = mp_init_ports();

    //loop
    //for(int i=0; i<3; i++) {
    audioValuesP[0] = 0;
    while(*exitP == 0) {
        //process
        if (audio_process(audio1aP) == 1) {
            audioValuesP[0] = audioValuesP[0] + 1;
        }

        /*
        volatile _SPM short * xP = (volatile _SPM short *)*(volatile _SPM unsigned int *)*audio1aP->x_pnt;
        audioValuesP[i] = xP[0];
        audioValuesP[i] = (audioValuesP[i] & 0xFFFF) | ( (xP[1] & 0xFFFF) << 16 );
        */

    }


    // exit with return value
    int ret = 0;
    corethread_exit(&ret);
    return;
}



int main() {

    //create corethread type var
    corethread_t threadOne = (corethread_t) 1;
    //arguments to thread 1 function
    int exit = 0;
    int allocsDone[2] = {0, 0};
    volatile _UNCACHED int *exitP = (volatile _UNCACHED int *) &exit;
    volatile _UNCACHED int *allocsDoneP = (volatile _UNCACHED int *) &allocsDone;
    volatile _UNCACHED int (*threadFunc_args[6]);
    threadFunc_args[0] = exitP;
    threadFunc_args[1] = allocsDoneP;

    int audioValues[3] = {0, 0, 0};
    volatile _UNCACHED int *audioValuesP = (volatile _UNCACHED int *) &audioValues[0];
    threadFunc_args[3] = audioValuesP;

    printf("starting thread and NoC channels...\n");
    //set thread function and start thread
    corethread_create(&threadOne, &threadFunc, (void*) threadFunc_args);

    /*
    qpd_t * chan2 = mp_create_qport(MP_CHAN_2_ID, SINK,
        MP_CHAN_2_MSG_SIZE, MP_CHAN_2_NUM_BUF);

    */

    /*
      AUDIO STUFF HERE
    */

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


    int cpuid = get_cpuid();

    // create effects and connect
    struct AudioFX audio0a;
    struct AudioFX *audio0aP = &audio0a;
    // READ FROM SCHEDULER
    int n=0;
    fx_t fx_type  = (fx_t)  FX_SCHED[n][2];
    int xb_size   =         FX_SCHED[n][3];
    int yb_size   =         FX_SCHED[n][4];
    int p         =         FX_SCHED[n][5];
    con_t in_con  = (con_t) FX_SCHED[n][6];
    con_t out_con = (con_t) FX_SCHED[n][7];
    fst_t is_fst;
    if(FX_SCHED[n][8] == -1) {
        is_fst = FIRST;
    }
    else {
        is_fst = NO_FIRST;
    }
    lst_t is_lst;
    if(FX_SCHED[n][9] == -1) {
        is_lst = LAST;
    }
    else {
        is_lst = NO_LAST;
    }
    alloc_audio_vars(audio0aP, fx_type, in_con, out_con, xb_size, yb_size, p, is_fst, is_lst);

    struct AudioFX audio0b;
    struct AudioFX *audio0bP = &audio0b;
    //from same, to NoC, is not 1st, is not last
    alloc_audio_vars(audio0bP, DRY_8S, NO_NOC, NOC, 8, 8, 8, NO_FIRST, NO_LAST);


    struct AudioFX audio0c;
    struct AudioFX *audio0cP = &audio0c;
    //from NoC, to same, is not 1st, is last
    alloc_audio_vars(audio0cP, DRY, NOC, NO_NOC, 8, 8, 1, NO_FIRST, LAST);

    audio_connect_same_core(audio0aP, audio0bP); //effects on same core
    audio_connect_to_core(audio0bP, 0); // effect audio0b to corresponding channelID
    audio_connect_from_core(1, audio0cP); //effect audio0c from corresponding channelID




    // wait until all cores are ready
    allocsDoneP[cpuid] = 1;
    for(int i=0; i<AUDIO_CORES; i++) {
        while(allocsDoneP[i] == 0);
    }


    // Initialize the communication channels
    int nocret = mp_init_ports();
    if(nocret == 1) {
        printf("Thread and NoC initialised correctly\n");
    }
    else {
        printf("ERROR: Problem with NoC initialisation\n");
    }


    //CPU cycles stuff
    int CPUcycles[1000] = {0};
    int cpu_pnt = 0;


    //loop
    //for(int i=0; i<3; i++) {

    /*
    int diff1 = ((unsigned int)&func2 - (unsigned int)&func1);
    int diff2 = ((unsigned int)&alloc_space - (unsigned int)&func2);
    printf("diff1 is 0x%x, diff2 is 0x%x\n", (unsigned int)diff1, (unsigned int)diff2);
    int diffSer = ((unsigned int)&func1 - (unsigned int)&audio_dry);
    printf("diffSer is 0x%x\n", (unsigned int)diffSer);

    //copying audio_dry to instruction SPM
    volatile _SPM unsigned int * instSpmP;
    unsigned int *funcP = (unsigned int *)&audio_dry;
    instSpmP = (volatile _SPM unsigned int *) 0x00010000; //instruction SPM start point (OFFSET???)
    for(unsigned int i=0; i<(diffSer/4); i++) {
        *(instSpmP+i) = *(funcP+i);
        printf("@ %d: copied from 0x%x to 0x%x\n", i, (unsigned int)(funcP+i), (unsigned int)(instSpmP+i));
        printf("copied data: 0x%x, 0x%x\n", (unsigned int)*(funcP+i), *(instSpmP+i));
    }

    //assign new value to pointer function
    //audio0aP->funcP = (unsigned int)instSpmP;
    //audio0bP->funcP = (unsigned int)instSpmP;
    //audio0cP->funcP = (unsigned int)instSpmP;
    */


    int first = 1;
    while(*keyReg != 3) {
        //process
        //int cycles = get_cpu_cycles();

        audio_process(audio0aP);

        /*
        cycles = get_cpu_cycles() - cycles;
        printf("cpu cycles: %d\n", cycles);
        */

        /*
        printf("\n\n\nINPUT DATA A: \n");
        volatile _SPM short * xP;
        xP = (volatile _SPM short *)*audio0aP->x_pnt;
        for(unsigned int i=0; i < *audio0aP->xb_size; i++) {
            printf("%d, %d    ", xP[i*2], xP[i*2+1]);
        }

        volatile _SPM short * yP;

         printf("\noutput data a: \n");
         volatile _SPM short * yP;
         yP = (volatile _SPM short *)*audio0aP->y_pnt;
        for(unsigned int i=0; i < *audio0aP->yb_size; i++) {
            printf("%d, %d    ", yP[i*2], yP[i*2+1]);
        }



        //printf("in values: %d, %d\n", audio0bP->x[0], audio0bP->x[1]);

        printf("\ninput data b: \n");
        xP = (volatile _SPM short *)*audio0bP->x_pnt;
        for(unsigned int i=0; i < *audio0bP->xb_size; i++) {
            printf("%d, %d    ", xP[i*2], xP[i*2+1]);
        }
        */


        audio_process(audio0bP);




        /*
        printf("\noutput data b: \n");
        yP = (volatile _SPM short *)*(volatile _SPM unsigned int *)*audio0bP->y_pnt;
        for(unsigned int i=0; i < *audio0bP->yb_size; i++) {
            printf("%d, %d    ", yP[i*2], yP[i*2+1]);
        }
        */


        /*
        printf("y_pnt points to 0x%x\n", *audio0bP->y_pnt);
        printf("*y_pnt points to 0x%x\n", *(volatile _SPM unsigned int *)*audio0bP->y_pnt);
        printf("data at **y_pnt is %d, %d\n", *(volatile _SPM short *)*(volatile _SPM unsigned int *)*audio0bP->y_pnt, *(volatile _SPM short *)((*(volatile _SPM unsigned int *)*audio0bP->y_pnt)+2));

        printf("write buffer address: 0x%x\n", (int)((qpd_t *)*audio0bP->sendChanP)->write_buf);
        printf("write buffer data: %d, %d\n", (int)*((volatile _SPM short *)((qpd_t *)*audio0bP->sendChanP)->write_buf), (int)*((volatile _SPM short *)((qpd_t *)*audio0bP->sendChanP)->write_buf+ 1));



        volatile _SPM short * xP = (volatile _SPM short *)*(volatile _SPM unsigned int *)*audio0cP->x_pnt;
        printf("RECEIVED FROM CORE 1: %d, %d\n", xP[0], xP[1]);
        */

        if(first == 0) {
            audio_process(audio0cP);
            /*
            printf("\noutput data c: \n");
            yP = (volatile _SPM short *)*audio0cP->y_pnt;
            for(unsigned int i=0; i < *audio0cP->yb_size; i++) {
                printf("%d, %d    ", yP[i*2], yP[i*2+1]);
            }
            */
        }
        else {
            first = 0;
        }


        /*
        printf("\ninput data c: \n");
        xP = (volatile _SPM short *)*(volatile _SPM unsigned int *)*audio0cP->x_pnt;
        for(unsigned int i=0; i < *audio0cP->xb_size; i++) {
            printf("%d, %d    ", xP[i*2], xP[i*2+1]);
        }
        */





        /*
        //store CPU Cycles
        CPUcycles[cpu_pnt] = get_cpu_cycles();
        cpu_pnt++;
        if(cpu_pnt == 1000) {
            //break;
            cpu_pnt = 0;
        }
        */

    }

    /*
    for(int i=0; i<3; i++) {
        printf("received @ %d: %d, %d\n", i, (short)(audioValuesP[i] & 0xFFFF), (short)( (audioValuesP[i] & 0xFFFF0000) >> 16 ));
    }
    */


    /*
    printf("SOME CARACS:\n");
    printf("AUDIO1:\n");
    printf("CPUID = %d\n", *audio1P->cpuid);
    printf("IS_FST = %d\n", *audio1P->is_fst);
    printf("IS_LST = %d\n", *audio1P->is_lst);
    printf("IN_CON = %d\n", *audio1P->in_con);
    printf("OUT_CON = %d\n", *audio1P->out_con);
    printf("X_PNT = 0x%x\n", *audio1P->x_pnt);
    printf("Y_PNT = 0x%x\n", *audio1P->y_pnt);
    printf("AUDIO2:\n");
    printf("CPUID = %d\n", *audio2P->cpuid);
    printf("IS_FST = %d\n", *audio2P->is_fst);
    printf("IS_LST = %d\n", *audio2P->is_lst);
    printf("IN_CON = %d\n", *audio2P->in_con);
    printf("OUT_CON = %d\n", *audio2P->out_con);
    printf("X_PNT = 0x%x\n", *audio2P->x_pnt);
    printf("Y_PNT = 0x%x\n", *audio2P->y_pnt);
    */

    //exit stuff
    printf("exit here!\n");
    exit = 1;
    printf("waiting for thread 1 to finish...\n");

    /*
    for(int i=1; i<1000; i++) {
        printf("%d\n", (CPUcycles[i]-CPUcycles[i-1]));
    }
    */

    //join with thread 1
    int *retval;
    corethread_join(threadOne, (void **)&retval);
    printf("thread 1 finished!\n");

    printf("thread 1 timeout amounts: %d\n", audioValuesP[0]);

    return 0;
}
