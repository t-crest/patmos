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
 * IPv4 section of ethlib (ethernet library)
 * 
 * Authors: Luca Pezzarossa (lpez@dtu.dk)
 *          Jakob Kenn Toft
 *          Jesper Lønbæk
 *          Russell Barnes
 */

#include "ipv4.h"

unsigned char my_ip[4] = {192, 168, 24, 50};

///////////////////////////////////////////////////////////////
//Functions to get the IPv4 protocol field
///////////////////////////////////////////////////////////////

//This function returns the IPv4 version of a received packet.
char ipv4_get_version(unsigned int pkt_addr){
	char version;
	version = mem_iord_byte(pkt_addr + 14);
	version = version >> 4; //Version is only a nibble, thus a shift of 4
	return version;
}

//This function returns the IPv4 type of service of a received packet.
char ipv4_get_type_of_service(unsigned int pkt_addr){
	char type;
	type = mem_iord_byte(pkt_addr + 14 + 1);
	type = type >> 2; //Type is 6 bits, thus a shift of 2 bits
	return type;
}

//This function returns the IPv4 length of a received packet.
char ipv4_get_length(unsigned int pkt_addr){
	unsigned int length;
	length = mem_iord_byte(pkt_addr + 14 + 2); //Upper Byte
	length = length << 8;
	length = length + mem_iord_byte(pkt_addr + 14 + 3); //Lowe Byte
	return length;
}

//This function returns the IPv4 identification of a received packet.
unsigned int ipv4_get_identification(unsigned int pkt_addr){
	unsigned int identification;
	identification = mem_iord_byte(pkt_addr + 14 + 4); //Upper Byte
	identification = identification << 8;
	identification = identification + mem_iord_byte(pkt_addr + 14 + 5); //Lower Byte
	return identification;
}

//This function returns the IPv4 flags of a received packet.
char ipv4_get_flags(unsigned int pkt_addr){
	char flags;
	flags = mem_iord_byte(pkt_addr + 14 + 6);
	flags = flags >> 5; //Flags is only 3 bits, thus the shift
	return flags;
}

//This function returns the IPv4 time to live if a received packet.
char ipv4_get_ttl(unsigned int pkt_addr){
	return mem_iord_byte(pkt_addr + 14 + 8);
}

//This function returns the IPv4 protocol of a received packet.
char ipv4_get_protocol(unsigned int pkt_addr){
	return mem_iord_byte(pkt_addr + 14 + 9);
}

//This function returns the IPv4 checksum of a received packet.
unsigned int ipv4_get_checksum(unsigned int pkt_addr){
	unsigned int checksum;
	checksum = (mem_iord_byte(pkt_addr + 24)<< 8) + (mem_iord_byte(pkt_addr + 25) & 0xFF);
	return checksum;
}

//This function returns the source IP of a received packet.
void ipv4_get_source_ip(unsigned int pkt_addr, unsigned char source_ip[]){
	int i;
	for (i=0; i<4; i++){
		source_ip[i] = mem_iord_byte(pkt_addr+26+i);
	}
	return;	
}

//This function returns the destination IP of a received packet.
void ipv4_get_destination_ip(unsigned int pkt_addr, unsigned char destination_ip[]){
	int i;
	for (i=0; i<4; i++){
		destination_ip[i] = mem_iord_byte(pkt_addr+30+i);
	}
	return;
}

///////////////////////////////////////////////////////////////
//Support functions related to the IPv4 protocol
///////////////////////////////////////////////////////////////

//This function compare 2 IPs. If they are the same, it returns 1, otherwise 0.
int ipv4_compare_ip(unsigned char ip1[], unsigned char ip2[]){
	if(ip1[0] == ip2[0] && ip1[1] == ip2[1] && ip1[2] == ip2[2] && ip1[3] == ip2[3]){
		return 1;
	}else{
		return 0;
	}
}

//This function set the IP os the system
void ipv4_set_my_ip(unsigned char new_ip[]){
	int i;
	for (i=0; i<4; i++){
		my_ip[i] = new_ip[i];
	}
	return;
}

//This function compute and returns the IP header checksum. The function ignore the the field checksum.
unsigned short int ipv4_compute_checksum(unsigned int pkt_addr){
	unsigned int checksum;
	checksum = 0;
	for (int i = 0; i<20; i=i+2){
		checksum = checksum + (mem_iord_byte(pkt_addr + 14 + i) << 8) + (mem_iord_byte(pkt_addr + 15 + i) & 0xFF);
	}
	checksum = checksum - (mem_iord_byte(pkt_addr + 24) << 8) + (mem_iord_byte(pkt_addr + 25) & 0xFF);
	while((checksum >> 16) != 0){
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	}
	checksum = ((~checksum) & 0xFFFF);
	return (unsigned short int) checksum;		
}

//This function verify the IP header checksum. If the checksum is correct it returns 1, otherwise it returns 0.
int ipv4_verify_checksum(unsigned int pkt_addr){
	unsigned int checksum;
	checksum = 0;
	for (int i = 0; i<20; i=i+2){
		checksum = checksum + (mem_iord_byte(pkt_addr + 14 + i) << 8) + (mem_iord_byte(pkt_addr + 15 + i) & 0xFF);
	}
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


///////////////////////////////////////////////////////////////
//Print functions related to the IPv4 protocol
///////////////////////////////////////////////////////////////

//This function prints an IP addr received as argument. No special caracters or spaces are appended before or after.
void ipv4_print_ip(unsigned char ip_addr[]){
	printf("%d.%d.%d.%d", ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
	return;
}

//This function prints the system IP addr. No special caracters or spaces are appended before or after.
void ipv4_print_my_ip(){
	ipv4_print_ip(my_ip);
	return;
}

//This function prints the source ip of a received packet. No special caracters or spaces are appended before or after.
void ipv4_print_source_ip(unsigned int pkt_addr){
	unsigned char source_ip[4];
	ipv4_get_source_ip(pkt_addr, source_ip);
	ipv4_print_ip(source_ip);
	return;
}

//This function prints the destination ip of a received packet. No special caracters or spaces are appended before or after.
void ipv4_print_destination_ip(unsigned int pkt_addr){
	unsigned char destination_ip[4];
	ipv4_get_destination_ip(pkt_addr, destination_ip);
	ipv4_print_ip(destination_ip);
	return;
}

