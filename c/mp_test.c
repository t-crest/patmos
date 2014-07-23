/*
	Author: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
	Copyright: DTU, BSD License
*/
const int NOC_MASTER = 0;
//#include <stdio.h>
//#include <stdlib.h>
#include <machine/patmos.h>
#include "libnoc/noc.h"
#include "libmp/mp.h"
#include "bootable.h"
#include "patio.h"

int main() {
  if (get_cpuid() == 0) {
    WRITE("Core 0\n",7);
    mp_t send_buf;
    mp_t rcv_buf;
    char send_data[] = "Hell";
    char rcv_data[5];
    mp_send_init(&send_buf,1,NOC_SPM_BASE,NOC_SPM_BASE,8,2);
    mp_rcv_init(&rcv_buf,1,NOC_SPM_BASE+16,NOC_SPM_BASE+16,8,2);
    WRITE("Initialized buffers\n",20);
    for(int i = 0; i < sizeof(send_data); i++) {
      *((volatile char _SPM *)NOC_SPM_BASE+i) = send_data[i];
    }
    mp_send(&send_buf);
    WRITE("Message sent\n",13);
    mp_rcv(&rcv_buf);
    WRITE("Message recv\n",13);
    for(int i = 0; i < sizeof(rcv_data)-1; i++) {
      rcv_data[i] = *((volatile char _SPM *)NOC_SPM_BASE+64+i);
    }
    rcv_data[4] = '\n';
    WRITE(rcv_data,5);
    return 0;

  } else if(get_cpuid() == 1) {
    WRITE("Core 1\n",7);
    mp_t send_buf;
    mp_t rcv_buf;
    mp_rcv_init(&rcv_buf,0,NOC_SPM_BASE,NOC_SPM_BASE,8,2);
    mp_send_init(&send_buf,0,NOC_SPM_BASE+16,NOC_SPM_BASE+16,8,2);
    WRITE("Initialized buffers\n",20);
    mp_rcv(&rcv_buf);
    WRITE("Message recv\n",13);
    for(int i = 0; i < 8; i++){
      *((volatile char _SPM *)NOC_SPM_BASE+64+i) = *((volatile char _SPM *)NOC_SPM_BASE+i);
    }
    mp_send(&send_buf);
    WRITE("Message sent\n",13);
    return 0;
  }
  return -1;
}
