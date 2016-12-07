/*
#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
*/

#include "libaudio/audio.h"
#include "libaudio/audio.c"

const int LIM = 1000;

//master core
const int NOC_MASTER = 0;


//how many cores take part in the audio system
const int AUDIO_CORES = 3;
//how many effects are on the system in total
const int FX_AMOUNT = 6;
// FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | P (S) | IN_TYPE | OUT_TYPE | FROM_ID | TO_ID //
const int FX_SCHED[FX_AMOUNT][10] = {
    {0, 0,  0, 8, 8, 1, 0, 0, -1,  1},
    {1, 0,  2, 8, 8, 1, 0, 1,  0,  0},
    {2, 1,  0, 8, 8, 1, 1, 0,  0,  3},
    {3, 1,  3, 8, 8, 1, 0, 1,  2,  1},
    {4, 2,  5, 8, 8, 1, 1, 1,  1,  2},
    {5, 0,  0, 8, 8, 1, 1, 0,  2, -1}
};

/*
//how many cores take part in the audio system
const int AUDIO_CORES = 3;
//how many effects are on the system in total
const int FX_AMOUNT = 5;
// FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | P (S) | IN_TYPE | OUT_TYPE | FROM_ID | TO_ID //
const int FX_SCHED[FX_AMOUNT][10] = {
    {0, 0, 0, 8, 8, 1, 0, 0, -1,  1},
    {1, 0, 1, 8, 8, 8, 0, 1,  0,  0},
    {2, 1, 4, 8, 1, 1, 1, 1,  0,  1},
    {3, 2, 0, 1, 8, 1, 1, 1,  1,  2},
    {4, 0, 0, 8, 8, 1, 1, 0,  2, -1}
};
*/
/*
//how many cores take part in the audio system
const int AUDIO_CORES = 4;
//how many effects are on the system in total
const int FX_AMOUNT = 5;
// FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | P (S) | IN_TYPE | OUT_TYPE | FROM_ID | TO_ID //
const int FX_SCHED[FX_AMOUNT][10] = {
    {0, 0, 0, 1, 1, 1, 0, 1, -1,  0},
    {1, 1, 5, 1, 8, 1, 1, 1,  0,  1},
    {2, 2, 1, 8, 8, 8, 1, 1,  1,  2},
    {3, 3, 0, 8, 1, 1, 1, 1,  2,  3},
    {4, 0, 0, 1, 1, 1, 1, 0,  3, -1}
};
*/


