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
 * MAC section of ethlib (ethernet library)
 * 
 * Authors: Luca Pezzarossa (lpez@dtu.dk)
 *          Jakob Kenn Toft
 *          Jesper Lønbæk
 *          Russell Barnes
 */

#include "mac.h"

unsigned char my_mac[6] = {0x00, 0x80, 0x6e, 0xF0, 0xDA, 0x42};

enum eth_protocol mac_packet_type(unsigned int addr){
	unsigned char b = mem_iord_byte(addr + 13);
	if (mem_iord_byte(addr + 12) == 0x08){
		if(b == 0x00){//IP
			b = mem_iord_byte(addr + 23);
			if (b == 0x01){
				return ICMP;//ICMP
			}else if(b == 0x11){
				return UDP;//UDP
			}else if(b == 0x06){
				return TCP;//TCP
			}else{
				return UNSUPPORTED;
			}
		}else if (b == 0x06){
			return ARP;//ARP
		}else{
			return UNSUPPORTED;
		}
	} else if(mem_iord_byte(addr + 12) == 0x88){
		if (b == 0xc) {
			return LLDP;
		}
		else if(b == 0xf7) {
			return PTP;
		}	
	} else if(mem_iord_byte(addr + 12) == 0x89){
		if(b == 0x1d){
			return TTE_PCF;
		}
	}
	return UNSUPPORTED;
}

//This function retrieves the mac of the sender
unsigned char* mac_addr_sender(unsigned int rx_addr){
	unsigned char src_mac[6];
	#pragma loopbound min 6 max 6
	for (int i=0; i<6; i++){
		src_mac[i] = mem_iord_byte(rx_addr + 6 + i);//ETH header mymac
	}
	return src_mac;
}

//This function retrieves the mac of the destination
unsigned char* mac_addr_dest(unsigned int rx_addr){
	unsigned char dst_mac[6];
	#pragma loopbound min 6 max 6
	for (int i=0; i<6; i++){
		dst_mac[i] = mem_iord_byte(rx_addr + i);//ETH header mymac
	}
	return dst_mac;
}

///////////////////////////////////////////////////////////////
//Support functions related to the MAC layer
///////////////////////////////////////////////////////////////

//This function prints an MAC addrs. If they are the same, it returns 1, otherwise 0.
unsigned char mac_compare_mac(unsigned char mac1[], unsigned char mac2[]){
	if(mac1[0] == mac2[0] && mac1[1] == mac2[1] && mac1[2] == mac2[2] && mac1[3] == mac2[3] && mac1[4] == mac2[4] && mac1[5] == mac2[5]){
		return 1;
	}else{
		return 0;
	}
}

///////////////////////////////////////////////////////////////
//Print functions related to the MAC layer
///////////////////////////////////////////////////////////////

//This function prints an MAC addr received as argument. No special caracters or spaces are appended before or after.
void mac_print_mac(unsigned char mac_addr[]){
	printf("%02X:%02X:%02X:%02X:%02X:%02X", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
	return;
}

//This function prints the system MAC addr. No special caracters or spaces are appended before or after.
void mac_print_my_mac(){
	mac_print_mac(my_mac);
	return;
}

