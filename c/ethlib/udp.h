/*
   Copyright 2014 Technical University of Denmark, DTU Compute. 
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
 * UDP section of ethlib (ethernet library)
 * 
 * Authors: Luca Pezzarossa (lpez@dtu.dk)
 *          Jakob Kenn Toft
 *          Jesper Lønbæk
 *          Russell Barnes
 */

#ifndef _UDP_H_
#define _UDP_H_

#include <stdio.h>
#include "eth_patmos_io.h"
#include "arp.h"
#include "ipv4.h"

///////////////////////////////////////////////////////////////
//Functions to get the UDP protocol fields
///////////////////////////////////////////////////////////////

//This function gets the source port of an UDP packet.
unsigned short int udp_get_source_port(unsigned int pkt_addr);

//This function gets the destination port of an UDP packet.
unsigned short int udp_get_destination_port(unsigned int pkt_addr);

//This function gets the packet length of an UDP packet.
unsigned short int udp_get_packet_length(unsigned int pkt_addr);

//This function gets the checksum field of an UDP packet.
unsigned short int udp_get_checksum(unsigned int pkt_addr);

//This function gets the data length field of an UDP packet.
unsigned short int udp_get_data_length(unsigned int pkt_addr);

//This function gets the data field of an UDP packet.
unsigned char udp_get_data(unsigned int pkt_addr, unsigned char data[], unsigned int data_length);

///////////////////////////////////////////////////////////////
//Support functions related to the UDP protocol
///////////////////////////////////////////////////////////////

//This function comute and returns the UDP checksum. The function ignore the the field checksum.
unsigned short int udp_compute_checksum(unsigned int pkt_addr);

//This function compute and returns the UDP checksum. The function ignore the the field checksum.
int udp_verify_checksum(unsigned int pkt_addr);

//This function sends an UDP packet to the dstination IP.
int udp_send(unsigned int tx_addr, unsigned int rx_addr, unsigned char destination_ip[], unsigned short int source_port, unsigned short int destination_port, unsigned char data[], unsigned short int data_length, long long unsigned int timeout);

#endif
