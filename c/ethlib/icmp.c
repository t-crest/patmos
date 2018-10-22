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
 * ICMP section of ethlib (ethernet library)
 * 
 * Authors: Luca Pezzarossa (lpez@dtu.dk)
 *          Jakob Kenn Toft
 *          Jesper Lønbæk
 *          Russell Barnes
 */

#include "icmp.h"

///////////////////////////////////////////////////////////////
//Functions to get the ICMP protocol field
///////////////////////////////////////////////////////////////

//This function return the ICMP type of a ICMP packet starting in pkt_addr.
unsigned char icmp_get_type(unsigned int pkt_addr){
	return mem_iord_byte(pkt_addr + 34);
}

//This function return the ICMP code of a ICMP packet starting in pkt_addr.
unsigned char icmp_get_code(unsigned int pkt_addr){
	return mem_iord_byte(pkt_addr + 35);
}

//This function return the ICMP Quench of a ICMP packet starting in pkt_addr.
unsigned int icmp_get_quench(unsigned int pkt_addr){
	return mem_iord(pkt_addr + 38);
}

///////////////////////////////////////////////////////////////
//Support functions related to the ICMP protocol
///////////////////////////////////////////////////////////////

//This function takes the received ping request icmp paket starting in rx_addr and builds a reply packet starting in tx_addr. rx_addr and tx_addr can be the same.
unsigned int icmp_build_ping_reply(unsigned int rx_addr, unsigned int tx_addr){

	unsigned int frame_length = 14 + ((mem_iord_byte(rx_addr+16) << 8) | mem_iord_byte(rx_addr+17));
	
	//Copy the entire frame	
	if (rx_addr != tx_addr ){ 
		for(int i=0; i<frame_length; i++){
			mem_iowr_byte(tx_addr + i, mem_iord_byte(rx_addr + i));
		}
	} 
	//Swap MAC addrs
	for (int i=0; i<6; i++){
		mem_iowr_byte(tx_addr + i, mem_iord_byte(tx_addr+6+i));
		mem_iowr_byte(tx_addr + 6 + i, my_mac[i]);
	}
	//Swap IP addr
	for (int i=0; i<4; i++){
		mem_iowr_byte(tx_addr + 30 + i, mem_iord_byte(tx_addr+26+i));
		mem_iowr_byte(tx_addr + 26 + i, my_ip[i]);
	}
	//Change ICMP type from request to reply
	mem_iowr_byte(tx_addr + 34, 0x00);

	//Change ICMP checksum
	unsigned short int checksum;
	checksum = (mem_iord_byte(tx_addr+36)<<8) | mem_iord_byte(tx_addr+37);
	checksum = ~((~checksum) - 0x0800);
	mem_iowr_byte(tx_addr+36, checksum>>8 );//hi byte
	mem_iowr_byte(tx_addr+37, checksum & 0xff );//lo byte

	return frame_length;
}

//This function process a received ICMP package. If it is a ping request and we are the destination (IP) it reply the ping and returns 1. Otherwise it returns 0.
int icmp_process_received(unsigned int rx_addr, unsigned int tx_addr){
	//Check if it is a ping request
	if (icmp_get_type(rx_addr) == 0x08){
		//Check if we are the destnation (IP)
		unsigned char destination_ip[4];
		ipv4_get_destination_ip(rx_addr, destination_ip);
		if (ipv4_compare_ip(destination_ip, my_ip) == ICMP){
			unsigned int frame_length = icmp_build_ping_reply(rx_addr, tx_addr);
			eth_mac_send(tx_addr, frame_length);
			return 1;
		}	
	}
	return 0;
}

