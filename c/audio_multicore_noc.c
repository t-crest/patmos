#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "audio.h"
#include "audio.c"

const int NOC_MASTER = 0;

#define MP_CHAN_SHORTS_AMOUNT 2

#define MP_CHAN_1_ID 1
#define MP_CHAN_1_NUM_BUF 1
#define MP_CHAN_1_MSG_SIZE MP_CHAN_SHORTS_AMOUNT * 2 // 2 shorts = 4 bytes (actually always multiples of 4)

#define MP_CHAN_2_ID 2
#define MP_CHAN_2_NUM_BUF 1
#define MP_CHAN_2_MSG_SIZE MP_CHAN_SHORTS_AMOUNT * 2 // 2 shorts = 4 bytes


void thread1_delay(void* args) {
    volatile _UNCACHED int **inArgs = (volatile _UNCACHED int **) args;
    volatile _UNCACHED int *ledOnTimeP  = inArgs[0];
    volatile _UNCACHED int *ledOffTimeP = inArgs[1];
    volatile _UNCACHED int *exitP       = inArgs[2];

    volatile _UNCACHED int *writeP_chan1P = inArgs[3];
    volatile _UNCACHED int *readP_chan1P  = inArgs[4];
    volatile _UNCACHED int *writeP_chan2P = inArgs[5];
    volatile _UNCACHED int *readP_chan2P  = inArgs[6];

    //MP:
    // Create the queuing ports
    qpd_t * chan1 = mp_create_qport(MP_CHAN_1_ID, SINK,
        MP_CHAN_1_MSG_SIZE, MP_CHAN_1_NUM_BUF);
    qpd_t * chan2 = mp_create_qport(MP_CHAN_2_ID, SOURCE,
        MP_CHAN_2_MSG_SIZE, MP_CHAN_2_NUM_BUF);

    // Initialize the communication channels
    int nocret = mp_init_ports();

    //receive from master and respond
    for(short i=0; i<10; i++) {

        //receive
        mp_recv(chan1,0);
        short received[2];
        for(int j=0; j<MP_CHAN_SHORTS_AMOUNT; j++) {
            received[j] = *((volatile _SPM short *)chan1->read_buf+j);
        }
        //acknowledge
        mp_ack(chan1,0);
        //set respond data
        for(int j=0; j<MP_CHAN_SHORTS_AMOUNT; j++) {
            *((volatile _SPM short *)chan2->write_buf+j) = received[j];
        }
        //respond
        mp_send(chan2,0);

    }


    while(*exitP == 0) {
        for(int i=0; i<(*ledOnTimeP * 100000); i++) {
            *ledReg = 1;
        }
        for(int i=0; i<(*ledOffTimeP * 100000); i++) {
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
    int ledOnTime = 15;
    int ledOffTime = 5;
    int exit = 0;
    volatile _UNCACHED int *ledOnTimeP = (volatile _UNCACHED int *) &ledOnTime;
    volatile _UNCACHED int *ledOffTimeP = (volatile _UNCACHED int *) &ledOffTime;
    volatile _UNCACHED int *exitP      = (volatile _UNCACHED int *) &exit;
    volatile _UNCACHED int (*thread1_args[7]);
    thread1_args[0] = ledOnTimeP;
    thread1_args[1] = ledOffTimeP;
    thread1_args[2] = exitP;

    int writeP_chan1 = 0;
    int readP_chan1  = 0;
    int writeP_chan2 = 0;
    int readP_chan2  = 0;
    volatile _UNCACHED int *writeP_chan1P = (volatile _UNCACHED int *) &writeP_chan1;
    volatile _UNCACHED int *readP_chan1P = (volatile _UNCACHED int *) &readP_chan1;
    volatile _UNCACHED int *writeP_chan2P = (volatile _UNCACHED int *) &writeP_chan2;
    volatile _UNCACHED int *readP_chan2P = (volatile _UNCACHED int *) &readP_chan2;
    thread1_args[3] = writeP_chan1P;
    thread1_args[4] = readP_chan1P;
    thread1_args[5] = writeP_chan2P;
    thread1_args[6] = readP_chan2P;
    printf("starting thread...\n");
    //set thread function and start thread
    corethread_create(&threadOne, &thread1_delay, (void*) thread1_args);

    // MP: create message passing ports
    qpd_t * chan1 = mp_create_qport(MP_CHAN_1_ID, SOURCE,
        MP_CHAN_1_MSG_SIZE, MP_CHAN_1_NUM_BUF);
    qpd_t * chan2 = mp_create_qport(MP_CHAN_2_ID, SINK,
        MP_CHAN_2_MSG_SIZE, MP_CHAN_2_NUM_BUF);

    printf("created MP channels\n");

    // Initialize the communication channels
    int nocret = mp_init_ports();
    // TODO: check on retval

    printf("Initialized buffers\n");

    printf("Buffer size of channel 1: %d\n", chan1->buf_size);
    printf("Buffer size of channel 2: %d\n", chan2->buf_size);

    short send_data[10][MP_CHAN_SHORTS_AMOUNT];
    short recv_data[10][MP_CHAN_SHORTS_AMOUNT];

    for(short i=0; i<10; i++) {
        printf("sending...\n");
        //send message from master to slave
        for(int j=0; j<MP_CHAN_SHORTS_AMOUNT; j++) {
            send_data[i][j] = (short)((i*5)+j);
            //printf("write buffer pointer is: 0x%x\n", (unsigned int)&(*((volatile _SPM short *)chan1->write_buf)));
            //printf("pos to write is: 0x%x\n", (unsigned int)&(*((volatile _SPM short *)chan1->write_buf+j)));
            *((volatile _SPM short *)chan1->write_buf+j) = send_data[i][j];;
        }

        /*
        *writeP_chan1P = (int)chan1->write_buf;
        *readP_chan1P  = (int)chan1->read_buf;
        *writeP_chan2P = (int)chan2->write_buf;
        *readP_chan2P  = (int)chan2->read_buf;
        printf("JUST BEFORE MASTER SENDS @ %d:\n", i);
        printf("chanel 1 write pointer: 0x%x\n", *writeP_chan1P);
        printf("chanel 1 read pointer: 0x%x\n", *readP_chan1P);
        printf("chanel 2 write pointer: 0x%x\n", *writeP_chan2P);
        printf("chanel 2 read pointer: 0x%x\n", *readP_chan2P);
        */

        mp_send(chan1,0);

        printf("Sent!\n");

        //receive message from slave
        mp_recv(chan2,0);

        for(int j=0; j<MP_CHAN_SHORTS_AMOUNT; j++) {
            recv_data[i][j] = *((volatile _SPM short *)chan2->read_buf+j);
        }

        // Acknowledge the received data
        mp_ack(chan2,0);


        /*
        *writeP_chan1P = (int)chan1->write_buf;
        *readP_chan1P  = (int)chan1->read_buf;
        *writeP_chan2P = (int)chan2->write_buf;
        *readP_chan2P  = (int)chan2->read_buf;
        printf("JUST AFTER MASTER RECEIVES @ %d:\n", i);
        printf("chanel 1 write pointer: 0x%x\n", *writeP_chan1P);
        printf("chanel 1 read pointer: 0x%x\n", *readP_chan1P);
        printf("chanel 2 write pointer: 0x%x\n", *writeP_chan2P);
        printf("chanel 2 read pointer: 0x%x\n", *readP_chan2P);
        */
    }

    for(short i=0; i<10; i++) {
        printf("sent data at %d was", i);
        for(int j=0; j<MP_CHAN_SHORTS_AMOUNT; j++) {
            printf(" %d,", send_data[i][j]);
        }
        printf("\nreceived data at %d was", i);
        for(int j=0; j<MP_CHAN_SHORTS_AMOUNT; j++) {
            printf(" %d,", recv_data[i][j]);
        }
        printf("\n");
    }

    //AudioFX struct: contains no effect
    struct AudioFX audio1FX;
    struct AudioFX *audio1FXPnt = &audio1FX;
    int AUDIO_ALLOC_AMOUNT = alloc_dry_vars(audio1FXPnt, 0);

    setup(0); //guitar?

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
        audio_dry(audio1FXPnt);
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
