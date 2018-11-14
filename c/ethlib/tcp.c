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

#include "tcp.h"

/*
 * Low-level TCP protocol functions
 */
//This function gets the source port of an TCP packet.
unsigned short tcp_get_source_port(unsigned int pkt_addr){
	unsigned short int source_port = (mem_iord_byte(pkt_addr+34) << 8) + (mem_iord_byte(pkt_addr+35));
	return source_port;	
}

//This function gets the destination port of an TCP packet.
unsigned short tcp_get_destination_port(unsigned int pkt_addr){
	unsigned short destination_port = (mem_iord_byte(pkt_addr+36) << 8) + (mem_iord_byte(pkt_addr+37));
	return destination_port;
}

unsigned int tcp_get_seqnum(unsigned int pkt_addr){
	unsigned int sequence	= (mem_iord_byte(pkt_addr+38) << 24) 
							+ (mem_iord_byte(pkt_addr+39) << 16) 
							+ (mem_iord_byte(pkt_addr+40) << 8) 
							+ (mem_iord_byte(pkt_addr+41));
	return sequence;
}

unsigned int tcp_get_acknum(unsigned int pkt_addr){
	unsigned int acknowledgment = (mem_iord_byte(pkt_addr+42) << 24) 
								+ (mem_iord_byte(pkt_addr+43) << 16) 
								+ (mem_iord_byte(pkt_addr+44) << 8) 
								+ (mem_iord_byte(pkt_addr+45));
	return acknowledgment;
}

//This function gets the packet length of an TCP packet.
unsigned char tcp_get_header_length(unsigned int pkt_addr){
	return ((mem_iord_byte(pkt_addr+46))>>2);	
}

unsigned char tcp_get_flags(unsigned int pkt_addr){
	return (mem_iord_byte(pkt_addr+47) & 0xFF);
}

//This function gets the checksum field of an TCP packet.
unsigned short tcp_get_checksum(unsigned int pkt_addr){
	unsigned short int checksum;
	checksum = (mem_iord(pkt_addr+50) >> 16);
	return checksum;	
}

//This function gets the data length field of an TCP packet.
unsigned short tcp_get_data_length(unsigned int pkt_addr){
	unsigned short int data_length;
	data_length = ipv4_get_length(pkt_addr) - 20 - tcp_get_header_length(pkt_addr);
	return data_length;	
}

//This function gets the data field of an TCP packet.
unsigned int tcp_get_data(unsigned int pkt_addr, unsigned char* data, unsigned int data_length){
	int i = 0;
	for (i = 0; i<tcp_get_data_length(pkt_addr); i++){
		data[i] = mem_iord_byte(pkt_addr + 34 + tcp_get_header_length(pkt_addr) + i);
	}
	return i;
}

