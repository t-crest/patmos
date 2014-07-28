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
*       2 * (buf_size + FLAG_SIZE) + sizeof(recv_count)(Aligned to DW)
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

int main() {
  if (get_cpuid() == 0) {
    puts("Core 0");
    
    mpd_t send_chan;
    mpd_t recv_chan;
    char send_data[] = "Hello World!, Sending messages is cool!";
    char recv_data[40];

    // Initialization of message passing buffers
    mp_send_init(&send_chan,1,NOC_SPM_BASE,NOC_SPM_BASE,8,2);
    mp_recv_init(&recv_chan,1,NOC_SPM_BASE+16,NOC_SPM_BASE+16,40,2);
    // +16 is equal to 64 bytes

    puts("Initialized buffers");
    int i = 0;
    // While there is still data to be sent
    while(i < sizeof(send_data)) {
        int chunk = 0;

        if ( sizeof(send_data)-i >= send_chan.buf_size - FLAG_SIZE) {
            // If the remaining data is more than the size of the buffer
            chunk = send_chan.buf_size - FLAG_SIZE;   
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
    return 0;

  } else if(get_cpuid() == 1) {
    mpd_t send_chan;
    mpd_t recv_chan;
    // Initialize the message passing buffers
    mp_recv_init(&recv_chan,0,NOC_SPM_BASE,NOC_SPM_BASE,8,2);
    mp_send_init(&send_chan,0,NOC_SPM_BASE+16,NOC_SPM_BASE+16,40,2);
    // For each of the messages that is received
    for (int i = 0; i < 5; ++i) {
        mp_recv(&recv_chan);
        for(int j = 0; j < 8; j++){
            // Copy the received data to the send buffer
            *((volatile char _SPM *)send_chan.write_buf + i * 8 + j ) = *((volatile char _SPM *)recv_chan.read_buf + j);
        }
        // Acknowledge the received data.
        mp_ack(&recv_chan);
    }
    
    mp_send(&send_chan);
    return 0;
  } else {
    for (; ; ) { }
    return 0;
  }

  return -1;
}
