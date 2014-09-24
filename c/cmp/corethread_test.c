/**
* PROGRAM DESCRIPTION:
*
* This is an example program for using the libcorethread (corethread library).
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
#include "libcorethread/corethread.h"
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


void func_worker_1(void* arg) {
  int worker_1_param = *((int*)arg);

  mpd_t send_chan;
  mpd_t recv_chan;
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
          // Like a Cesar code, shifting the asci alphabet
          *copy_to = (*copy_from)+(char)worker_1_param;
      }
      // Acknowledge the received data.
      mp_ack(&recv_chan);
  }
  
  mp_send(&send_chan);
  int ret = 0;
  corethread_exit(&ret);
  return;
}

int main() {
  //corethread_worker(); // TODO: This should be called by the constructor

  puts("Master");
  corethread_t worker_1 = 2; // For now the core ID
  corethread_attr_t worker_1_attr = joinable; // For now this does nothing
  int worker_1_param = 1;

  corethread_create(&worker_1,&worker_1_attr,&func_worker_1,(void*)&worker_1_param);

  mpd_t send_chan;
  mpd_t recv_chan;
  char send_data[] = "Hello World!, Sending messages is cool!";
  char recv_data[40];

  // Initialization of message passing buffers
  // mp_send_init() and mp_recv_init() return false if local and remote
  // addresses are not aligned to double words
  if (!mp_send_init(&send_chan,
      worker_1,
      MP_CHAN_1,
      MP_CHAN_1,
      MP_CHAN_1_BUF_SIZE,
      MP_CHAN_1_NUM_BUF)) {
      abort();
  }
  if (!mp_recv_init(&recv_chan,
      worker_1,
      MP_CHAN_2,
      MP_CHAN_2,
      MP_CHAN_2_BUF_SIZE,
      MP_CHAN_2_NUM_BUF)) {
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

  int* res;
  corethread_join(worker_1,&res);

  return *res;  
}