void threadFunc(void* args) {
    volatile _UNCACHED int **inArgs = (volatile _UNCACHED int **) args;
    volatile _UNCACHED int *exitP      = inArgs[0];
    volatile _UNCACHED int *allocsDoneP = inArgs[1];


    /*
      AUDIO STUFF HERE
    */

    int cpuid = get_cpuid();

    // -------------------ALLOCATE FX------------------//

    int FX_HERE = 0; //amount of effects in this core
    for(int n=0; n<FX_AMOUNT; n++) {
        if(FX_SCHED[n][1] == cpuid) {
            FX_HERE++;
        }
    }
    //create structs
    struct AudioFX FXp[FX_HERE];
    //struct parameters:
    int fx_id;
    fx_t fx_type;
    int xb_size, yb_size, p;
    con_t in_con;
    con_t out_con;
    fst_t is_fst;
    lst_t is_lst;

    // READ FROM SCHEDULER
    int fx_ind = 0;
    for(int n=0; n<FX_AMOUNT; n++) {
        if(FX_SCHED[n][1] == cpuid) { //same core
            //assign parameters from SCHEDULER
            fx_id   =         FX_SCHED[n][0];
            fx_type = (fx_t)  FX_SCHED[n][2];
            xb_size =         FX_SCHED[n][3];
            yb_size =         FX_SCHED[n][4];
            p       =         FX_SCHED[n][5];
            in_con  = (con_t) FX_SCHED[n][6];
            out_con = (con_t) FX_SCHED[n][7];
            if(FX_SCHED[n][8] == -1) {
                is_fst = FIRST;
            }
            else {
                is_fst = NO_FIRST;
            }
            if(FX_SCHED[n][9] == -1) {
                is_lst = LAST;
            }
            else {
                is_lst = NO_LAST;
            }
            //allocate
            alloc_audio_vars(&FXp[fx_ind], fx_id, fx_type, in_con,
                out_con, xb_size, yb_size, p, is_fst, is_lst);
            fx_ind++;
        }
    }

    //CONNECT EFFECTS
    for(int n=0; n<FX_HERE; n++) {
        // same core
        if(*FXp[n].out_con == NO_NOC) {
            int destID = FX_SCHED[*FXp[n].fx_id][9]; //ID to connect to
            if(FX_SCHED[destID][8] != *FXp[n].fx_id) {
                printf("ERROR: SAME CORE CONNECTION MISMATCH\n");
            }
            for(int m=0; m<FX_HERE; m++) {
                if(*FXp[m].fx_id == destID) {
                    audio_connect_same_core(&FXp[n], &FXp[m]);
                    break;
                }
            }
        }
        // NoC
        if(*FXp[n].in_con == NOC) {
            int recvChanID = FX_SCHED[*FXp[n].fx_id][8]; //NoC receive channel ID
            audio_connect_from_core(recvChanID, &FXp[n]);
        }
        if(*FXp[n].out_con == NOC) {
            int sendChanID = FX_SCHED[*FXp[n].fx_id][9]; //NoC send channel ID
            audio_connect_to_core(&FXp[n], sendChanID);
        }
    }

    // wait until all cores are ready
    allocsDoneP[cpuid] = 1;
    for(int i=0; i<AUDIO_CORES; i++) {
        while(allocsDoneP[i] == 0);
    }


    // Initialize the communication channels
    int nocret = mp_init_ports();

    //loop
    //audioValuesP[0] = 0;
    //int i=0;
    while(*exitP == 0) {
    //i++;
    //for(int i=0; i<DEBUG_LOOPLENGTH; i++) {

        for(int n=0; n<FX_HERE; n++) {
            if (audio_process(&FXp[n]) == 1) {
                //timeout stuff here
            }
        }

    }


    // exit with return value
    int ret = 0;
    corethread_exit(&ret);
    return;
}



