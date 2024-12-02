#include "libaudio/audio.h"
#include "libaudio/audio.c"

const int LIM = 1000;

int allocFX(struct AudioFX *FXp, int *FX_HERE, int cpuid, int mode,
    volatile _UNCACHED int *send_chans_conP, volatile _UNCACHED int *recv_chans_conP) {

    //read current FX_SCHED, SEND_ARRAY and RECV_ARRAY
    int FX_SCHED[MAX_FX][8];
    int SEND_ARRAY[MAX_FX][CHAN_AMOUNT];
    int RECV_ARRAY[MAX_FX][CHAN_AMOUNT];
    for(int fx=0; fx<FX_AMOUNT[mode]; fx++) {
        for(int col=0; col<8; col++) { //FX_SCHED first
            FX_SCHED[fx][col] = *((FX_SCHED_P[mode]) + fx*8 + col);
        }
        for(int ch=0; ch<CHAN_AMOUNT; ch++) { //then, SEND_ARRAY and RECV_ARRAY
            SEND_ARRAY[fx][ch] = *((SEND_ARRAY_P[mode]) + fx*CHAN_AMOUNT + ch);
            RECV_ARRAY[fx][ch] = *((RECV_ARRAY_P[mode]) + fx*CHAN_AMOUNT + ch);
        }
    }

    //struct parameters:
    int fx_id;
    fx_t fx_type;
    int xb_size, yb_size, s;
    con_t in_con, out_con;
    int recv_am, send_am;

    // READ FROM SCHEDULER
    int fx_ind = 0;
    for(int n=0; n<FX_AMOUNT[mode]; n++) {

        if(FX_SCHED[n][1] == cpuid) { //same core
            //one more FX on this core
            FX_HERE[mode]++;
            //assign parameters from SCHEDULER
            fx_id   =         FX_SCHED[n][0];
            fx_type = (fx_t)  FX_SCHED[n][2];
            xb_size =         FX_SCHED[n][3];
            yb_size =         FX_SCHED[n][4];
            s       =         FX_SCHED[n][5];
            in_con  = (con_t) FX_SCHED[n][6];
            out_con = (con_t) FX_SCHED[n][7];
            recv_am = 0;
            send_am = 0;
            for(int ch=0; ch<CHAN_AMOUNT; ch++) {
                if(RECV_ARRAY[n][ch] == 1) {
                    recv_am++;
                }
                if(SEND_ARRAY[n][ch] == 1) {
                    send_am++;
                }
            }
            if(recv_am == 0) { recv_am = 1; } //there is always at least 1
            if(send_am == 0) { send_am = 1; } //there is always at least 1
            //allocate
            if(alloc_audio_vars(&FXp[fx_ind], fx_id, fx_type, in_con,
                    out_con, recv_am, send_am, xb_size, yb_size, s, LATENCY[mode]) == 1) {
                if(cpuid == 0) {
                    printf("ERROR: allocation failed: not enough space on SPM\n");
                }
                return 1;
            }
            fx_ind++;
        }
    }

    //CONNECT EFFECTS
    for(int n=0; n<FX_HERE[mode]; n++) {
        //print input effects
        if( (cpuid == 0) && (*FXp[n].in_con == FIRST) ) {
            printf("FIRST: ID=%d\n", *FXp[n].fx_id);
        }

        // OUTPUT
        int recvChanID[*FXp[n].recv_am]; // channel ID to connect from
        int sendChanID[*FXp[n].send_am]; // channel ID to connect to
        int recv_pnt = 0; //amount of receive channels (for joins)
        int send_pnt = 0; //amount of send    channels (for forks)
        for(int ch=0; ch<CHAN_AMOUNT; ch++) {
            if(RECV_ARRAY[*FXp[n].fx_id][ch] == 1) {
                recvChanID[recv_pnt] = ch; //found channel ID to connect from
                recv_pnt++;
            }
            if(SEND_ARRAY[*FXp[n].fx_id][ch] == 1) {
                sendChanID[send_pnt] = ch; //found channel ID to connect to
                send_pnt++;
            }
        }
        //NoC
        if(*FXp[n].in_con == NOC) {
            for(int r=0; r<*FXp[n].recv_am; r++) {
                if(audio_connect_from_core(recvChanID[r], &FXp[n], r) == 1) {
                    return 1;
                }
                if(cpuid == 0) {
                    printf("Connected NoC Chanel ID %d to FX ID %d\n", recvChanID[r], *FXp[n].fx_id);
                }
                *recv_chans_conP += 1;
            }
        }
        if(*FXp[n].out_con == NOC) {
            for(int s=0; s<*FXp[n].send_am; s++) {
                if(audio_connect_to_core(&FXp[n], sendChanID[s], s) == 1) {
                    return 1;
                }
                if(cpuid == 0) {
                    printf("Connected FX ID %d to NoC Chanel ID %d\n", *FXp[n].fx_id, sendChanID[s]);
                }
                *send_chans_conP += 1;
            }
        }
        //same core
        if(*FXp[n].out_con == SAME) {
            //search for effect in this core with same receive channel
            int con_same_bool = 0;
            for(int m=0; m<FX_HERE[mode]; m++) {
                if( (m != n) && (*FXp[m].in_con == SAME) &&
                    (RECV_ARRAY[*FXp[m].fx_id][sendChanID[0]] == 1) ) { //there is just one output
                    con_same_bool = 1; //connection found!
                    if(audio_connect_same_core(&FXp[n], &FXp[m]) == 1) {
                        return 1;
                    }
                    if(cpuid == 0) {
                        printf("Connected FX ID %d to FX ID %d\n", *FXp[n].fx_id, *FXp[m].fx_id);
                    }
                    *send_chans_conP += 1;
                    *recv_chans_conP += 1;
                    break;
                }
            }
            if( (cpuid == 0) && (con_same_bool == 0) ) {
                printf("ERROR: Same core connection mismatch: no destination found\n");
                return 1;
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
    volatile _UNCACHED int *exitP           = inArgs[0];
    volatile _UNCACHED int *current_modeP   = inArgs[1];
    volatile _UNCACHED int *allocsDoneP     = inArgs[2];
    volatile _UNCACHED int *send_chans_conP = inArgs[2+AUDIO_CORES];
    volatile _UNCACHED int *recv_chans_conP = inArgs[2+AUDIO_CORES+1];
    volatile _UNCACHED int *reconfigDoneP   = inArgs[2+AUDIO_CORES+2];


    *ledReg = 0;

    // -------------------ALLOCATE FX------------------//

    _SPM int *cpuid;
    cpuid = (_SPM int *) mp_alloc(sizeof(int));
    *cpuid = get_cpuid();

    //create structs
    struct AudioFX FXp[MODES][MAX_FX_PER_CORE[*cpuid]];

    int FX_HERE[MODES] = {0};

    //iterate through modes
    for(int mode=0; mode<MODES; mode++) {
        if(allocFX(FXp[mode], FX_HERE, *cpuid, mode, send_chans_conP, recv_chans_conP) == 1) {
            *exitP = 1;
        }
    }

    // wait until all cores are ready
    allocsDoneP[*cpuid] = 1;
    *ledReg = 1;
    for(int i=0; i<AUDIO_CORES; i++) {
        while(allocsDoneP[i] == 0);
    }


    // Initialize the communication channels
    int nocret = mp_init_ports();

    //copy of current_mode in the SPM
    _SPM unsigned int *cmode_spm;
    cmode_spm = (_SPM unsigned int *) mp_alloc(sizeof(unsigned int));
    *cmode_spm = MODES; //to force new mode

    //initial stuff
    while(reconfigDoneP[0] == 0); //wait until its 1
    *cmode_spm = *current_modeP;
    // wait until all cores see the new mode
    for(int i=0; i<AUDIO_CORES; i++) {
        if(i == *cpuid) {
            reconfigDoneP[i] = 1;
        }
        while(reconfigDoneP[i] == 0);
    }

    //-------------------PROCESS AUDIO------------------//

    //loop
    while(*exitP == 0) {

        //update current mode SPM
        if(*cmode_spm != *current_modeP) {
            *cmode_spm = *current_modeP;
            // wait until all cores see the new mode
            for(int i=0; i<AUDIO_CORES; i++) {
                if(i == *cpuid) {
                    reconfigDoneP[i] = 1;
                }
                while(reconfigDoneP[i] == 0);
            }
        }

        for(int n=0; n<FX_HERE[*cmode_spm]; n++) {
            audio_process(&FXp[*cmode_spm][n]);
        }

    }

    //free memory allocation
    for(int mode=0; mode<MODES; mode++) {
        for(int n=0; n<FX_HERE[mode]; n++) {
            free_audio_vars(&FXp[mode][n]);
        }
    }

    // exit with return value
    int ret = 0;
    *ledReg = 0;
    corethread_exit(&ret);
    return;
}



int main() {

    //arguments to thread 1 function
    int exit = 0;
    int allocsDone[AUDIO_CORES] = {0};
    int send_chans_con = 0;
    int recv_chans_con = 0;
    int reconfigDone[AUDIO_CORES] = {0};
    volatile _UNCACHED int *exitP           = (volatile _UNCACHED int *) &exit;
    volatile _UNCACHED int *current_modeP   = (volatile _UNCACHED int *) &current_mode;
    volatile _UNCACHED int *allocsDoneP     = (volatile _UNCACHED int *) &allocsDone;
    volatile _UNCACHED int *send_chans_conP = (volatile _UNCACHED int *) &send_chans_con;
    volatile _UNCACHED int *recv_chans_conP = (volatile _UNCACHED int *) &recv_chans_con;
    volatile _UNCACHED int *reconfigDoneP   = (volatile _UNCACHED int *) &reconfigDone;
    volatile _UNCACHED int (*threadFunc_args[2+AUDIO_CORES+2+AUDIO_CORES]);
    threadFunc_args[0] = exitP;
    threadFunc_args[1] = current_modeP;
    threadFunc_args[2] = allocsDoneP;
    threadFunc_args[2+AUDIO_CORES] = send_chans_conP;
    threadFunc_args[2+AUDIO_CORES+1] = recv_chans_conP;
    threadFunc_args[2+AUDIO_CORES+2] = reconfigDoneP;

    *ledReg = 0;

    //check if amount of FX cores exceeds available cores
    if(AUDIO_CORES > NOC_CORES) {
        printf("ERROR: need %d audio cores, but current platform has %d\n", AUDIO_CORES, NOC_CORES);
        exit = 1;
    }

    printf("starting thread and NoC channels...\n");
    //set thread function and start thread
    int threads[AUDIO_CORES-1];
    for(int i=0; i<(AUDIO_CORES-1); i++) {
        threads[i] = (corethread_t) (i+1);
        if (corethread_create(threads[i], &threadFunc, (void*) threadFunc_args) != 0) {
            printf("ERROR: Thread %d was not creaded correctly\n", i+1);
            exit = 1;
        }
        printf("Thread created on core %d\n", i+1);
    }

    #if GUITAR == 1
    setup(1); //for guitar
    #else
    setup(0); //for volca
    #endif

    // enable input
    *audioAdcEnReg = 1;
    //let input buffer fill in before starting to output
    for(unsigned int i=0; i<(BUFFER_SIZE * 1536); i++) { //wait for BUFFER_SIZE samples
        *audioDacEnReg = 0;
    }
    //finally, enable output
    *audioDacEnReg = 1;

    setInputBufferSize(BUFFER_SIZE);
    setOutputBufferSize(BUFFER_SIZE);



    // -------------------ALLOCATE FX------------------//

    int cpuid = get_cpuid();

    //create structs
    struct AudioFX FXp[MODES][MAX_FX_PER_CORE[cpuid]];

    int FX_HERE[MODES] = {0};

    //iterate through modes
    for(int mode=0; mode<MODES; mode++) {
        printf("**\n**************** ALLOCATING FX OF MODE %d ****************\n**\n", mode);
        if(allocFX(FXp[mode], FX_HERE, cpuid, mode, send_chans_conP, recv_chans_conP) == 1) {
            printf("ERROR DURING FX ALLOCATION\n");
            exit = 1;
        }
    }

    // wait until all cores are ready
    allocsDoneP[cpuid] = 1;
    *ledReg = 1;
    printf("waiting for all cores to finish allocation... (look at the LEDs)\n");
    for(int i=0; i<AUDIO_CORES; i++) {
        if(*exitP == 0) {
            while(allocsDoneP[i] == 0);
        }
    }
    printf("all cores finished allocation!\n");

    //check if all NoC channels have been connected correctly
    if(*exitP == 0) {
        if(*send_chans_conP != CHAN_AMOUNT) {
            printf("ERROR: NoC Channel connection unbalanced: %d channels, but have %d sources\n",
                CHAN_AMOUNT, *send_chans_conP);
            exit = 1;
        }
        if(*recv_chans_conP != CHAN_AMOUNT) {
            printf("ERROR: NoC Channel connection unbalanced: %d channels, but have %d destinations\n",
                CHAN_AMOUNT, *recv_chans_conP);
            exit = 1;
        }
    }

    // Initialize the communication channels
    if(*exitP == 0) {
        printf("gonna initialise NoC channels...\n");
        if(mp_init_ports() == 1) {
            printf("Thread and NoC initialised correctly\n");
        }
        else {
            printf("ERROR: Problem with NoC initialisation\n");
            exit = 1;
        }
    }

    //CPU cycles stuff
    int CPUcycles[LIM] = {0};
    unsigned int cpu_pnt = 0;

    //copy of current_mode in the SPM
    _SPM unsigned int *cmode_spm;
    cmode_spm = (_SPM unsigned int *) mp_alloc(sizeof(unsigned int));
    *cmode_spm = *current_modeP;
#ifdef NOC_RECONFIG
    //reconfiguration function
    noc_sched_set(*cmode_spm);
#endif
    //previous keyReg value
    _SPM unsigned int *keyReg_prev;
    keyReg_prev = (_SPM unsigned int *) mp_alloc(sizeof(unsigned int));
    *keyReg_prev = *keyReg;

    //only process if all initialisation was done correctly
    if(*exitP == 1) {
        printf("ERROR: ALLOCATION FAILED IN ONE OF THE SLAVE CORES\n");
    }
    if(*exitP == 0) {
    //-------------------PROCESS AUDIO------------------//
        printf("READY TO PLAY!!!!\n");


        // all cores should sync here
        for(int i=0; i<AUDIO_CORES; i++) {
            if(i == 0) {
                reconfigDoneP[0] = 1;
            }
            while(reconfigDoneP[i] == 0);
        }
        printf("starting\n");
        for(int i=0; i<AUDIO_CORES; i++) {
            reconfigDoneP[i] = 0;
        }

        while(*keyReg != 3) {

            //check if there is a mode change
            if( (*keyReg == 14) && (*keyReg != *keyReg_prev) ) {
                *current_modeP = (*current_modeP + 1) % MODES;
                //update current mode SPM
                *cmode_spm = *current_modeP;
                printf("mode: %d\n", *cmode_spm);
                //reset latency
                *FXp[*cmode_spm][FX_HERE[*cmode_spm]-1].last_count = 0;
                *FXp[*cmode_spm][FX_HERE[*cmode_spm]-1].last_init = 1;

                // wait until all cores see the new mode
                for(int i=0; i<AUDIO_CORES; i++) {
                    if(i == 0) {
                        reconfigDoneP[0] = 1;
                    }
                    while(reconfigDoneP[i] == 0);
                }
                for(int i=0; i<AUDIO_CORES; i++) {
                    reconfigDoneP[i] = 0;
                }
#ifdef NOC_RECONFIG
                //reconfiguration function
                noc_sched_set(*cmode_spm);
#endif

            }
            if( (*keyReg == 13) && (*keyReg != *keyReg_prev) ) {
                if(*current_modeP == 0) {
                    *current_modeP = MODES - 1;
                }
                else {
                    *current_modeP = (*current_modeP - 1) % MODES;
                }
                //update current mode SPM
                *cmode_spm = *current_modeP;
                printf("mode: %d\n", *cmode_spm);
                //reset latency
                *FXp[*cmode_spm][FX_HERE[*cmode_spm]-1].last_count = 0;
                *FXp[*cmode_spm][FX_HERE[*cmode_spm]-1].last_init = 1;

                // wait until all cores see the new mode
                for(int i=0; i<AUDIO_CORES; i++) {
                    if(i == 0) {
                        reconfigDoneP[0] = 1;
                    }
                    while(reconfigDoneP[i] == 0);
                }
                for(int i=0; i<AUDIO_CORES; i++) {
                    reconfigDoneP[i] = 0;
                }
#ifdef NOC_RECONFIG
                //reconfiguration function
                noc_sched_set(*cmode_spm);
#endif

            }
            *keyReg_prev = *keyReg;


            //printf("******* ITERATION %d *******\n", cpu_pnt);

            for(int n=0; n<FX_HERE[*cmode_spm]; n++) {
                audio_process(&FXp[*cmode_spm][n]);
            }

            //printf("in: %d, %d       out: %d, %d    %d, %d\n", FXp[0].x[0], FXp[0].x[1], FXp[FX_HERE-1].x[0], FXp[FX_HERE-1].x[1], FXp[FX_HERE-1].y[0], FXp[FX_HERE-1].y[1]);

            //store CPU Cycles
            CPUcycles[cpu_pnt] = get_cpu_cycles();
            cpu_pnt++;
            if(cpu_pnt == LIM) {
                //break;
                cpu_pnt = 0;
            }

        }
    }

    //free memory allocation
    for(int mode=0; mode<MODES; mode++) {
        for(int n=0; n<FX_HERE[mode]; n++) {
            free_audio_vars(&FXp[mode][n]);
        }
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
    //join with thread 1

    int *retval;
    for(int i=0; i<(AUDIO_CORES-1); i++) {
        corethread_join(threads[i], (void **)&retval);
        printf("thread %d finished!\n", (i+1));
    }

    *ledReg = 0;

    return 0;
}
