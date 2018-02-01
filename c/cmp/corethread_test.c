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
#include <string.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include "libcorethread/corethread.h"
#include "libmp/mp.h"

#define MP_CHAN_1_NUM_BUF 2
#define MP_CHAN_1_BUF_SIZE 8

#define MP_CHAN_2_NUM_BUF 2
#define MP_CHAN_2_BUF_SIZE 40

qpd_t chan1;
qpd_t chan2;

communicator_t comm;
coreid_t cores[] = {0,2};

void func_worker_1(void* arg) {
  int worker_1_param = *((int*)arg);
  // For each of the messages that is received
  for (int i = 0; i < 5; ++i) {
      mp_recv(&chan1);
      for(int j = 0; j < chan1.buf_size; j++){
          // Copy the received data to the send buffer
          int to_offset = i * chan1.buf_size + j;
          volatile char _SPM * copy_to;
          volatile char _SPM * copy_from;
          copy_to = (volatile char _SPM *)chan2.write_buf + to_offset;
          
          copy_from = (volatile char _SPM *)chan1.read_buf + j;
          // Like a Cesar code, shifting the ascii alphabet
          *copy_to = (*copy_from)+(char)worker_1_param;
      }
      // Acknowledge the received data.
      mp_ack(&chan1);
  }

  mp_barrier(&comm);

  mp_send(&chan2);
  int ret = 0;
  corethread_exit(&ret);
  return;
}

int main() {

  puts("Master");
  int worker_1 = 2; // For now the core ID
  int worker_1_param = 1;

  char send_data[] = "Hello World!, Sending messages is cool!";
  char recv_data[40];

  // Initialization of message passing buffers
  // mp_chan_init() return false if local and remote
  // addresses are not aligned to words
  if (!mp_chan_init(&chan1,
      get_cpuid(),
      worker_1,
      MP_CHAN_1_BUF_SIZE,
      MP_CHAN_1_NUM_BUF)) {
      abort();
  }
  if (!mp_chan_init(&chan2,
      worker_1,
      get_cpuid(),
      MP_CHAN_2_BUF_SIZE,
      MP_CHAN_2_NUM_BUF)) {
      abort();
  }
  puts("Initialized buffers");
  
  if (!mp_communicator_init(&comm,
      2,
      cores,
      0)) {
    abort();
  }
  puts("Initialized barrier");
  
  
  corethread_create(worker_1,&func_worker_1,(void*)&worker_1_param);

  int i = 0;
  // While there is still data to be sent
  while(i < sizeof(send_data)) {
      int chunk = 0;

      if ( sizeof(send_data)-i >= chan1.buf_size) {
          // If the remaining data is more than the size of the buffer
          chunk = chan1.buf_size;   
      } else {
          // The remaining data all fits in a buffer
          chunk = sizeof(send_data)-i;
      }
      // Copy the chunk of data to the write buffer
      for (int j = 0; j < chunk; ++j) {
          *((volatile char _SPM *)chan1.write_buf+j) = send_data[i+j];
      }
      // Send the chunk of data
      mp_send(&chan1);
      i += chunk;
  }
  puts("Messages sent");

  mp_barrier(&comm);
  puts("Barrier reached");

  mp_recv(&chan2);
  puts("Message recv");
  // Copy the received data to the recv_data array
  for(int i = 0; i < sizeof(recv_data)-1; i++) {
    recv_data[i] = (*((volatile char _SPM *)chan2.read_buf+i)-worker_1_param);
  }
  // Acknowledge the received data
  mp_ack(&chan2);

  recv_data[39] = '\0';
  puts(recv_data);
  int* res;
  corethread_join(worker_1,&res);

  return *res;  
}

