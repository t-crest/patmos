/*
    A small program that tests the NoC and can be compiled into the
    boot ROM.

    Authors: Evangelia Kasapaki, Wolfgang Puffitsch
    Copyright: DTU, BSD License

*/

const int NOC_MASTER = 0;
#include <machine/spm.h>
#include "libnoc/noc.h"
#include "patio.h"

#include "bootable.h"

static void master(void);

static void slave(void);

struct msg_t {
  int sum;
  int ready; // flag at end so it is transmitted last
};

volatile _SPM struct msg_t *spm_in  = (volatile _SPM struct msg_t *)NOC_SPM_BASE;
volatile _SPM struct msg_t *spm_out = (volatile _SPM struct msg_t *)NOC_SPM_BASE+1;

int main() {

  // clear communication areas
  // cannot use memset() for _SPM pointers!
  for(int i = 0; i < sizeof(struct msg_t); i++) {
    ((volatile _SPM char *)spm_in)[i] = 0;
    ((volatile _SPM char *)spm_out)[i] = 0;
  }

  // dispatch on core ID
  if(get_cpuid() == 0) {
    master();
  } else {
    slave();
  }

  return 0;
}

static void master(void) {

    // reset sum
    spm_out->sum = 0;
    // the message is ready
    spm_out->ready = 1;

	// send message to core 1
	noc_send(get_cpuid()+1, spm_in, spm_out, sizeof(struct msg_t));

    WRITE("SENT\n", 5);

	// wait and poll
	while(!spm_in->ready) {
      /* spin */
    }

    WRITE("RCVD ", 5);

    static char msg[10];
    msg[0] = XDIGIT((spm_in->sum >> 28) & 0xf);
    msg[1] = XDIGIT((spm_in->sum >> 24) & 0xf);
    msg[2] = XDIGIT((spm_in->sum >> 20) & 0xf);
    msg[3] = XDIGIT((spm_in->sum >> 16) & 0xf);
    msg[4] = XDIGIT((spm_in->sum >> 12) & 0xf);
    msg[5] = XDIGIT((spm_in->sum >>  8) & 0xf);
    msg[6] = XDIGIT((spm_in->sum >>  4) & 0xf);
    msg[7] = XDIGIT((spm_in->sum >>  0) & 0xf);
    msg[8] = '\n';
    WRITE(msg, 9);

	return;
}

static void slave(void) {

	// wait and poll until message arrives
	while(!spm_in->ready) {
      /* spin */
    }

	// PROCESS: add ID to sum_id
	spm_out->sum = spm_in->sum + get_cpuid();
    spm_out->ready = 1;

	// send to next slave
	int rcv_id = (get_cpuid()==(NOC_CORES-1)) ? 0 : get_cpuid()+1;
	noc_send(rcv_id, spm_in, spm_out, sizeof(struct msg_t));

	return;
}


