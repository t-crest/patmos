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

#ifndef _IPV4_H_
#define _IPV4_H_

#include <stdio.h>
#include "eth_patmos_io.h"

extern unsigned char my_ip[4];

typedef struct
{
   unsigned char ver_headlen;
   unsigned char dscp_ecn;
   unsigned short length;
   unsigned short identification;
   unsigned short flags_fragmentoff;
   unsigned char ttl;
   unsigned char protocol;
   unsigned short head_checksum;
   unsigned char source_ip[4];
   unsigned char destination_ip[4];
} iphead_t;

typedef struct{
   iphead_t ip_head;
   unsigned char* data;
} ip_t;


///////////////////////////////////////////////////////////////
//Functions to get the IPv4 protocol field
///////////////////////////////////////////////////////////////

//This function returns the IPv4 version of a received packet.
char ipv4_get_version(unsigned int pkt_addr);

//This function returns the IPv4 type of service of a received packet.
char ipv4_get_type_of_service(unsigned int pkt_addr);

//This function returns the IPv4 length of a received packet.
unsigned short ipv4_get_length(unsigned int pkt_addr);

//This function returns the IPv4 identification of a received packet.
unsigned int ipv4_get_identification(unsigned int pkt_addr);

//This function returns the IPv4 flags of a received packet.
char ipv4_get_flags(unsigned int pkt_addr);

//This function returns the IPv4 time to live if a received packet.
char ipv4_get_ttl(unsigned int pkt_addr);

//This function returns the IPv4 protocol of a received packet.   
char ipv4_get_protocol(unsigned int pkt_addr);

//This function returns the IPv4 checksum of a received packet.
unsigned int ipv4_get_checksum(unsigned int pkt_addr);

//This function returns the source IP of a received packet.
void ipv4_get_source_ip(unsigned int pkt_addr, unsigned char source_ip[]);

//This function returns the destination IP of a received packet.
void ipv4_get_destination_ip(unsigned int pkt_addr, unsigned char destination_ip[]);

///////////////////////////////////////////////////////////////
//Support functions related to the IPv4 protocol
///////////////////////////////////////////////////////////////

//This function compare 2 IPs. If they are the same, it returns 1, otherwise 0.
int ipv4_compare_ip(unsigned char ip1[], unsigned char ip2[]);

//This function set the IP os the system
void ipv4_set_my_ip(unsigned char new_ip[]);

//This function compute and returns the IP header checksum. The function ignore the the field checksum.
unsigned short int ipv4_compute_checksum(unsigned int pkt_addr);

//This function verify the IP header checksum. If the checksum is correct it returns 1, otherwise it returns 0.
int ipv4_verify_checksum(unsigned int pkt_addr);

///////////////////////////////////////////////////////////////
//Print functions related to the IPv4 protocol
///////////////////////////////////////////////////////////////

//This function prints an IP addr received as argument. No special caracters or spaces are appended before or after.
void ipv4_print_ip(unsigned char ip_addr[]);

//This function prints the system IP addr. No special caracters or spaces are appended before or after.
void ipv4_print_my_ip();

//This function prints the source ip of a received packet. No special caracters or spaces are appended before or after.
void ipv4_print_source_ip(unsigned int pkt_addr);

//This function prints the destination ip of a received packet. No special caracters or spaces are appended before or after.
void ipv4_print_destination_ip(unsigned int pkt_addr);

#endif
