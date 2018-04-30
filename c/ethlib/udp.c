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

#include "udp.h"

///////////////////////////////////////////////////////////////
//Functions to get the UDP protocol fields
///////////////////////////////////////////////////////////////

unsigned short int ipv4_id = 0x1111;

//This function gets the source port of an UDP packet.
unsigned short int udp_get_source_port(unsigned int pkt_addr){
	unsigned short int source_port;
	source_port = (mem_iord(pkt_addr+32) & 0x0000FFFF);
	return source_port;	
}

//This function gets the destination port of an UDP packet.
unsigned short int udp_get_destination_port(unsigned int pkt_addr){
	unsigned short int destination_port;
	destination_port = (mem_iord(pkt_addr+36) >> 16);
	return destination_port;	
}

//This function gets the packet length of an UDP packet.
unsigned short int udp_get_packet_length(unsigned int pkt_addr){
	unsigned short int packet_length;
	packet_length = (mem_iord(pkt_addr+36) & 0x0000FFFF);
	return packet_length;	
}

//This function gets the checksum field of an UDP packet.
unsigned short int udp_get_checksum(unsigned int pkt_addr){
	unsigned short int checksum;
	checksum = (mem_iord(pkt_addr+40) >> 16);
	return checksum;	
}

//This function gets the data length field of an UDP packet.
unsigned short int udp_get_data_length(unsigned int pkt_addr){
	unsigned short int data_length;
	data_length = udp_get_packet_length(pkt_addr) - 8;
	return data_length;	
}

//This function gets the data field of an UDP packet.
unsigned char udp_get_data(unsigned int pkt_addr, unsigned char data[], unsigned int data_length){
	if (data_length <= udp_get_data_length(pkt_addr)){	
		for (int i = 0; i<data_length; i++){
			data[i] = mem_iord_byte(pkt_addr + 42 + i);
		}
		return 1;
	}else{
		return 0;
	}	
}

///////////////////////////////////////////////////////////////
//Support functions related to the UDP protocol
///////////////////////////////////////////////////////////////

//This function comute and returns the UDP checksum. The function ignore the the field checksum.
unsigned short int udp_compute_checksum(unsigned int pkt_addr){
	unsigned short int udp_length;
	unsigned short int corrected_length;
	unsigned int checksum;
	udp_length = mem_iord(pkt_addr + 36) & 0xFFFF;
	//
	if ((udp_length & 0x1) == 0){
		//even
		corrected_length = udp_length;
	}else{
		//odd
		corrected_length = udp_length + 1;
		mem_iowr_byte(pkt_addr + udp_length + 34, 0x00);
	}
	checksum = 0;
	for (int i = 0; i<corrected_length; i=i+2){
		checksum = checksum + (mem_iord_byte(pkt_addr + 34 + i) << 8) + (mem_iord_byte(pkt_addr + 35 + i) & 0xFF);
	}
	checksum = checksum - (mem_iord_byte(pkt_addr + 40) << 8) + (mem_iord_byte(pkt_addr + 41) & 0xFF);
	for (int i = 0; i<4; i=i+2){
		checksum = checksum + (mem_iord_byte(pkt_addr + 26 + i) << 8) + (mem_iord_byte(pkt_addr + 27 + i) & 0xFF);
		checksum = checksum + (mem_iord_byte(pkt_addr + 30 + i) << 8) + (mem_iord_byte(pkt_addr + 31 + i) & 0xFF);
	}
	checksum = checksum + 0x0011 + udp_length;
	while((checksum >> 16) != 0){
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	}
	checksum = ((~checksum) & 0xFFFF);
	return (unsigned short int) checksum;		
}

