/*
    This is minimal demonstration how to use the Argo NoC
    with one message passing channel from core 0 to core 1.
    The value is returned via a shared variable in main memory.

    Author: Martin Schoeberl
    Copyright: DTU, BSD License
*/

#include <stdio.h>
#include <machine/patmos.h>

#include "libcorethread/corethread.h"
#include "libmp/mp.h"

#define NUM_BUF 2
#define BUF_SIZE 100

// Whatever this contant means, it is needed
const int NOC_MASTER = 0;
// Shared data in main memory for the return value
volatile _UNCACHED static int field;

// The main function for the other thread on core 1
void work(void* arg) {

  // create a channel
  qpd_t *channel = mp_create_qport(1, SINK, BUF_SIZE, NUM_BUF);
  // init
  mp_init_ports();
  // receive
  mp_recv(channel, 0);
  int data = *(volatile int _SPM *) channel->read_buf;
  mp_ack(channel, 0);

  // Return a change value in the shared variable
  field = data + 1;
}

int main() {

  printf("Hello Argo NoC\n");
  int core_id = 1; // The core number
  corethread_create(core_id, &work, NULL);

  int data = 42;
  // create a channel
  qpd_t *channel = mp_create_qport(1, SOURCE, BUF_SIZE, NUM_BUF);
  // init
  mp_init_ports();
  // write data into the send buffer
  *(volatile int _SPM *) channel->write_buf = data;
  // send the buffer
  mp_send(channel, 0);
  printf("Data sent\n");
  printf("Returned data is: %d\n", field);
  int* res;
  corethread_join(core_id, (void *) &res );
  return 0;
}