__attribute__((noinline))
int tcp_send(unsigned int tx_addr, unsigned int rx_addr, tcp_connection conn, unsigned short flags, unsigned char data[], unsigned short data_length){
	unsigned short int tcp_length = data_length + 24;
	unsigned short int ip_length = tcp_length + 20;
	unsigned short int frame_length = ip_length + 14;

	//MAC addrs
	for (int i=0; i<6; i++){
		mem_iowr_byte(tx_addr + i, conn.dstMAC[i]);//ETH header destination
		mem_iowr_byte(tx_addr + 6 + i, conn.srcMAC[i]);//ETH header mymac
	}
	//MAC type + IP version + IP type
	mem_iowr(tx_addr + 12, 0x08004500);
	//IP Length
	mem_iowr_byte(tx_addr + 16, ip_length >> 8);
	mem_iowr_byte(tx_addr + 17, ip_length & 0xFF);
	//Identification
	conn.ipv4_id++;
	mem_iowr_byte(tx_addr + 18, (conn.ipv4_id >> 8));//Need to be changed
	mem_iowr_byte(tx_addr + 19, (conn.ipv4_id & 0xFF));//Need to be changed
	//Flags + TTL + Protocol
	mem_iowr(tx_addr + 20, 0x40004006);
	//Checksum
	mem_iowr_byte(tx_addr + 24, 0x00);//Nobody cares about IP checksum
	mem_iowr_byte(tx_addr + 25, 0x00);//Nobody cares about IP checksum
	//IP addrs
	for (int i=0; i<4; i++){
		mem_iowr_byte(tx_addr + 26 + i, conn.srcIP[i]);//Sender myip
		mem_iowr_byte(tx_addr + 30 + i, conn.dstIP[i]);//Destination ip
	}
	//TCP Source port
	mem_iowr_byte(tx_addr + 34, conn.srcport >> 8);
	mem_iowr_byte(tx_addr + 35, conn.srcport & 0xFF);
	//TCP Destination port
	mem_iowr_byte(tx_addr + 36, conn.dstport >> 8);
	mem_iowr_byte(tx_addr + 37, conn.dstport & 0xFF);
	//TCP Sequence number
	mem_iowr_byte(tx_addr + 38, conn.seqNum >> 24);
	mem_iowr_byte(tx_addr + 39, (conn.seqNum >> 16) & 0xFF);
	mem_iowr_byte(tx_addr + 40, (conn.seqNum >> 8) & 0xFF);
	mem_iowr_byte(tx_addr + 41, conn.seqNum & 0xFF);
	//TCP Acknowledgment number
	mem_iowr_byte(tx_addr + 42, conn.ackNum >> 24);
	mem_iowr_byte(tx_addr + 43, (conn.ackNum >> 16) & 0xFF);
	mem_iowr_byte(tx_addr + 44, (conn.ackNum >> 8) & 0xFF);
	mem_iowr_byte(tx_addr + 45, conn.ackNum & 0xFF);
	//TCP Length
	mem_iowr_byte(tx_addr + 46, (tcp_length-data_length) << 2);
	//TCP Flags
	mem_iowr_byte(tx_addr + 47, (unsigned char)flags & 0xFF);
	//TCP Window size
	mem_iowr_byte(tx_addr + 48, 0x72);//Just 1KB
	mem_iowr_byte(tx_addr + 49, 0x10);
	//TCP Checksum
	mem_iowr_byte(tx_addr + 50, 0x00);//Nobody cares about IP checksum
	mem_iowr_byte(tx_addr + 51, 0x00);//Nobody cares about IP checksum
	//TCP Urgent pointer
	mem_iowr_byte(tx_addr + 52, 0x00);
	mem_iowr_byte(tx_addr + 53, 0x00);
	//TCP MSS [optional]
	mem_iowr_byte(tx_addr + 54, 0x02);
	mem_iowr_byte(tx_addr + 55, 0x04);
	mem_iowr_byte(tx_addr + 56, 0x05);
	mem_iowr_byte(tx_addr + 57, 0xb4);
	//TCP data
	for (int i=0; i<data_length; i++){
		mem_iowr_byte(tx_addr + 58 + i, data[i]);
	}
	//IPv4 checksum
	unsigned short int checksum = ipv4_compute_checksum(tx_addr);
	mem_iowr_byte(tx_addr + 24, (checksum >> 8));
	mem_iowr_byte(tx_addr + 25, (checksum & 0xFF));
	//TCP checksum
	checksum = tcp_compute_checksum(tx_addr, tcp_length, data_length);
	mem_iowr_byte(tx_addr + 50, (checksum >> 8));
	mem_iowr_byte(tx_addr + 51, (checksum & 0xFF));
	//Ethernet send
	eth_mac_send(tx_addr, frame_length);
	return 1;
}

/*
 * High-level TCP protocol functions
 */
const char* tcpstatenames[] = {"closed", "listen", "syn_sent", "syn_rcvd", "established", "pushed", "fin_wait_1", "fin_wait_2", "close_wait", "closing", "last_ack"};

__attribute__((noinline))
void tcp_init_connection(tcp_connection *conn, unsigned char srcMAC[6], unsigned char dstMAC[6], unsigned char srcIP[4], unsigned char dstIP[4], unsigned short srcport, unsigned short dstport){
	for(int i=0; i<6; i++){
		conn->srcMAC[i] = srcMAC[i];
		conn->dstMAC[i] = dstMAC[i];
	}
	for(int i=0; i<4; i++){
		conn->srcIP[i] = srcIP[i];
		conn->dstIP[i] = dstIP[i];
	}
	conn->srcport = srcport;
	conn->dstport = dstport;
	conn->status = CLOSED;
	conn->ipv4_id = 0x1111;
	conn->seqNum = 0x12345678;
	conn->ackNum = 0x0;
}