//This function compute and returns the UDP checksum. The function ignore the the field checksum.
int udp_verify_checksum(unsigned int pkt_addr){
	unsigned short int udp_length;
	unsigned short int corrected_length;
	unsigned int checksum;
	udp_length = mem_iord(pkt_addr + 36) & 0xFFFF;
	//
	if ((udp_length & 0x1) == 0){
		//even
		corrected_length = udp_length;
	}else{
		//odd
		corrected_length = udp_length + 1;
		mem_iowr_byte(pkt_addr + udp_length + 34, 0x00);
	}
	checksum = 0;
	for (int i = 0; i<corrected_length; i=i+2){
		checksum = checksum + (mem_iord_byte(pkt_addr + 34 + i) << 8) + (mem_iord_byte(pkt_addr + 35 + i) & 0xFF);
	}
	for (int i = 0; i<4; i=i+2){
		checksum = checksum + (mem_iord_byte(pkt_addr + 26 + i) << 8) + (mem_iord_byte(pkt_addr + 27 + i) & 0xFF);
		checksum = checksum + (mem_iord_byte(pkt_addr + 30 + i) << 8) + (mem_iord_byte(pkt_addr + 31 + i) & 0xFF);
	}
	checksum = checksum + 0x0011 + udp_length;
	while((checksum >> 16) != 0){
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	}
	checksum = ((~checksum) & 0xFFFF);
	if (checksum == 0){
		return 1;
	}else{
		return 0;
	}		
}

//This function sends an UDP packet to the dstination IP.
int udp_send(unsigned int tx_addr, unsigned int rx_addr, unsigned char destination_ip[], unsigned short int source_port, unsigned short int destination_port, unsigned char data[], unsigned short int data_length, long long unsigned int timeout){
	//Resolve the ip address
	unsigned char destination_mac[6];
	if (arp_table_search(destination_ip, destination_mac) == 0){
		if (arp_resolve_ip(rx_addr, tx_addr, destination_ip, timeout) == 0){
			return 0;
		}else{
			arp_table_search(destination_ip, destination_mac);
		}
	}
	unsigned short int udp_length = data_length + 8;
	unsigned short int ip_length = udp_length + 20;
	unsigned short int frame_length = ip_length + 14;

	//MAC addrs
	for (int i=0; i<6; i++){
		mem_iowr_byte(tx_addr + i, destination_mac[i]);//ETH header destination
		mem_iowr_byte(tx_addr + 6 + i, my_mac[i]);//ETH header mymac
	}
	//MAC type + IP version + IP type
	mem_iowr(tx_addr + 12, 0x08004500);

	mem_iowr_byte(tx_addr + 16, ip_length >> 8);
	mem_iowr_byte(tx_addr + 17, ip_length & 0xFF);
	//Identification
	ipv4_id++;
	mem_iowr_byte(tx_addr + 18, (ipv4_id >> 8));//Need to be changed
	mem_iowr_byte(tx_addr + 19, (ipv4_id & 0xFF));//Need to be changed
	//Flags + TTL + Protocol
	mem_iowr(tx_addr + 20, 0x40004011);
	//Checksum
	mem_iowr_byte(tx_addr + 24, 0x00);//Nobody cares about IP checksum
	mem_iowr_byte(tx_addr + 25, 0x00);//Nobody cares about IP checksum
	//IP addrs
	for (int i=0; i<4; i++){
		mem_iowr_byte(tx_addr + 26 + i, my_ip[i]);//Sender myip
		mem_iowr_byte(tx_addr + 30 + i, destination_ip[i]);//Destination ip
	}
	//Source port
	mem_iowr_byte(tx_addr + 34, source_port >> 8);
	mem_iowr_byte(tx_addr + 35, source_port & 0xFF);
	//Destination port
	mem_iowr_byte(tx_addr + 36, destination_port >> 8);
	mem_iowr_byte(tx_addr + 37, destination_port & 0xFF);
	//UDP length
	mem_iowr_byte(tx_addr + 38, udp_length >> 8);
	mem_iowr_byte(tx_addr + 39, udp_length & 0xFF);
	//UDP checksum = 0
	mem_iowr_byte(tx_addr + 40, 0x00);
	mem_iowr_byte(tx_addr + 41, 0x00);
	//UDP data
	for (int i=0; i<data_length; i++){
		mem_iowr_byte(tx_addr + 42 + i, data[i]);//Sender myip
	}
	//IPv4 checksum
	unsigned short int checksum = ipv4_compute_checksum(tx_addr);
	mem_iowr_byte(tx_addr + 24, (checksum >> 8));
	mem_iowr_byte(tx_addr + 25, (checksum & 0xFF));
	//UDP checksum
	checksum = udp_compute_checksum(tx_addr);
	mem_iowr_byte(tx_addr + 40, (checksum >> 8));
	mem_iowr_byte(tx_addr + 41, (checksum & 0xFF));
	eth_mac_send(tx_addr, frame_length);
	return 1;
}