int main() {

    //arguments to thread 1 function
    int exit = 0;
    int allocsDone[AUDIO_CORES] = {0};
    volatile _UNCACHED int *exitP = (volatile _UNCACHED int *) &exit;
    volatile _UNCACHED int *allocsDoneP = (volatile _UNCACHED int *) &allocsDone;
    volatile _UNCACHED int (*threadFunc_args[1+AUDIO_CORES]);
    threadFunc_args[0] = exitP;
    threadFunc_args[1] = allocsDoneP;

    printf("starting thread and NoC channels...\n");
    //set thread function and start thread
    corethread_t threads[AUDIO_CORES-1];
    for(int i=0; i<(AUDIO_CORES-1); i++) {
        threads[i] = (corethread_t) (i+1);
        corethread_create(&threads[i], &threadFunc, (void*) threadFunc_args);
        printf("Thread created on core %d\n", i+1);
    }

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

    // -------------------ALLOCATE FX------------------//

    int FX_HERE = 0; //amount of effects in this core
    for(int n=0; n<FX_AMOUNT; n++) {
        if(FX_SCHED[n][1] == cpuid) {
            FX_HERE++;
        }
    }
    //create structs
    struct AudioFX FXp[FX_HERE];
    //struct parameters:
    int fx_id;
    fx_t fx_type;
    int xb_size, yb_size, p;
    con_t in_con;
    con_t out_con;
    fst_t is_fst;
    lst_t is_lst;

    // READ FROM SCHEDULER
    int fx_ind = 0;
    for(int n=0; n<FX_AMOUNT; n++) {
        if(FX_SCHED[n][1] == cpuid) { //same core
            //assign parameters from SCHEDULER
            fx_id   =         FX_SCHED[n][0];
            fx_type = (fx_t)  FX_SCHED[n][2];
            xb_size =         FX_SCHED[n][3];
            yb_size =         FX_SCHED[n][4];
            p       =         FX_SCHED[n][5];
            in_con  = (con_t) FX_SCHED[n][6];
            out_con = (con_t) FX_SCHED[n][7];
            if(FX_SCHED[n][8] == -1) {
                is_fst = FIRST;
            }
            else {
                is_fst = NO_FIRST;
            }
            if(FX_SCHED[n][9] == -1) {
                is_lst = LAST;
            }
            else {
                is_lst = NO_LAST;
            }
            //allocate
            alloc_audio_vars(&FXp[fx_ind], fx_id, fx_type, in_con,
                out_con, xb_size, yb_size, p, is_fst, is_lst);
            fx_ind++;
        }
    }

    //CONNECT EFFECTS
    for(int n=0; n<FX_HERE; n++) {
        //print input effects
        if(*FXp[n].is_fst == FIRST) {
            printf("FIRST: ID=%d\n", *FXp[n].fx_id);
        }
        // same core
        if( (*FXp[n].out_con == NO_NOC) && (*FXp[n].is_lst == NO_LAST) ) {
            int destID = FX_SCHED[*FXp[n].fx_id][9]; //ID to connect to
            if(FX_SCHED[destID][8] != *FXp[n].fx_id) {
                printf("ERROR: SAME CORE CONNECTION MISMATCH\n");
            }
            for(int m=0; m<FX_HERE; m++) {
                if(*FXp[m].fx_id == destID) {
                    audio_connect_same_core(&FXp[n], &FXp[m]);
                    printf("SAME CORE: connected ID=%d and ID=%d\n", *FXp[n].fx_id, *FXp[m].fx_id);
                    break;
                }
            }
        }
        // NoC
        if(*FXp[n].in_con == NOC) {
            int recvChanID = FX_SCHED[*FXp[n].fx_id][8]; //NoC receive channel ID
            audio_connect_from_core(recvChanID, &FXp[n]);
            printf("NoC: connected recvChanelID=%d to ID=%d\n", recvChanID, *FXp[n].fx_id);
        }
        if(*FXp[n].out_con == NOC) {
            int sendChanID = FX_SCHED[*FXp[n].fx_id][9]; //NoC send channel ID
            audio_connect_to_core(&FXp[n], sendChanID);
            printf("NoC: connected ID=%d to sendChanelID=%d\n", *FXp[n].fx_id, sendChanID);
        }
        //print output effects
        if(*FXp[n].is_lst == LAST) {
            printf("LAST: ID=%d\n", *FXp[n].fx_id);
        }
    }

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
    int CPUcycles[LIM] = {0};
    unsigned int cpu_pnt = 0;


    //int wait_recv = 18; //amount of loops until audioOut is done
    int wait_recv = 3;
    //for debugging
    //const int WAIT = wait_recv;

    //short audio_in[LIM][2] = {0};
    //short audio_out[LIM][2] = {0};

    while(*keyReg != 3) {

        for(int n=0; n<FX_HERE; n++) {
            if( (*FXp[n].is_lst == NO_LAST) || (wait_recv == 0) ) {
                audio_process(&FXp[n]);
                /*
                if(n==0) {
                    audio_in[cpu_pnt][0] = FXp[n].x[0];
                    audio_in[cpu_pnt][1] = FXp[n].x[1];
                }
                if(n==(FX_HERE-1)) {
                    audio_out[cpu_pnt-WAIT][0] = FXp[n].y[0];
                    audio_out[cpu_pnt-WAIT][1] = FXp[n].y[1];
                }
                */
            }
            else {
                wait_recv--;
            }
        }


        //store CPU Cycles
        CPUcycles[cpu_pnt] = get_cpu_cycles();
        cpu_pnt++;
        if(cpu_pnt == LIM) {
            //break;
            cpu_pnt = 0;
        }


    }


    //exit stuff
    printf("exit here!\n");
    exit = 1;
    printf("waiting for all threads to finish...\n");


    for(int i=1; i<LIM; i++) {
        printf("%d\n", (CPUcycles[i]-CPUcycles[i-1]));
    }


    /*
    for(int i=0; i<(LIM-WAIT); i++) {
        if( (audio_in[i][0] != audio_out[i][0]) || (audio_in[i][1] != audio_out[i][1]) ){
            printf("CORRUPT: i=%d: x[0]=%d, y[0]=%d   :   x[1]=%d, y[1]=%d\n", i, audio_in[i][0], audio_out[i][0], audio_in[i][1], audio_out[i][1]);
        }
    }
    */


    //join with thread 1

    int *retval;
    for(int i=0; i<(AUDIO_CORES-1); i++) {
        corethread_join(threads[i], (void **)&retval);
        printf("thread %d finished!\n", (i+1));
    }

    return 0;
}
