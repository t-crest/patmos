/*
   Copyright 2016 Technical University of Denmark, DTU Compute. 
   All rights reserved.
   
   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

/*
 * Basic functions to download application via UDP
 * 
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

#include "boot.h"

#ifdef ETHMAC

#include "ethlib/arp.h"
#include "ethlib/udp.h"

unsigned char host_ip[4];

static int available;
static int read_offset;

void ethmac_init(void) {
  eth_mac_initialize();
  arp_table_init();

  my_mac[0] = 0x00;
  my_mac[1] = 0xFF;
  my_mac[2] = 0xEE;
  my_mac[3] = 0xF0;
  my_mac[4] = 0xDA;
  my_mac[5] = 0x42;

  my_ip[0] = 192;
  my_ip[1] = 168;
  my_ip[2] = 24;
  my_ip[3] = 50;

  available = 0;
  read_offset = 0;
}

int ethmac_get_byte(void) {
  while (!available) {
    eth_mac_receive(RX_ADDR, 0);
    unsigned char packet_type = mac_packet_type(RX_ADDR);

    // receive UDP packet
    if (packet_type == 2) {
      // at the correct port
      if (udp_get_source_port(RX_ADDR) == HOST_PORT &&
          udp_get_destination_port(RX_ADDR) == TARGET_PORT) {
        available = udp_get_data_length(RX_ADDR);
        read_offset = 0;
        // remember source
        ipv4_get_source_ip(RX_ADDR, host_ip);
      }
    }
    // respond to ARP requests
    if (packet_type == 3) {
      arp_process_received(RX_ADDR, TX_ADDR);
    }
  }

  // return a byte from a previously received packet
  available--;
  unsigned char c = mem_iord_byte(RX_ADDR + 42 + read_offset++);
  return c;
}

void ethmac_put_byte(unsigned char c) {
  unsigned char b = c;
  udp_send(TX_ADDR, ARP_ADDR, host_ip, TARGET_PORT, HOST_PORT, &b, 1, 10000);
}

#endif /* ETHMAC */
