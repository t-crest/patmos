/**
* PROGRAM DESCRIPTION:
*
* This is an test case for the queuing message passing in libmp.
*
* The test sends characters from core 0 to a slave core and then
* back to core 0. The characters are send back and forth in
* differently size buffers.
*
*
*        __________ (chan 1)  _________
*        |        | --------> |       |
*        | Core 0 |           | Slave |
*        |________| <-------- |_______|
*                   (chan 2)
*
*/

/*
	Author: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
	Copyright: DTU, BSD License
*/
const int NOC_MASTER = 0;
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include "libcorethread/corethread.h"
#include "libmp/mp.h"
#include "include/debug.h"

#define MP_CHAN_1_ID 1
#define MP_CHAN_1_NUM_BUF 2
#define MP_CHAN_1_MSG_SIZE 8

#define MP_CHAN_2_ID 2
#define MP_CHAN_2_NUM_BUF 2
#define MP_CHAN_2_MSG_SIZE 40


void func_worker_1(void* arg) {
  // Cast and load the parameter
  int worker_1_param = *((int*)arg);
  // Create the queuing ports
  qpd_t * chan1 = mp_create_qport(MP_CHAN_1_ID, SINK,
              NOC_MASTER, MP_CHAN_1_MSG_SIZE, MP_CHAN_1_NUM_BUF);
  qpd_t * chan2 = mp_create_qport(MP_CHAN_2_ID, SOURCE,
              NOC_MASTER, MP_CHAN_2_MSG_SIZE, MP_CHAN_2_NUM_BUF);
  // TODO: check that the returned pointers are not NULL
    if (chan1 == NULL || chan2 == NULL) {
    DEBUGF(chan1);
    DEBUGF(chan2);
    abort();
  }
  // Initialize the communication channels
  int retval = mp_init_chans();
  // TODO: check on retval

  // For each of the messages that is received
  for (int i = 0; i < 5; ++i) {
      mp_recv(chan1,0);
      for(int j = 0; j < chan1->buf_size; j++){
          // Copy the received data to the send buffer
          int to_offset = i * chan1->buf_size + j;
          volatile char _SPM * copy_to;
          volatile char _SPM * copy_from;
          copy_to = (volatile char _SPM *)chan2->write_buf + to_offset;
          
          copy_from = (volatile char _SPM *)chan1->read_buf + j;
          // Like a Cesar code, shifting the ascii alphabet
          *copy_to = (*copy_from)+(char)worker_1_param;
      }
      // Acknowledge the received data.
      mp_ack(chan1,0);
  }

  mp_send(chan2,0);
  int ret = 0;
  corethread_exit(&ret);
  return;
}

int main() {

  puts("Master");
  corethread_t worker_1 = 2; // For now the core ID
  int worker_1_param = 1;

  corethread_create(&worker_1,&func_worker_1,(void*)&worker_1_param);

  char send_data[] = "Hello World!, Sending messages is cool!";
  char recv_data[40];

  // Create the queuing ports
  qpd_t * chan1 = mp_create_qport(MP_CHAN_1_ID, SOURCE,
              worker_1, MP_CHAN_1_MSG_SIZE, MP_CHAN_1_NUM_BUF);
  qpd_t * chan2 = mp_create_qport(MP_CHAN_2_ID, SINK,
              worker_1, MP_CHAN_2_MSG_SIZE, MP_CHAN_2_NUM_BUF);
  // TODO: check that the returned pointers are not NULL
  if (chan1 == NULL || chan2 == NULL) {
    DEBUGF(chan1);
    DEBUGF(chan2);
    abort();
  }

  // Initialize the communication channels
  int retval = mp_init_chans();
  // TODO: check on retval

  puts("Initialized buffers");

  int i = 0;
  // While there is still data to be sent
  while(i < sizeof(send_data)) {
      int chunk = 0;

      if ( sizeof(send_data)-i >= chan1->buf_size) {
          // If the remaining data is more than the size of the buffer
          chunk = chan1->buf_size;   
      } else {
          // The remaining data all fits in a buffer
          chunk = sizeof(send_data)-i;
      }
      // Copy the chunk of data to the write buffer
      for (int j = 0; j < chunk; ++j) {
          *((volatile char _SPM *)chan1->write_buf+j) = send_data[i+j];
      }
      // Send the chunk of data
      mp_send(chan1,0);
      i += chunk;
  }
  puts("Messages sent");

  mp_recv(chan2,0);
  puts("Message recv");
  // Copy the received data to the recv_data array
  for(int i = 0; i < sizeof(recv_data)-1; i++) {
    recv_data[i] = (*((volatile char _SPM *)chan2->read_buf+i)-worker_1_param);
  }
  // Acknowledge the received data
  mp_ack(chan2,0);

  recv_data[39] = '\0';
  puts(recv_data);
  int* res;
  corethread_join(worker_1,&res);

  return *res;  
}

