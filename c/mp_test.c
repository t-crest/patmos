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
    //WRITE("Core 0\n",7);
    
    mpd_t send_chan;
    mpd_t recv_chan;
    char send_data[] = "Hello World!, Sending messages is cool!";
    char recv_data[40];

    mp_send_init(&send_chan,1,NOC_SPM_BASE,NOC_SPM_BASE,8,2);

    mp_recv_init(&recv_chan,1,NOC_SPM_BASE+16,NOC_SPM_BASE+16,40,2);

    //WRITE("Initialized buffers\n",20);
    puts("Initialized buffers");
    int i = 0;
    while(i < sizeof(send_data)) {
        int chunk = 0;
        if ( sizeof(send_data)-i >= send_chan.buf_size-8)
        {
            chunk = send_chan.buf_size-8;   
        } else {
            chunk = sizeof(send_data)-i;
        }
        // Copy data to the write buffer
        for (int j = 0; j < chunk; ++j)
        {
            *((volatile char _SPM *)send_chan.write_buf+j) = send_data[i+j];
        }
        mp_send(&send_chan);
        i += chunk;
        //WRITE("Message sent\n",13);
        puts("Message sent");
    }

    mp_recv(&recv_chan);
    //WRITE("Message recv\n",13);
    puts("Message recv");
    for(int i = 0; i < sizeof(recv_data)-1; i++) {
      recv_data[i] = *((volatile char _SPM *)recv_chan.read_buf+i);
    }
    //recv_data[39] = '\n';
    //WRITE(recv_data,40);
    recv_data[39] = '\0';
    puts(recv_data);
    return 0;

  } else if(get_cpuid() == 1) {
    //WRITE("Core 1\n",7);
    mpd_t send_chan;
    mpd_t recv_chan;
    mp_recv_init(&recv_chan,0,NOC_SPM_BASE,NOC_SPM_BASE,8,2);
    mp_send_init(&send_chan,0,NOC_SPM_BASE+16,NOC_SPM_BASE+16,40,2);
    //WRITE("Initialized buffers\n",20);
    for (int i = 0; i < 5; ++i) {
        mp_recv(&recv_chan);
        //WRITE("Message recv\n",13);
        for(int j = 0; j < 8; j++){
          *((volatile char _SPM *)send_chan.write_buf + i * 8 + j ) = *((volatile char _SPM *)recv_chan.read_buf + j);
        }
        mp_ack(&recv_chan);
    }
    
    mp_send(&send_chan);
    //WRITE("Message sent\n",13);
    return 0;
  } else {
    for (; ; ) { }
    return 0;
  }

  return -1;
}

/*else if(get_cpuid() == 1) {
    //WRITE("Core 1\n",7);
    mpd_t send_chan;
    mpd_t recv_chan;
    mp_recv_init(&recv_chan,0,NOC_SPM_BASE,NOC_SPM_BASE,8,2);
    mp_send_init(&send_chan,0,NOC_SPM_BASE+16,NOC_SPM_BASE+16,40,2);
    //WRITE("Initialized buffers\n",20);
    for (int i = 0; i < 5; ++i) {
        mp_recv(&recv_chan);
        //WRITE("Message recv\n",13);
        for(int j = 0; j < 8; j++){
          *((volatile char _SPM *)send_chan.write_buf + i * 8 + j ) = *((volatile char _SPM *)recv_chan.read_buf + j);
        }
        mp_ack(&recv_chan);
    }
    
    mp_send(&send_chan);
    //WRITE("Message sent\n",13);
    return 0;
  } */
