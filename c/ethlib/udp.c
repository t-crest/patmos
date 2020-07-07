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
__attribute__((noinline))
unsigned char udp_get_data(unsigned int pkt_addr, unsigned char data[], unsigned int data_length){
	_Pragma("loopbound min 0 max 64")
	for (int i = 0; i<data_length; i+=1){
		data[i] = mem_iord_byte(pkt_addr + 42 + i);
	}
	return 1;
}

void udp_build_packet(udp_t *packet, unsigned char src_ip[4], unsigned char dst_ip[4], unsigned short src_port, unsigned short dst_port, unsigned char* data, unsigned short data_length)
{
	packet->ip_head.ver_headlen = 0x45;
    packet->ip_head.dscp_ecn = 0x00;
    packet->ip_head.length = 20 + sizeof(udphead_t) + data_length;
    packet->ip_head.identification = ipv4_id;
    packet->ip_head.flags_fragmentoff = 0x4000;
    packet->ip_head.ttl = 0x40;
    packet->ip_head.protocol = 0x11;
    packet->ip_head.head_checksum = 0x0;
    packet->ip_head.source_ip[0] = src_ip[0];
    packet->ip_head.source_ip[1] = src_ip[1];
    packet->ip_head.source_ip[2] = src_ip[2];
    packet->ip_head.source_ip[3] = src_ip[3];
    packet->ip_head.destination_ip[0] = dst_ip[0];
    packet->ip_head.destination_ip[1] = dst_ip[1];
    packet->ip_head.destination_ip[2] = dst_ip[2];
    packet->ip_head.destination_ip[3] = dst_ip[3];
	packet->udp_head.source_port = src_port;
	packet->udp_head.destination_port = dst_port;
    packet->udp_head.data_length = data_length;
    packet->udp_head.checksum = 0x0;
    for(int i=0; i<data_length; i+=1)
    {
        packet->data[i] = data[i];
    }
}

///////////////////////////////////////////////////////////////
//Support functions related to the UDP protocol
///////////////////////////////////////////////////////////////

//This function comute and returns the UDP checksum. The function ignore the the field checksum.
__attribute__((noinline))
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
	_Pragma("loopbound min 0 max 14")
	for (int i = 0; i<corrected_length; i=i+2){
		checksum = checksum + (mem_iord_byte(pkt_addr + 34 + i) << 8) + (mem_iord_byte(pkt_addr + 35 + i) & 0xFF);
	}
	checksum = checksum - (mem_iord_byte(pkt_addr + 40) << 8) + (mem_iord_byte(pkt_addr + 41) & 0xFF);
	_Pragma("loopbound min 0 max 2")
	for (int i = 0; i<4; i=i+2){
		checksum = checksum + (mem_iord_byte(pkt_addr + 26 + i) << 8) + (mem_iord_byte(pkt_addr + 27 + i) & 0xFF);
		checksum = checksum + (mem_iord_byte(pkt_addr + 30 + i) << 8) + (mem_iord_byte(pkt_addr + 31 + i) & 0xFF);
	}
	checksum = checksum + 0x0011 + udp_length;
	if ((checksum & 0xFFFF0000) > 0)
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	if ((checksum & 0xFFFF0000) > 0)
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	if ((checksum & 0xFFFF0000) > 0)
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	checksum = ((~checksum) & 0xFFFF);
	return (unsigned short int) checksum;		
}

//This function compute and returns the UDP checksum. The function ignore the the field checksum.
__attribute__((noinline))
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
	_Pragma("loopbound min 0 max 14")
	for (int i = 0; i<corrected_length; i=i+2){
		checksum = checksum + (mem_iord_byte(pkt_addr + 34 + i) << 8) + (mem_iord_byte(pkt_addr + 35 + i) & 0xFF);
	}
	_Pragma("loopbound min 0 max 2")
	for (int i = 0; i<4; i=i+2){
		checksum = checksum + (mem_iord_byte(pkt_addr + 26 + i) << 8) + (mem_iord_byte(pkt_addr + 27 + i) & 0xFF);
		checksum = checksum + (mem_iord_byte(pkt_addr + 30 + i) << 8) + (mem_iord_byte(pkt_addr + 31 + i) & 0xFF);
	}
	checksum = checksum + 0x0011 + udp_length;
	if ((checksum & 0xFFFF0000) > 0)
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	if ((checksum & 0xFFFF0000) > 0)
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	if ((checksum & 0xFFFF0000) > 0)
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	checksum = ((~checksum) & 0xFFFF);
	if (checksum == 0){
		return 1;
	}else{
		return 0;
	}		
}

