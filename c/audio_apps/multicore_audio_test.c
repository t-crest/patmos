/*
#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
*/

#include "libaudio/audio.h"
#include "libaudio/audio.c"

//DEBUG STUFF
const int DEBUG_ELEMENTS = 2;
const int DEBUG_LOOPLENGTH = 60;
const int PARAM_AMOUNT = 5;

//master core
const int NOC_MASTER = 0;

/*
//how many cores take part in the audio system
const int AUDIO_CORES = 3;
//how many effects are on the system in total
const int FX_AMOUNT = 6;
// FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | P (S) | IN_TYPE | OUT_TYPE | FROM_ID | TO_ID //
const int FX_SCHED[FX_AMOUNT][10] = {
    {0, 0, 0, 8, 8, 1, 0, 0, -1,  1},
    {1, 0, 1, 8, 8, 8, 0, 1,  0,  0},
    {2, 1, 0, 8, 8, 1, 1, 0,  0,  3},
    {3, 1, 0, 8, 8, 1, 0, 1,  2,  1},
    {4, 2, 0, 8, 8, 1, 1, 1,  1,  2},
    {5, 0, 0, 8, 8, 1, 1, 0,  2, -1}
};
*/
/*
//how many cores take part in the audio system
const int AUDIO_CORES = 3;
//how many effects are on the system in total
const int FX_AMOUNT = 5;
// FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | P (S) | IN_TYPE | OUT_TYPE | FROM_ID | TO_ID //
const int FX_SCHED[FX_AMOUNT][10] = {
    {0, 0, 0, 8, 8, 1, 0, 0, -1,  1},
    {1, 0, 1, 8, 8, 8, 0, 1,  0,  0},
    {2, 1, 0, 8, 1, 1, 1, 1,  0,  1},
    {3, 2, 0, 1, 8, 1, 1, 1,  1,  2},
    {4, 0, 0, 8, 8, 1, 1, 0,  2, -1}
};
*/

//how many cores take part in the audio system
const int AUDIO_CORES = 4;
//how many effects are on the system in total
const int FX_AMOUNT = 5;
// FX_ID | CORE | FX_TYPE | XB_SIZE | YB_SIZE | P (S) | IN_TYPE | OUT_TYPE | FROM_ID | TO_ID //
const int FX_SCHED[FX_AMOUNT][10] = {
    {0, 0, 0, 1, 1, 1, 0, 1, -1,  0},
    {1, 1, 0, 1, 8, 1, 1, 1,  0,  1},
    {2, 2, 1, 8, 8, 8, 1, 1,  1,  2},
    {3, 3, 0, 8, 1, 1, 1, 1,  2,  3},
    {4, 0, 0, 1, 1, 1, 1, 0,  3, -1}
};