__attribute__((noinline))
unsigned short tcp_compute_checksum(unsigned int pkt_addr, unsigned short tcp_length, unsigned short data_length){
	unsigned checksum = 0;
	unsigned short hex = 0; 
	unsigned short corrected_length = 0;
	unsigned i = 0;
	//Length alignment
	if ((tcp_length & 0x1) == 0){
		//even
		corrected_length = tcp_length;
	}else{
		//odd
		corrected_length = tcp_length + 1;
		mem_iowr_byte(pkt_addr + 34 + tcp_length , 0x00);
	}
	//Pseudo IP Header
	for (i=0; i<4; i+=2){
		hex = (mem_iord_byte(pkt_addr + 26 + i) << 8) + mem_iord_byte(pkt_addr + 26 + i + 1);
		checksum += hex;
	}
	for (i=0; i<4; i+=2){
		hex = (mem_iord_byte(pkt_addr + 30 + i) << 8) + mem_iord_byte(pkt_addr + 30 + i + 1);
		checksum += hex;
	}
	checksum += 0x0006;
	checksum += (tcp_length);
	//TCP Header
	for (i=0; i<corrected_length; i+=2){
		checksum += (mem_iord_byte(pkt_addr + 34 + i) << 8) + mem_iord_byte(pkt_addr + 34 + i + 1);
	}
	while((checksum >> 16) != 0){
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	}
	//One's complement
	checksum = (~checksum) & 0xFFFF;
	return (unsigned short) checksum;
}

__attribute__((noinline))
int tcp_verify_checksum(unsigned int pkt_addr){
	return 1;
}

int tcp_connect(unsigned int tx_addr, unsigned int rx_addr, tcp_connection* conn){
	tcp_send(tx_addr, rx_addr, *conn, SYN, (unsigned char[]){'0'}, 0);
	printf("conn.status=[%s]->", tcpstatenames[conn->status]);
	conn->status = SYN_SENT;
	printf("[%s]\n", tcpstatenames[conn->status]);
	return 1;
}

int tcp_listen(unsigned int tx_addr, unsigned int rx_addr, tcp_connection* conn){
	tcp_send(tx_addr, rx_addr, *conn, SYN, (unsigned char[]){'0'}, 0);
	printf("conn.status=[%s]->", tcpstatenames[conn->status]);
	conn->status = LISTEN;
	printf("[%s]\n", tcpstatenames[conn->status]);
	return 1;
}

int tcp_close(unsigned int tx_addr, unsigned int rx_addr, tcp_connection* conn){
	tcp_send(tx_addr, rx_addr, *conn, FIN, (unsigned char[]){'0'}, 0);
	printf("conn.status=[%s]->", tcpstatenames[conn->status]);
	switch(conn->status){
		case SYN_RCVD:
			conn->status = FIN_WAIT_1;
			break;
		case ESTABLISHED:
			conn->status = FIN_WAIT_1;
			break;
		case CLOSE_WAIT:
			conn->status = LAST_ACK;
			break;
		default:
			conn->status = CLOSED;
			break;
	}
	printf("[%s]\n", tcpstatenames[conn->status]);
	return 1;
}

__attribute__((noinline))
int tcp_push(unsigned int tx_addr, unsigned int rx_addr, tcp_connection* conn, unsigned char* data, unsigned int data_length){
	tcp_send(tx_addr, rx_addr, *conn, (PSH|ACK), data, data_length);
	printf("conn.status=[%s]->", tcpstatenames[conn->status]);
	conn->status = PUSH;
	printf("[%s]\n", tcpstatenames[conn->status]);
	return 1;
}