//This function sends an UDP packet to the dstination IP.
__attribute__((noinline))
int udp_send(unsigned int tx_addr, unsigned int rx_addr, unsigned char destination_ip[], unsigned short source_port, unsigned short destination_port, unsigned char data[], unsigned short data_length, long long timeout){
	//Resolve the ip address
	unsigned char destination_mac[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	if (arp_table_search(destination_ip, destination_mac) == 0){
		if (arp_resolve_ip(rx_addr, tx_addr, destination_ip, timeout)){
			arp_table_search(destination_ip, destination_mac);
		}
	}
	unsigned short int udp_length = data_length + 8;
	unsigned short int ip_length = udp_length + 20;
	unsigned short int frame_length = ip_length + 14;
	//MAC addrs
	mem_iowr(tx_addr, (destination_mac[0] << 24) | (destination_mac[1] << 16) | (destination_mac[2] << 8) | destination_mac[3]);
	mem_iowr(tx_addr + 4, (destination_mac[4] << 24) | (destination_mac[5] << 16) | (my_mac[0] << 8) | my_mac[1]);
	mem_iowr(tx_addr + 8, (my_mac[2] << 24) | (my_mac[3] << 16) | (my_mac[4] << 8) | my_mac[5]);
	//MAC type + IP version + IP type
	mem_iowr(tx_addr + 12, 0x08004500);
	//Length + Identification
	mem_iowr(tx_addr + 16, (ip_length << 16) | (ipv4_id));
	//Flags + TTL + Protocol
	mem_iowr(tx_addr + 20, 0x40004011);
	//IP addrs + Ports + UDP Length
	mem_iowr(tx_addr + 24, (my_ip[0] << 8) | my_ip[1]);
	mem_iowr(tx_addr + 28, (my_ip[2] << 24) | (my_ip[3] << 16) | (destination_ip[0] << 8) | destination_ip[1]);
	mem_iowr(tx_addr + 32, (destination_ip[2] << 24) | (destination_ip[3] << 16) | source_port);
	mem_iowr(tx_addr + 36, (destination_port << 16) | udp_length);
	mem_iowr(tx_addr + 40, 0x0000);
	//UDP Data
	_Pragma("loopbound min 0 max 64")
	for (int i=0; i<data_length; i++){
		mem_iowr_byte(tx_addr + 42 + i, data[i]);//Sender myip
	}
	//IPv4 checksum
	unsigned short int checksum = ipv4_compute_checksum(tx_addr);
	mem_iowr_byte(tx_addr + 24, (checksum >> 8));
	mem_iowr_byte(tx_addr + 25, (checksum & 0xFF));
	// UDP checksum
	checksum = udp_compute_checksum(tx_addr);
	mem_iowr_byte(tx_addr + 40, (checksum >> 8));
	mem_iowr_byte(tx_addr + 41, (checksum & 0xFF));
	eth_mac_send(tx_addr, frame_length);
	ipv4_id+=0x10000;
	return 1;
}

//This function sends an UDP packet to the dstination IP and destination MAC.
__attribute__((noinline))
int udp_send_mac(unsigned int tx_addr, unsigned int rx_addr, unsigned char destination_mac[], unsigned char destination_ip[], unsigned short source_port, unsigned short destination_port, unsigned char data[], unsigned short data_length, long long timeout){
	//Resolve the ip address
	unsigned short udp_length = data_length + 8;
	unsigned short ip_length = udp_length + 20;
	unsigned short frame_length = ip_length + 14;
	//MAC addrs
	mem_iowr(tx_addr, (destination_mac[0] << 24) | (destination_mac[1] << 16) | (destination_mac[2] << 8) | destination_mac[3]);
	mem_iowr(tx_addr + 4, (destination_mac[4] << 24) | (destination_mac[5] << 16) | (my_mac[0] << 8) | my_mac[1]);
	mem_iowr(tx_addr + 8, (my_mac[2] << 24) | (my_mac[3] << 16) | (my_mac[4] << 8) | my_mac[5]);
	//MAC type + IP version + IP type
	mem_iowr(tx_addr + 12, 0x08004500);
	//Length + Identification
	mem_iowr(tx_addr + 16, (ip_length << 16) | (ipv4_id));
	//Flags + TTL + Protocol
	mem_iowr(tx_addr + 20, 0x40004011);
	//IP addrs + Ports + UDP Length
	mem_iowr(tx_addr + 24, (my_ip[0] << 8) | my_ip[1]);
	mem_iowr(tx_addr + 28, (my_ip[2] << 24) | (my_ip[3] << 16) | (destination_ip[0] << 8) | destination_ip[1]);
	mem_iowr(tx_addr + 32, (destination_ip[2] << 24) | (destination_ip[3] << 16) | source_port);
	mem_iowr(tx_addr + 36, (destination_port << 16) | udp_length);
	mem_iowr(tx_addr + 40, 0x0000);
	//UDP Data
	_Pragma("loopbound min 0 max 64")
	for (int i=0; i<data_length; i++){
		mem_iowr_byte(tx_addr + 42 + i, data[i]);//Sender myip
	}
	//IPv4 checksum
	unsigned short int checksum = ipv4_compute_checksum(tx_addr);
	mem_iowr_byte(tx_addr + 24, (checksum >> 8));
	mem_iowr_byte(tx_addr + 25, (checksum & 0xFF));
	// UDP checksum
	checksum = udp_compute_checksum(tx_addr);
	mem_iowr_byte(tx_addr + 40, (checksum >> 8));
	mem_iowr_byte(tx_addr + 41, (checksum & 0xFF));
	eth_mac_send(tx_addr, frame_length);
	ipv4_id+=0x10000;
	return 1;
}

//This function sends a UDP packet
int udp_send_packet(unsigned int tx_addr, unsigned int rx_addr, udp_t packet, long long timeout){
	//Resolve the ip address
	unsigned char destination_mac[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	if (arp_table_search(packet.ip_head.destination_ip, destination_mac) == 0){
		if (arp_resolve_ip(rx_addr, tx_addr, packet.ip_head.destination_ip, timeout)){
			arp_table_search(packet.ip_head.destination_ip, destination_mac);
		}
	}
	unsigned short int udp_length = packet.udp_head.data_length+8;
	unsigned short int ip_length = udp_length + 20;
	unsigned short int frame_length = ip_length + 14;
	//MAC addrs
	mem_iowr(tx_addr, (destination_mac[0] << 24) | (destination_mac[1] << 16) | (destination_mac[2] << 8) | destination_mac[3]);
	mem_iowr(tx_addr + 4, (destination_mac[4] << 24) | (destination_mac[5] << 16) | (my_mac[0] << 8) | my_mac[1]);
	mem_iowr(tx_addr + 8, (my_mac[2] << 24) | (my_mac[3] << 16) | (my_mac[4] << 8) | my_mac[5]);
	//MAC type + IP version + IP type
	mem_iowr(tx_addr + 12, (0x0800 << 16) | 0x4500);
	//Length + Identification
	mem_iowr(tx_addr + 16, (ip_length << 16) | (ipv4_id));
	//Flags + TTL + Protocol
	mem_iowr(tx_addr + 20, 0x40004011);
	//IP addrs + Ports + UDP Length
	mem_iowr(tx_addr + 24, (packet.ip_head.source_ip[0] << 8) | packet.ip_head.source_ip[1]);
	mem_iowr(tx_addr + 28, (packet.ip_head.source_ip[2] << 24) | (packet.ip_head.source_ip[3] << 16) | (packet.ip_head.destination_ip[0] << 8) | packet.ip_head.destination_ip[1]);
	mem_iowr(tx_addr + 32, (packet.ip_head.destination_ip[2] << 24) | (packet.ip_head.destination_ip[3] << 16) | packet.udp_head.source_port);
	mem_iowr(tx_addr + 36, (packet.udp_head.destination_port << 16) | udp_length);
	mem_iowr(tx_addr + 40, 0x0000);
	//UDP Data
	_Pragma("loopbound min 0 max 64")
	for (int i=0; i<packet.udp_head.data_length; i++){
		mem_iowr_byte(tx_addr + 42 + i, packet.data[i]);//Sender myip
	}
	//IPv4 checksum
	unsigned short int checksum = ipv4_compute_checksum(tx_addr);
	mem_iowr_byte(tx_addr + 24, (checksum >> 8));
	mem_iowr_byte(tx_addr + 25, (checksum & 0xFF));
	// UDP checksum
	checksum = udp_compute_checksum(tx_addr);
	mem_iowr_byte(tx_addr + 40, (checksum >> 8));
	mem_iowr_byte(tx_addr + 41, (checksum & 0xFF));
	eth_mac_send(tx_addr, frame_length);
	ipv4_id+=0x10000;
	return 1;
}