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
 * TCP section of ethlib (ethernet library)
 * 
 * Authors: Eleftherios Kyriakakis (elky@dtu.dk)
 */
#ifndef _TCP_H_
#define _TCP_H_

#include <stdio.h>
#include "eth_patmos_io.h"
#include <machine/rtc.h>
#include "arp.h"
#include "ipv4.h"

/*
 * TCP definitions
 */
#define TCP_RETRY_INTERVAL 3600000 //us
#define TCP_SYN_RETRIES 5   //times
#define TCP_SYNACK_RETRIES 5    //times

enum tcpstate{CLOSED, LISTEN, SYN_SENT, SYN_RCVD, ESTABLISHED, PUSH, FIN_WAIT_1, FIN_WAIT_2, TIME_WAIT, CLOSE_WAIT, CLOSING, LAST_ACK};
enum tcpstatus{UNEXPECTED=-1, UNHANDLED=0, HANDLED=1};
enum tcpflags{FIN=0x1, SYN=0x2, RST=0x4, PSH=0x8, ACK=0x10};

typedef struct {
    unsigned char dstMAC[6];
    unsigned char srcMAC[6];
    unsigned char srcIP[4];
    unsigned char dstIP[4];
    unsigned short int srcport;
    unsigned short int dstport;
    unsigned int seqNum;
    unsigned int ackNum;
    enum tcpstate status;
    unsigned short int ipv4_id;
} tcp_connection;

/*
 * Low-level TCP protocol functions
 */
//This function gets the source port of an TCP packet.
unsigned short tcp_get_source_port(unsigned int pkt_addr);
unsigned short tcp_get_destination_port(unsigned int pkt_addr);
unsigned int tcp_get_seqnum(unsigned int pkt_addr);
unsigned int tcp_get_acknum(unsigned int pkt_addr);
unsigned char tcp_get_header_length(unsigned int pkt_addr);
unsigned char tcp_get_flags(unsigned int pkt_addr);
unsigned short tcp_get_checksum(unsigned int pkt_addr);
unsigned short tcp_get_data_length(unsigned int pkt_addr);
unsigned int tcp_get_data(unsigned int pkt_addr, unsigned char data[], unsigned int data_length);
int tcp_send(unsigned int tx_addr, unsigned int rx_addr, tcp_connection conn, unsigned short flags, unsigned char data[], unsigned short data_length);
unsigned short tcp_compute_checksum(unsigned int pkt_addr, unsigned short tcp_length, unsigned short data_length);
int tcp_verify_checksum(unsigned int pkt_addr);

/*
 * High-level TCP protocol functions
 */
void tcp_init_connection(tcp_connection *conn, unsigned char srcMAC[6], unsigned char dstMAC[6], unsigned char srcIP[4], unsigned char dstIP[4], unsigned short srcport, unsigned short dstport);
int tcp_connect(unsigned int tx_addr, unsigned int rx_addr, tcp_connection* conn);
int tcp_listen(unsigned int tx_addr, unsigned int rx_addr, tcp_connection* conn);
int tcp_close(unsigned int tx_addr, unsigned int rx_addr, tcp_connection* conn);
int tcp_push(unsigned int tx_addr, unsigned int rx_addr, tcp_connection* conn, unsigned char* data, unsigned int data_length);
int tcp_handle(unsigned int tx_addr, unsigned int rx_addr, tcp_connection* conn, unsigned char* data, unsigned int data_length);

#endif