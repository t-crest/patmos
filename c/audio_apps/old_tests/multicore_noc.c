#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libaudio/audio.h"
#include "libaudio/audio.c"

#define MP_CHAN_SHORTS_AMOUNT 2

#define MP_CHAN_1_ID 1
#define MP_CHAN_1_NUM_BUF 1
#define MP_CHAN_1_MSG_SIZE MP_CHAN_SHORTS_AMOUNT * 2 // 2 shorts = 4 bytes (actually always multiples of 4)

#define MP_CHAN_2_ID 2
#define MP_CHAN_2_NUM_BUF 1
#define MP_CHAN_2_MSG_SIZE MP_CHAN_SHORTS_AMOUNT * 2 // 2 shorts = 4 bytes


void thread1_delay(void* args) {
    volatile _UNCACHED int **inArgs = (volatile _UNCACHED int **) args;
    volatile _UNCACHED int *exitP      = inArgs[0];
    volatile _UNCACHED int *stateVar1P = inArgs[1];

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
            received[j] = received[j] + 2;
        }
        //acknowledge
        mp_ack(chan1,0);

        //set respond data
        for(int j=0; j<MP_CHAN_SHORTS_AMOUNT; j++) {
            *((volatile _SPM short *)chan2->write_buf+j) = received[j];
        }
        //before responding: set stateVar1
        *stateVar1P = i;
        //respond
        mp_send(chan2,0);

    }

    // exit with return value
    int ret = 0;
    corethread_exit(&ret);
    return;
}





int main() {

    printf("Core amount is %d\n", get_cpucnt());

    printf("BEGINNING:\n");
    printf("spm_alloc_array[0]=0x%x\n", (unsigned int)mp_alloc(0));

    printf("BEGINNING 2:\n");
    printf("spm_alloc_array[0]=0x%x\n", (unsigned int)mp_alloc(0));

    //create corethread type var
    int threadOne = 1;
    //arguments to thread 1 function
    int exit = 0;
    int stateVar1 = -1;
    volatile _UNCACHED int *exitP      = (volatile _UNCACHED int *) &exit;
    volatile _UNCACHED int *stateVar1P = (volatile _UNCACHED int *) &stateVar1;
    volatile _UNCACHED int (*thread1_args[2]);
    thread1_args[0] = exitP;
    thread1_args[1] = stateVar1P;

    printf("starting thread...\n");
    //set thread function and start thread
    corethread_create(threadOne, &thread1_delay, (void*) thread1_args);

    // MP: create message passing ports
    qpd_t * chan1 = mp_create_qport(MP_CHAN_1_ID, SOURCE,
        MP_CHAN_1_MSG_SIZE, MP_CHAN_1_NUM_BUF);

    printf("AFTER CREATE 1:\n");
    printf("spm_alloc_array[0]=0x%x\n", (unsigned int)mp_alloc(0));

    qpd_t * chan2 = mp_create_qport(MP_CHAN_2_ID, SINK,
        MP_CHAN_2_MSG_SIZE, MP_CHAN_2_NUM_BUF);

    printf("created MP channels\n");

    printf("AFTER CREATE 2:\n");
    printf("spm_alloc_array[0]=0x%x\n", (unsigned int)mp_alloc(0));

    // Initialize the communication channels
    int nocret = mp_init_ports();
    // TODO: check on retval

    printf("AFTER INIT:\n");
    printf("spm_alloc_array[0]=0x%x\n", (unsigned int)mp_alloc(0));

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
            *((volatile _SPM short *)chan1->write_buf+j) = send_data[i][j];
        }

        printf("just before sending, stateVar1 should be %d, and is %d\n", (i-1), stateVar1);


        printf("SEND_RECV_COUNT BEFORE SEND: 0x%x\n", *chan1->send_recv_count);

        mp_send(chan1,0);

        //receive message from slave
        printf("gonna receive\n");

        printf("SEND_RECV_COUNT BEFORE RECEIVE: 0x%x\n", *chan1->send_recv_count);
        mp_recv(chan2,0);
        printf("just after receiving, stateVar1 should be %d, and is %d\n", i, stateVar1);

        for(int j=0; j<MP_CHAN_SHORTS_AMOUNT; j++) {
            recv_data[i][j] = *((volatile _SPM short *)chan2->read_buf+j);
        }

        // Acknowledge the received data
        mp_ack(chan2,0);

        printf("@ END OF LOOP %d: \n", i);

        printf("chan1->direction_type: %d\n", chan1->direction_type);
        printf("chan1->remote: %hhd\n", chan1->remote);
        printf("chan1->recv_addr: 0x%x\n", (int)chan1->recv_addr);
        printf("*chan1->recv_addr: 0x%x\n", *((int *)chan1->recv_addr));
        printf("chan1->send_recv_count: 0x%x\n", (int)chan1->send_recv_count);
        printf("*chan1->send_recv_count: 0x%x\n", *chan1->send_recv_count);
        printf("chan1->buf_size: %d\n", chan1->buf_size);
        printf("chan1->num_buf: %d\n", chan1->num_buf);
        printf("THREAD 0 AS SENDER, CHAN1:\n");
        printf("chan1->write_buf: 0x%x\n", (int)chan1->write_buf);
        printf("*chan1->write_buf+0: %d\n", *((volatile _SPM short *)chan1->write_buf+0));
        printf("*chan1->write_buf+1: %d\n", *((volatile _SPM short *)chan1->write_buf+1));
        printf("chan1->shadow_write_buf: 0x%x\n", (int)chan1->shadow_write_buf);
        printf("*chan1->shadow_write_buf+0: %d\n", *((volatile _SPM short *)chan1->shadow_write_buf+0));
        printf("*chan1->shadow_write_buf+1: %d\n", *((volatile _SPM short *)chan1->shadow_write_buf+1));
        printf("chan1->send_count: %d\n", chan1->send_count);
        printf("chan1->send_ptr: %d\n", chan1->send_ptr);
        printf("THREAD 0 AS RECEIVER, CHAN2:\n");
        printf("chan2->read_buf: 0x%x\n", (int)chan2->read_buf);
        printf("*chan2->read_buf+0: %d\n", *((volatile _SPM short *)chan2->read_buf+0));
        printf("*chan2->read_buf+1: %d\n", *((volatile _SPM short *)chan2->read_buf+1));
        printf("chan2->recv_count: 0x%x\n", (int)chan2->recv_count);
        printf("*chan2->recv_count: %d\n", *chan2->recv_count);
        printf("chan2->recv_ptr: %d\n", chan2->recv_ptr);
    }

    //printing sent and received values:
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


    //exit stuff
    printf("exit here!\n");
    exit = 1;
    printf("waiting for thread 1 to finish...\n");
    //join with thread 1
    int *retval;
    corethread_join(threadOne, (void **)&retval);
    printf("thread 1 finished!\n");

    return 0;
}
