/*
    A small demo program demostrating the use of argo noc for communication between patmos processors
    Master core initiates a message that is sent from one core to the next (based on CORE_ID).
    Each core adds its ID to the sum of ids and forwards the message to the next core,
    until it reaches the master again.

    Reduced version that can be compiled into boot ROM.

    Author: Evangelia Kasapaki, Ioannis Kotleas, Wolfgang Puffitsch
    Copyright: DTU, BSD License
*/

#include <machine/spm.h>

const int NOC_MASTER = 0;
#include "libnoc/noc.h"
#include "patio.h"

#include "bootable.h"

static void master(void);
static void slave(void);

/*/////////////////////////////////////////////////////////////////////////
// Main application
/////////////////////////////////////////////////////////////////////////*/

int main() {

  // Clear scratch pad in all cores
  // 16+16 integers
  int i;
  for(i = 0; i < NOC_CORES*4; i++) {
    *(NOC_SPM_BASE+i) = 0;
    *(NOC_SPM_BASE+NOC_CORES*4+i) = 0;
  }

  if(get_cpuid() == 0) {
    master();
  } else {
    slave();
  }

  return 0;
}

static void master(void) {

  volatile _SPM char *spm_base = (volatile _SPM char *) NOC_SPM_BASE; // 0
  volatile _SPM char *spm_slave = spm_base+NOC_CORES*16;              // 64

  // message to be sent
  const char *msg_snd = "Hello slaves sum_id:0";
  char msg_rcv[22];

  // put message to spm
  int i;
  for (i = 0; i < 21; i++) {
    *(spm_base+i) = *(msg_snd+i);
  }

  // send message
  noc_send(1, spm_slave, spm_base, 21); //21 bytes

  WRITE("MASTER: message sent: ", 22);
  WRITE(msg_snd, 21);
  WRITE("\n", 1);

  // wait and poll
  while(*(spm_slave+20) == 0) {;}

  // received message
  WRITE("MASTER: finished polling\n", 25);
  WRITE("MASTER: message received: ", 26);
  // copy message to static location and print
  for (i = 0; i < 21; i++) {
    *(msg_rcv+i) = *(spm_slave+i);
  }
  WRITE(msg_rcv,21);
  WRITE("\n", 1);

  return;
}

static void slave(void) {

  volatile _SPM char *spm_base = (volatile _SPM char *) NOC_SPM_BASE; // 0
  volatile _SPM char *spm_slave = spm_base+NOC_CORES*16;              // 64

  // wait and poll until message arrives
  while(*(spm_slave+20) == 0) {;}

  // put message for master to spm
  const char *msg = "Hello master ";
  int i;
  for (i = 6; i < 12; i++) {
    *(spm_slave+i) = *(msg+i);
  }

  // PROCESS : add ID to sum_id
  *(spm_slave+20) = *(spm_slave+20) + get_cpuid();

  // send to next slave
  int rcv_id = (get_cpuid()==(NOC_CORES-1))? 0 : get_cpuid()+1;
  noc_send(rcv_id, spm_slave, spm_slave, 21);

  return;
}