int udp_send_mac(unsigned int tx_addr, unsigned int rx_addr, unsigned char destination_mac[], unsigned char destination_ip[], unsigned short int source_port, unsigned short int destination_port, unsigned char data[], unsigned short int data_length, long long unsigned int timeout){
	//Resolve the ip address
	unsigned short int udp_length = data_length + 8;
	unsigned short int ip_length = udp_length + 20;
	unsigned short int frame_length = ip_length + 14;

	//MAC addrs
	for (int i=0; i<6; i++){
		mem_iowr_byte(tx_addr + i, destination_mac[i]);//ETH header destination
		mem_iowr_byte(tx_addr + 6 + i, my_mac[i]);//ETH header mymac
	}
	//MAC type + IP version + IP type
	mem_iowr(tx_addr + 12, 0x08004500);

	mem_iowr_byte(tx_addr + 16, ip_length >> 8);
	mem_iowr_byte(tx_addr + 17, ip_length & 0xFF);
	//Identification
	ipv4_id++;
	mem_iowr_byte(tx_addr + 18, (ipv4_id >> 8));//Need to be changed
	mem_iowr_byte(tx_addr + 19, (ipv4_id & 0xFF));//Need to be changed
	//Flags + TTL + Protocol
	mem_iowr(tx_addr + 20, 0x40004011);
	//Checksum
	mem_iowr_byte(tx_addr + 24, 0x00);//Nobody cares about IP checksum
	mem_iowr_byte(tx_addr + 25, 0x00);//Nobody cares about IP checksum
	//IP addrs
	for (int i=0; i<4; i++){
		mem_iowr_byte(tx_addr + 26 + i, my_ip[i]);//Sender myip
		mem_iowr_byte(tx_addr + 30 + i, destination_ip[i]);//Destination ip
	}
	//Source port
	mem_iowr_byte(tx_addr + 34, source_port >> 8);
	mem_iowr_byte(tx_addr + 35, source_port & 0xFF);
	//Destination port
	mem_iowr_byte(tx_addr + 36, destination_port >> 8);
	mem_iowr_byte(tx_addr + 37, destination_port & 0xFF);
	//UDP length
	mem_iowr_byte(tx_addr + 38, udp_length >> 8);
	mem_iowr_byte(tx_addr + 39, udp_length & 0xFF);
	//UDP checksum = 0
	mem_iowr_byte(tx_addr + 40, 0x00);
	mem_iowr_byte(tx_addr + 41, 0x00);
	//UDP data
	for (int i=0; i<data_length; i++){
		mem_iowr_byte(tx_addr + 42 + i, data[i]);//Sender myip
	}
	//IPv4 checksum
	unsigned short int checksum = ipv4_compute_checksum(tx_addr);
	mem_iowr_byte(tx_addr + 24, (checksum >> 8));
	mem_iowr_byte(tx_addr + 25, (checksum & 0xFF));
	//UDP checksum
	checksum = udp_compute_checksum(tx_addr);
	mem_iowr_byte(tx_addr + 40, (checksum >> 8));
	mem_iowr_byte(tx_addr + 41, (checksum & 0xFF));
	eth_mac_send(tx_addr, frame_length);
	return 1;
}