__attribute__((noinline))
int tcp_handle(unsigned int tx_addr, unsigned int rx_addr, tcp_connection* conn, unsigned char* data, unsigned int data_length){
	unsigned char resolved = 1;
	unsigned char flags = tcp_get_flags(rx_addr);
	unsigned seqNum = tcp_get_seqnum(rx_addr);
	unsigned short rx_dst_port = tcp_get_destination_port(rx_addr);
	unsigned char tcp_hdr_length = tcp_get_header_length(rx_addr);
	printf("conn.status=[%s]->", tcpstatenames[conn->status]);
	if(rx_dst_port == conn->srcport){
		switch(conn->status){
			case CLOSED:
				tcp_send(tx_addr, rx_addr, *conn, (RST|ACK), (unsigned char[]){'0'}, 0);
				resolved = 1;
				break;
			case LISTEN:
				if((flags & SYN)==SYN){
					tcp_send(tx_addr, rx_addr, *conn, (SYN|ACK), (unsigned char[]){'0'}, 0);
					conn->status = SYN_RCVD;
					resolved = 0;
				} else {
					resolved = 1;
				}
				break;
			case SYN_SENT:
				if(flags==(SYN|ACK)){
					conn->seqNum = tcp_get_acknum(rx_addr);
					conn->ackNum = tcp_get_seqnum(rx_addr) + 1;
					tcp_send(tx_addr, rx_addr, *conn, ACK, (unsigned char[]){'0'}, 0);
					conn->status = ESTABLISHED;
					resolved = 1;
				} else if (flags==(SYN)){
					tcp_send(tx_addr, rx_addr, *conn, (SYN|ACK), (unsigned char[]){'0'}, 0);
					conn->status = SYN_RCVD;
					resolved = 0;
				} else {
					resolved = 0;
				}
				break;
			case SYN_RCVD:
				if((flags & ACK)==ACK){
					conn->status = ESTABLISHED;
					resolved = 1;
				} else {
					resolved = 0;
				}
				break;
			case ESTABLISHED:
				if((flags & PSH)==PSH){
					tcp_get_data(rx_addr, data, data_length);
					resolved = 1;
				}
				if((flags & FIN)==FIN){
					conn->status = CLOSE_WAIT;
					resolved = 0;
				}					
				tcp_send(tx_addr, rx_addr, *conn, ACK, (unsigned char[]){'0'}, 0);
				break;
			case PUSH:
				if((flags & ACK)==ACK){
					conn->status = ESTABLISHED;
					resolved = 1;
				} else {
					resolved = 0;
				}
				break;
			case FIN_WAIT_1:
				if((flags & FIN)==FIN){
					tcp_send(tx_addr, rx_addr, *conn, ACK, (unsigned char[]){'0'}, 0);
					conn->status = CLOSING;
					resolved = 0;
				} else if((flags & FIN)==FIN && (flags & ACK)==ACK){
					tcp_send(tx_addr, rx_addr, *conn, ACK, (unsigned char[]){'0'}, 0);
					conn->status = TIME_WAIT;
					resolved = 0;
				} else if((flags & ACK)==ACK){
					tcp_send(tx_addr, rx_addr, *conn, ACK, (unsigned char[]){'0'}, 0);
					conn->status = FIN_WAIT_2;
					resolved = 0;
				} else {
					resolved = 0;
				}
				break;
			case FIN_WAIT_2:
				if((flags & FIN)==FIN){
					tcp_send(tx_addr, rx_addr, *conn, ACK, (unsigned char[]){'0'}, 0);
					conn->status = CLOSING;
					resolved = 0;
				} else {
					resolved = 0;
				}
				break;
			case CLOSE_WAIT:
				tcp_send(tx_addr, rx_addr, *conn, FIN, (unsigned char[]){'0'}, 0);
				conn->status = LAST_ACK;
				resolved = 0;
				break;
			case CLOSING:
				resolved = 0;
				break;
			case TIME_WAIT:
				conn->status = CLOSED;
				resolved = 1;
				break;
			case LAST_ACK:
				if((flags & FIN)==FIN && (flags & ACK)==ACK){
					tcp_send(tx_addr, rx_addr, *conn, ACK, (unsigned char[]){'0'}, 0);
					conn->status = CLOSED;
					resolved = 1;
				} else {
					resolved = 0;
				}
				break;
			default:
				break;
		}
		printf("[%s]\n", tcpstatenames[conn->status]);
	}
	return resolved;
}