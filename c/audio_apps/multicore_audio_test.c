#include "libaudio/audio.h"
#include "libaudio/audio.c"

const int LIM = 1000;
//master core
const int NOC_MASTER = 0;

int allocFX(struct AudioFX *FXp, int FX_HERE, int cpuid) {

    //struct parameters:
    int fx_id;
    fx_t fx_type;
    int xb_size, yb_size, p;
    con_t in_con, out_con;

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
            //allocate
            alloc_audio_vars(&FXp[fx_ind], fx_id, fx_type, in_con,
                out_con, xb_size, yb_size, p);
            fx_ind++;
        }
    }

    //CONNECT EFFECTS
    for(int n=0; n<FX_HERE; n++) {
        //print input effects
        if( (cpuid == 0) && (*FXp[n].in_con == FIRST) ) {
            printf("FIRST: ID=%d\n", *FXp[n].fx_id);
        }

        // OUTPUT
        int recvChanID, sendChanID; // channel ID to connect from and to
        for(int ch=0; ch<CHAN_AMOUNT; ch++) {
            if(SEND_ARRAY[*FXp[n].fx_id][ch] == 1) {
                sendChanID = ch; //found channel ID to connect to
            }
            if(RECV_ARRAY[*FXp[n].fx_id][ch] == 1) {
                recvChanID = ch; //found channel ID to connect to
            }
        }
        //NoC
        if(*FXp[n].in_con == NOC) {
            audio_connect_from_core(recvChanID, &FXp[n]);
            if(cpuid == 0) {
                printf("Connected NoC Chanel ID %d to FX ID %d\n", recvChanID, *FXp[n].fx_id);
            }
        }
        if(*FXp[n].out_con == NOC) {
            audio_connect_to_core(&FXp[n], sendChanID);
            if(cpuid == 0) {
                printf("Connected FX ID %d to NoC Chanel ID %d\n", *FXp[n].fx_id, sendChanID);
            }
        }

        //same core
        if(*FXp[n].out_con == SAME) {
            //search for effect in this core with same receive channel
            int con_same_bool = 0;
            for(int m=0; m<FX_HERE; m++) {
                if( (m != n) && (*FXp[m].in_con == SAME) &&
                    (RECV_ARRAY[*FXp[m].fx_id][sendChanID] == 1) ) {
                    con_same_bool = 1; //connection found!
                    audio_connect_same_core(&FXp[n], &FXp[m]);
                    if(cpuid == 0) {
                        printf("Connected FX ID %d to FX ID %d\n", *FXp[n].fx_id, *FXp[m].fx_id);
                    }
                    break;
                }
            }
            if( (cpuid == 0) && (con_same_bool == 0) ) {
                printf("ERROR: Same core connection mismatch: no destination found\n");
            }
        }

        //print output effects
        if( (cpuid == 0) && (*FXp[n].out_con == LAST) ) {
            printf("LAST: ID=%d\n", *FXp[n].fx_id);
        }
    }

    return 0;
}


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

    allocFX(FXp, FX_HERE, cpuid);

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

    //free memory allocation
    for(int n=0; n<FX_HERE; n++) {
        free_audio_vars(&FXp[n]);
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

    allocFX(FXp, FX_HERE, cpuid);

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

    /*
    printf("SOME INFO: \n");
    printf("FX_HERE=%d\n", FX_HERE);
    printf("fx_id: %d, cpuid: %d, is_fst:%u, is_lst: %u, in_con: %u, out_con: %u\n", *FXp[0].fx_id, *FXp[0].cpuid, *FXp[0].is_fst, *FXp[0].is_lst, *FXp[0].in_con, *FXp[0].out_con);
    printf("x_pnt=0x%x, y_pnt=0x%x, pt=%u, p=%u, rpr=%u, spr=%u, ppsr=%u, xb_size=%u, yb_size=%u, fx=%u, fx_pnt=0x%x\n", *FXp[0].x_pnt, *FXp[0].y_pnt, *FXp[0].pt, *FXp[0].p, *FXp[0].rpr, *FXp[0].spr, *FXp[0].ppsr, *FXp[0].xb_size, *FXp[0].yb_size, *FXp[0].fx, *FXp[0].fx_pnt);
    */


    //CPU cycles stuff
    int CPUcycles[LIM] = {0};
    unsigned int cpu_pnt = 0;


    //int wait_recv = 18; //amount of loops until audioOut is done
    //int wait_recv = LATENCY;
    //for debugging
    //const int WAIT = wait_recv;

    //short audio_in[LIM][2] = {0};
    //short audio_out[LIM][2] = {0};

    while(*keyReg != 3) {

        for(int n=0; n<FX_HERE; n++) {
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

        //printf("in: %d, %d       out: %d, %d\n", FXp[0].x[0], FXp[0].x[1], FXp[FX_HERE-1].y[0], FXp[FX_HERE-1].y[1]);


        //store CPU Cycles
        CPUcycles[cpu_pnt] = get_cpu_cycles();
        cpu_pnt++;
        if(cpu_pnt == LIM) {
            //break;
            cpu_pnt = 0;
        }


    }

    //free memory allocation
    for(int n=0; n<FX_HERE; n++) {
        free_audio_vars(&FXp[n]);
    }

    //exit stuff
    printf("exit here!\n");
    exit = 1;
    printf("waiting for all threads to finish...\n");

    /*
    for(int i=1; i<LIM; i++) {
        printf("%d\n", (CPUcycles[i]-CPUcycles[i-1]));
    }
    */

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
