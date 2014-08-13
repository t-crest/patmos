/**
* PROGRAM DESCRIPTION:
*
* This is an example program for using the libmp (message passing library).
*
* The program will send the string "Hello World!, Sending messages is cool!"
* from core 0 to core 1 and then back to core 0 and print it to the console.
*
* The string will be sent from core 0 in messages of 8 bytes at the time.
* Core 1 will receive the messages, acknowledge them and send them back in one
* message.
*
* It is up to the programmer to allocate buffering space in the communication
* scratch pads. The allocation is specified in the mp_send_init() and
* mp_recv_init() functions.
*
* The size of the message passing buffer structure in the commuincation
* scratch pads are:
*
* Sender side:
*       NUM_WRITE_BUF * (buf_size + FLAG_SIZE) + sizeof(recv_count)
*                                                       (Aligned to DW)
*
* Receiver side:
*       num_buf * (buf_size + FLAG_SIZE) + sizeof(remote_recv_count)
*                                                       (Aligned to DW)
*
* The local and remote addresses set in mp_send_init() and mp_recv_init()
* have to be choosen such that they do not overlap.
*
*/

/*
	Author: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
	Copyright: DTU, BSD License
*/
const int NOC_MASTER = 0;
#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include "libmp/mp.h"
//#include "bootable.h"
//#include "patio.h"

// The starting address of the message passing structure for channel 1
#define MP_CHAN_1 NOC_SPM_BASE
#define MP_CHAN_1_NUM_BUF 2
#define MP_CHAN_1_BUF_SIZE 8
// The size of the message passing structure for channel 1
#define MP_CHAN_1_SIZE (MP_CHAN_1_NUM_BUF * (MP_CHAN_1_BUF_SIZE \
                                         + FLAG_SIZE) + DWALIGN(sizeof(size_t)))

// The starting address of the message passing structure for channel 2
// This starting address is offset by the size of message pasing structure 1
#define MP_CHAN_2 NOC_SPM_BASE + MP_CHAN_1_SIZE
#define MP_CHAN_2_NUM_BUF 2
#define MP_CHAN_2_BUF_SIZE 40
// The size of the message passing structure for channel 1
#define MP_CHAN_2_SIZE (MP_CHAN_2_NUM_BUF * (MP_CHAN_2_BUF_SIZE \
                                         + FLAG_SIZE) + DWALIGN(sizeof(size_t)))


// The starting address of the barrier
#define MP_BARRIER_ADDR NOC_SPM_BASE + MP_CHAN_1_SIZE + MP_CHAN_2_SIZE
#define MP_BARRIER_SIZE NOC_CORES * DWALIGN(sizeof(int))
#define MP_ALLCORES

static const unsigned cpuids[] = {0,1,2,3};

int main() {
  if (get_cpuid() == 0) {
    puts("Core 0");
    
    mpd_t send_chan;
    mpd_t recv_chan;
    char send_data[] = "Hello World!, Sending messages is cool!";
    char recv_data[40];

    communicator_t barrier;

    // Initialization of message passing buffers
    // mp_send_init() and mp_recv_init() return false if local and remote
    // addresses are not aligned to double words
    if (!mp_send_init(&send_chan,
        1,
        MP_CHAN_1,
        MP_CHAN_1,
        MP_CHAN_1_BUF_SIZE,
        MP_CHAN_1_NUM_BUF)) {
        abort();
    }
    if (!mp_recv_init(&recv_chan,
        1,
        MP_CHAN_2,
        MP_CHAN_2,
        MP_CHAN_2_BUF_SIZE,
        MP_CHAN_2_NUM_BUF)) {
        abort();
    }
    if (!mp_barrier_init(&barrier,NOC_CORES,cpuids,MP_BARRIER_ADDR)) {
        abort();
    }

    puts("Initialized buffers");
    int i = 0;
    // While there is still data to be sent
    while(i < sizeof(send_data)) {
        int chunk = 0;

        if ( sizeof(send_data)-i >= send_chan.buf_size) {
            // If the remaining data is more than the size of the buffer
            chunk = send_chan.buf_size;   
        } else {
            // The remaining data all fits in a buffer
            chunk = sizeof(send_data)-i;
        }
        // Copy the chunk of data to the write buffer
        for (int j = 0; j < chunk; ++j) {
            *((volatile char _SPM *)send_chan.write_buf+j) = send_data[i+j];
        }
        // Send the chunk of data
        mp_send(&send_chan);
        i += chunk;
        puts("Message sent");
    }

    mp_recv(&recv_chan);
    puts("Message recv");
    // Copy the received data to the recv_data array
    for(int i = 0; i < sizeof(recv_data)-1; i++) {
      recv_data[i] = *((volatile char _SPM *)recv_chan.read_buf+i);
    }
    // Acknowledge the received data
    mp_ack(&recv_chan);

    recv_data[39] = '\0';
    puts(recv_data);
    mp_barrier_debug(&barrier);
    puts("Barrier reached");
    return 0;

  } else if(get_cpuid() == 1) {
    mpd_t send_chan;
    mpd_t recv_chan;
    communicator_t barrier;
    // Initialize the message passing buffers
    mp_recv_init(&recv_chan,
        0,
        MP_CHAN_1,
        MP_CHAN_1,
        MP_CHAN_1_BUF_SIZE,
        MP_CHAN_1_NUM_BUF);
    mp_send_init(&send_chan,
        0,
        MP_CHAN_2,
        MP_CHAN_2,
        MP_CHAN_2_BUF_SIZE,
        MP_CHAN_2_NUM_BUF);
    if (!mp_barrier_init(&barrier,NOC_CORES,cpuids,MP_BARRIER_ADDR)) {
        abort();
    }
    // For each of the messages that is received
    for (int i = 0; i < 5; ++i) {
        mp_recv(&recv_chan);
        for(int j = 0; j < recv_chan.buf_size; j++){
            // Copy the received data to the send buffer
            int to_offset = i * recv_chan.buf_size + j;
            volatile char _SPM * copy_to;
            volatile char _SPM * copy_from;
            copy_to = (volatile char _SPM *)send_chan.write_buf + to_offset;
            
            copy_from = (volatile char _SPM *)recv_chan.read_buf + j;
            *copy_to = *copy_from;
        }
        // Acknowledge the received data.
        mp_ack(&recv_chan);
    }
    
    mp_send(&send_chan);
    mp_barrier(&barrier);
    return 0;
  } else {
    //for (; ; ) { }
    communicator_t barrier;
    if (!mp_barrier_init(&barrier,NOC_CORES,cpuids,MP_BARRIER_ADDR)) {
        abort();
    }
    mp_barrier(&barrier);
    return 0;
  }

  return -1;
}