void threadFunc(void* args) {
    volatile _UNCACHED int **inArgs = (volatile _UNCACHED int **) args;
    volatile _UNCACHED int *exitP      = inArgs[0];
    volatile _UNCACHED int *allocsDoneP = inArgs[1];


    /*
      AUDIO STUFF HERE
    */

    int cpuid = get_cpuid();


    volatile _UNCACHED int *sendsP = inArgs[3 + DEBUG_LOOPLENGTH*DEBUG_ELEMENTS*((cpuid-1)*3)];
    volatile _UNCACHED int *recvsP = inArgs[3 + DEBUG_LOOPLENGTH*DEBUG_ELEMENTS*((cpuid-1)*3+1)];
    volatile _UNCACHED int *acksP = inArgs[3 + DEBUG_LOOPLENGTH*DEBUG_ELEMENTS*((cpuid-1)*3+2)];

    volatile _UNCACHED int *paramsP = inArgs[3 + DEBUG_LOOPLENGTH*DEBUG_ELEMENTS*9 + ((cpuid-1)*PARAM_AMOUNT)];

    /*
    //index:[3][1]
    *(sendsP+DEBUG_ELEMENTS*3+1) = 22*cpuid;
    *(recvsP+DEBUG_ELEMENTS*3+1) = 33*cpuid;
    *(acksP+DEBUG_ELEMENTS*3+1) = 44*cpuid;
    */

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

    //params debugging
    /*
    paramsP[0] = *FXp[0].in_con;
    paramsP[1] = *FXp[0].out_con;
    paramsP[2] = *FXp[0].fx_id;
    paramsP[3] = *FXp[0].is_fst;
    paramsP[4] = *FXp[0].is_lst;
    */

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


    paramsP[0] = *FXp[0].rpr;
    paramsP[1] = *FXp[0].spr;
    paramsP[2] = *FXp[0].p;
    paramsP[3] = ((qpd_t *)*FXp[0].recvChanP)->buf_size;
    paramsP[4] = ((qpd_t *)*FXp[0].sendChanP)->buf_size;


    //loop
    //audioValuesP[0] = 0;
    //int i=0;
    //while(*exitP == 0) {
    //i++;
    for(int i=0; i<DEBUG_LOOPLENGTH; i++) {

        for(int j=0; j<DEBUG_ELEMENTS; j++) {
            *(sendsP+DEBUG_ELEMENTS*i+j) = 0;
            *(recvsP+DEBUG_ELEMENTS*i+j) = 0;
            *(acksP+DEBUG_ELEMENTS*i+j) = 0;
        }


        for(int n=0; n<FX_HERE; n++) {
            if (audio_process(&FXp[n], (sendsP+DEBUG_ELEMENTS*i), (recvsP+DEBUG_ELEMENTS*i), (acksP+DEBUG_ELEMENTS*i)) == 1) {
                //timeout stuff here
            }
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

    //arguments to thread 1 function
    int exit = 0;
    int allocsDone[2] = {0, 0};
    volatile _UNCACHED int *exitP = (volatile _UNCACHED int *) &exit;
    volatile _UNCACHED int *allocsDoneP = (volatile _UNCACHED int *) &allocsDone;
    volatile _UNCACHED int (*threadFunc_args[3 + DEBUG_LOOPLENGTH*DEBUG_ELEMENTS*3*3 + 3*PARAM_AMOUNT]);
    threadFunc_args[0] = exitP;
    threadFunc_args[1] = allocsDoneP;

    int sends1[DEBUG_LOOPLENGTH][DEBUG_ELEMENTS] = {0};
    int recvs1[DEBUG_LOOPLENGTH][DEBUG_ELEMENTS] = {0};
    int acks1[DEBUG_LOOPLENGTH][DEBUG_ELEMENTS] = {0};
    int sends2[DEBUG_LOOPLENGTH][DEBUG_ELEMENTS] = {0};
    int recvs2[DEBUG_LOOPLENGTH][DEBUG_ELEMENTS] = {0};
    int acks2[DEBUG_LOOPLENGTH][DEBUG_ELEMENTS] = {0};
    int sends3[DEBUG_LOOPLENGTH][DEBUG_ELEMENTS] = {0};
    int recvs3[DEBUG_LOOPLENGTH][DEBUG_ELEMENTS] = {0};
    int acks3[DEBUG_LOOPLENGTH][DEBUG_ELEMENTS] = {0};

    int params1[PARAM_AMOUNT] = {0};
    int params2[PARAM_AMOUNT] = {0};
    int params3[PARAM_AMOUNT] = {0};

    volatile _UNCACHED int *sends1P = (volatile _UNCACHED int *) &sends1[0][0];
    volatile _UNCACHED int *recvs1P = (volatile _UNCACHED int *) &recvs1[0][0];
    volatile _UNCACHED int *acks1P = (volatile _UNCACHED int *) &acks1[0][0];
    volatile _UNCACHED int *sends2P = (volatile _UNCACHED int *) &sends2[0][0];
    volatile _UNCACHED int *recvs2P = (volatile _UNCACHED int *) &recvs2[0][0];
    volatile _UNCACHED int *acks2P = (volatile _UNCACHED int *) &acks2[0][0];
    volatile _UNCACHED int *sends3P = (volatile _UNCACHED int *) &sends3[0][0];
    volatile _UNCACHED int *recvs3P = (volatile _UNCACHED int *) &recvs3[0][0];
    volatile _UNCACHED int *acks3P = (volatile _UNCACHED int *) &acks3[0][0];

    volatile _UNCACHED int *params1P = (volatile _UNCACHED int *) &params1[0];
    volatile _UNCACHED int *params2P = (volatile _UNCACHED int *) &params2[0];
    volatile _UNCACHED int *params3P = (volatile _UNCACHED int *) &params3[0];

    threadFunc_args[3] = (volatile _UNCACHED int *)sends1P;
    threadFunc_args[3+DEBUG_LOOPLENGTH*DEBUG_ELEMENTS] = (volatile _UNCACHED int *)recvs1P;
    threadFunc_args[3+DEBUG_LOOPLENGTH*DEBUG_ELEMENTS*2] = (volatile _UNCACHED int *)acks1P;
    threadFunc_args[3+DEBUG_LOOPLENGTH*DEBUG_ELEMENTS*3] = (volatile _UNCACHED int *)sends2P;
    threadFunc_args[3+DEBUG_LOOPLENGTH*DEBUG_ELEMENTS*4] = (volatile _UNCACHED int *)recvs2P;
    threadFunc_args[3+DEBUG_LOOPLENGTH*DEBUG_ELEMENTS*5] = (volatile _UNCACHED int *)acks2P;
    threadFunc_args[3+DEBUG_LOOPLENGTH*DEBUG_ELEMENTS*6] = (volatile _UNCACHED int *)sends3P;
    threadFunc_args[3+DEBUG_LOOPLENGTH*DEBUG_ELEMENTS*7] = (volatile _UNCACHED int *)recvs3P;
    threadFunc_args[3+DEBUG_LOOPLENGTH*DEBUG_ELEMENTS*8] = (volatile _UNCACHED int *)acks3P;

    threadFunc_args[3+DEBUG_LOOPLENGTH*DEBUG_ELEMENTS*9] = (volatile _UNCACHED int *)params1P;
    threadFunc_args[3+DEBUG_LOOPLENGTH*DEBUG_ELEMENTS*9+PARAM_AMOUNT] = (volatile _UNCACHED int *)params2P;
    threadFunc_args[3+DEBUG_LOOPLENGTH*DEBUG_ELEMENTS*9+PARAM_AMOUNT*2] = (volatile _UNCACHED int *)params3P;


    printf("starting thread and NoC channels...\n");
    //set thread function and start thread
    corethread_t threads[AUDIO_CORES-1];
    for(int i=0; i<(AUDIO_CORES-1); i++) {
        threads[i] = (corethread_t) (i+1);
        corethread_create(&threads[i], &threadFunc, (void*) threadFunc_args);
        printf("Thread created on core %d\n", i+1);
    }

    int sends0[DEBUG_LOOPLENGTH][DEBUG_ELEMENTS] = {0};
    int recvs0[DEBUG_LOOPLENGTH][DEBUG_ELEMENTS] = {0};
    int acks0[DEBUG_LOOPLENGTH][DEBUG_ELEMENTS] = {0};
    volatile _UNCACHED int *sendsP = (volatile _UNCACHED int *) &sends0[0][0];
    volatile _UNCACHED int *recvsP = (volatile _UNCACHED int *) &recvs0[0][0];
    volatile _UNCACHED int *acksP = (volatile _UNCACHED int *) &acks0[0][0];


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



    //audio_connect_same_core(&FXp[0], &FXp[1]); //effects on same core
    //audio_connect_to_core(&FXp[1], 0); // to corresponding channelID
    //audio_connect_from_core(1, &FXp[2]); // from corresponding channelID




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
    //int CPUcycles[1000] = {0};
    //int cpu_pnt = 0;


    int wait_recv = 18; //amount of loops until audioOut is done


    //int i=0;
    //while(*keyReg != 3) {
    //i++;
    int i1 = 0;
    int i2 = 0;
    int i3 = 0;
    for(int i=0; i<DEBUG_LOOPLENGTH; i++) {


        printf("@ loop %d\n", i);
        int shown1 = sends1[i1][0];
        int shown2 = sends2[i2][0];
        int shown3 = sends3[i3][0];
        printf("sends done by core 1 at i1=%d: %d\n", shown1, i1);
        printf("sends done by core 2 at i2=%d: %d\n", shown2, i2);
        printf("sends done by core 3 at i3=%d: %d\n", shown3, i3);

        if(shown1 == 1) {
            i1++;
        }
        if(shown2 == 1) {
            i2++;
        }
        if(shown3 == 8) {
            i3++;
        }


        //init
        sends0[i][0] = 0;
        recvs0[i][0] = 0;
        acks0[i][0] = 0;


        for(int n=0; n<FX_HERE; n++) {
            printf("n=%d\n", n);
            //process
            //int cycles = get_cpu_cycles();

            if( (*FXp[n].is_lst == NO_LAST) || (wait_recv == 0) ) {
                audio_process(&FXp[n], (sendsP+DEBUG_ELEMENTS*i), (recvsP+DEBUG_ELEMENTS*i), (acksP+DEBUG_ELEMENTS*i));
            }
            else {
                wait_recv--;
            }

            /*
              cycles = get_cpu_cycles() - cycles;
              printf("cpu cycles: %d\n", cycles);
            */
        }



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


        //audio_process(audio0bP);
        //audio_process(&FXp[1]);




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

        /*
        if(first == 0) {
            //audio_process(audio0cP);
            audio_process(&FXp[2]);
            / *
            printf("\noutput data c: \n");
            yP = (volatile _SPM short *)*audio0cP->y_pnt;
            for(unsigned int i=0; i < *audio0cP->yb_size; i++) {
                printf("%d, %d    ", yP[i*2], yP[i*2+1]);
            }
            * /
        }
        else {
            first = 0;
        }
        */

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
    printf("waiting for all threads to finish...\n");

    /*
    for(int i=1; i<1000; i++) {
        printf("%d\n", (CPUcycles[i]-CPUcycles[i-1]));
    }
    */

    //join with thread 1

    int *retval;
    for(int i=0; i<(AUDIO_CORES-1); i++) {
        //corethread_join(threads[i], (void **)&retval);
        printf("thread %d finished!\n", (i+1));
    }




    printf("\n\n\n");

    for(int i=0; i<DEBUG_LOOPLENGTH; i++) {
        printf("@ TIME %u: i=%d: core 0 recved %d times\n", recvs0[i][1], i, recvs0[i][0]);
        printf("@ TIME %u: i=%d: core 0 acked  %d times\n", acks0[i][1], i, acks0[i][0]);
        printf("@ TIME %u: i=%d: core 0 sent   %d times\n", sends0[i][1], i, sends0[i][0]);
        printf("@ TIME %u: i=%d: core 1 recved %d times\n", recvs1[i][1], i, recvs1[i][0]);
        printf("@ TIME %u: i=%d: core 1 acked  %d times\n", acks1[i][1], i, acks1[i][0]);
        printf("@ TIME %u: i=%d: core 1 sent   %d times\n", sends1[i][1], i, sends1[i][0]);
        printf("@ TIME %u: i=%d: core 2 recved %d times\n", recvs2[i][1], i, recvs2[i][0]);
        printf("@ TIME %u: i=%d: core 2 acked  %d times\n", acks2[i][1], i, acks2[i][0]);
        printf("@ TIME %u: i=%d: core 2 sent   %d times\n", sends2[i][1], i, sends2[i][0]);
        printf("@ TIME %u: i=%d: core 3 recved %d times\n", recvs3[i][1], i, recvs3[i][0]);
        printf("@ TIME %u: i=%d: core 3 acked  %d times\n", acks3[i][1], i, acks3[i][0]);
        printf("@ TIME %u: i=%d: core 3 sent   %d times\n", sends3[i][1], i, sends3[i][0]);
        printf("\n");
    }



    //SORTING
    int stepsArray[DEBUG_LOOPLENGTH*3*4][DEBUG_ELEMENTS+3];
    for(int i=0; i<DEBUG_LOOPLENGTH; i++) { // iterations
        for(int j=0; j<DEBUG_ELEMENTS; j++) { //exec times and cpu times
            //core 0
            stepsArray[0*DEBUG_LOOPLENGTH+i][j] = recvs0[i][j];
            stepsArray[1*DEBUG_LOOPLENGTH+i][j] = acks0[i][j];
            stepsArray[2*DEBUG_LOOPLENGTH+i][j] = sends0[i][j];
            //core 1
            stepsArray[3*DEBUG_LOOPLENGTH+i][j] = recvs1[i][j];
            stepsArray[4*DEBUG_LOOPLENGTH+i][j] = acks1[i][j];
            stepsArray[5*DEBUG_LOOPLENGTH+i][j] = sends1[i][j];
            //core 2
            stepsArray[6*DEBUG_LOOPLENGTH+i][j] = recvs2[i][j];
            stepsArray[7*DEBUG_LOOPLENGTH+i][j] = acks2[i][j];
            stepsArray[8*DEBUG_LOOPLENGTH+i][j] = sends2[i][j];
            //core 3
            stepsArray[9*DEBUG_LOOPLENGTH+i][j] = recvs3[i][j];
            stepsArray[10*DEBUG_LOOPLENGTH+i][j] = acks3[i][j];
            stepsArray[11*DEBUG_LOOPLENGTH+i][j] = sends3[i][j];

        }
        //core 0
        stepsArray[0*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS]   = i;
        stepsArray[0*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS+1] = 0; //core 0
        stepsArray[0*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS+2] = 0; //type: recv
        stepsArray[1*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS]   = i;
        stepsArray[1*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS+1] = 0; //core 0
        stepsArray[1*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS+2] = 1; //type: ack
        stepsArray[2*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS]   = i;
        stepsArray[2*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS+1] = 0; //core 0
        stepsArray[2*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS+2] = 2; //type: send
        //core 1
        stepsArray[3*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS]   = i;
        stepsArray[3*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS+1] = 1; //core 1
        stepsArray[3*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS+2] = 0; //type: recv
        stepsArray[4*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS]   = i;
        stepsArray[4*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS+1] = 1; //core 1
        stepsArray[4*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS+2] = 1; //type: ack
        stepsArray[5*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS]   = i;
        stepsArray[5*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS+1] = 1; //core 1
        stepsArray[5*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS+2] = 2; //type: send
        //core 2
        stepsArray[6*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS]   = i;
        stepsArray[6*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS+1] = 2; //core 2
        stepsArray[6*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS+2] = 0; //type: recv
        stepsArray[7*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS]   = i;
        stepsArray[7*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS+1] = 2; //core 2
        stepsArray[7*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS+2] = 1; //type: ack
        stepsArray[8*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS]   = i;
        stepsArray[8*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS+1] = 2; //core 2
        stepsArray[8*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS+2] = 2; //type: send
        //core 2
        stepsArray[9*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS]   = i;
        stepsArray[9*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS+1] = 3; //core 3
        stepsArray[9*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS+2] = 0; //type: recv
        stepsArray[10*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS]   = i;
        stepsArray[10*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS+1] = 3; //core 3
        stepsArray[10*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS+2] = 1; //type: ack
        stepsArray[11*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS]   = i;
        stepsArray[11*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS+1] = 3; //core 3
        stepsArray[11*DEBUG_LOOPLENGTH+i][DEBUG_ELEMENTS+2] = 2; //type: send
    }

    //SORTING
    int swap[DEBUG_ELEMENTS+3];
    for (int c = 0 ; c < ( DEBUG_LOOPLENGTH*3*4 - 1 ); c++) {
        for (int d = 0 ; d < DEBUG_LOOPLENGTH*3*4 - c - 1; d++) {
            if (stepsArray[d][1] > stepsArray[d+1][1]) {
                for(int i=0; i<DEBUG_ELEMENTS+3; i++) {
                    swap[i] = stepsArray[d][i];
                    stepsArray[d][i]   = stepsArray[d+1][i];
                    stepsArray[d+1][i] = swap[i];
                }
            }
        }
    }



    printf("\n\n\n");

    for(int i=0; i<(DEBUG_LOOPLENGTH*3*4); i++) {
        if(stepsArray[i][1] != 0) {
            switch (stepsArray[i][4]) {
            case 0:
                printf("t=%u: i=%d: core=%d recved %d times\n", stepsArray[i][1], stepsArray[i][2], stepsArray[i][3], stepsArray[i][0]);
                break;
            case 1:
                printf("t=%u: i=%d: core=%d acked  %d times\n", stepsArray[i][1], stepsArray[i][2], stepsArray[i][3], stepsArray[i][0]);
                break;
            case 2:
                printf("t=%u: i=%d: core=%d sent   %d times\n", stepsArray[i][1], stepsArray[i][2], stepsArray[i][3], stepsArray[i][0]);
                break;
            }
        }
    }


    //params debugging
    printf("core 1: %d, %d, %d, %d, %d\n", params1[0], params1[1], params1[2], params1[3], params1[4]);
    printf("core 2: %d, %d, %d, %d, %d\n", params2[0], params2[1], params2[2], params2[3], params2[4]);
    printf("core 3: %d, %d, %d, %d, %d\n", params3[0], params3[1], params3[2], params3[3], params3[4]);

    printf("FXp[0], send buffer size: %d\n", ((qpd_t *)*FXp[0].sendChanP)->buf_size);
    printf("FXp[1], recv buffer size: %d\n", ((qpd_t *)*FXp[1].recvChanP)->buf_size);





    return 0;
}
