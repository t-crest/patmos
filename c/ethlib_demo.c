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
 * Main function for ethlib (ethernet library) demo
 * 
 * Authors: Luca Pezzarossa (lpez@dtu.dk)
 *          Jakob Kenn Toft
 *          Jesper Lønbæk
 *          Russell Barnes
 */

#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include "ethlib/icmp.h"
#include "ethlib/icmp.h"
#include "ethlib/arp.h"
#include "ethlib/mac.h"
#include "ethlib/udp.h"
#include "ethlib/ipv4.h"
#include "ethlib/eth_mac_driver.h"

#define LED ( *( ( volatile _IODEV unsigned * )	0xF0090000 ) )

unsigned short int led_udp_port = 1234;
unsigned short int calc_udp_port = 1235;
unsigned int rx_addr = 0x000;
unsigned int tx_addr = 0x800;

void print_general_info(){
	printf("\nGeneral info:\n");
	printf("  MAC: ");
	mac_print_my_mac();
	printf("\n  IP: ");
	ipv4_print_my_ip();
	printf("\n  UDP port for LEDS: %d", led_udp_port);
	printf("\n  UDP port for calculator: %d\n\n", calc_udp_port);
	arp_table_print();
	printf("\n");
	return;
}

void demo_mode(){
	printf("\nDemo (rx, tx, LED, calculator)");
	unsigned char packet_type;
	unsigned char ans;
	unsigned char target_ip[4];
	unsigned char target_mac[6];
	unsigned char sender_ip[4];
	unsigned char sender_mac[6];
	unsigned char udp_data[256];
	unsigned char source_ip[4];	
	unsigned char destination_ip[4];
	unsigned short int destination_port;
	int led_value;
	char opcode;
	int operand_1;
	int operand_2;	
	unsigned int string_length;
	unsigned char tmpip[4] = {192, 168, 24, 1};
	eth_mac_initialize();
	arp_table_init();
	for (int i =0; i<20; i++){

		printf("\n------------------------------------------------------------------------------\n");	
		printf("\nWaiting to receive a packet... \n");
		eth_mac_receive(rx_addr, 0);
		packet_type = mac_packet_type(rx_addr);
		switch (packet_type) {
		case 0:
			printf("...packet #%d received!\n\n", i+1);
			printf("Packet #%d info:\n", i+1);
			printf("- Level 2 protocol: Ethernet\n");
			printf("\n- Level 3 protocol: UNKNOWN\n");
			printf("\n- Notes:\n");
			printf("  - No actions performed.\n");
		break;
		case 1:
			ans = icmp_process_received(rx_addr, tx_addr);
			printf("...packet #%d received!\n\n", i+1);
			printf("Packet #%d info:\n", i+1);
			printf("- Level 2 protocol: Ethernet\n");
			printf("\n- Level 3 protocol: IP\n");
			ipv4_get_source_ip(rx_addr, source_ip);
			ipv4_get_destination_ip(rx_addr, destination_ip);
			printf("  - Source IP: ");
			ipv4_print_source_ip(rx_addr);
			printf("\n  - Destination IP: ");
			ipv4_print_destination_ip(rx_addr);
			if (destination_ip[3] == 255){
				printf(" [BROADCAST]");
			}
			printf("\n  - IP checksum: %04X", ipv4_get_checksum(rx_addr));
			if (ipv4_verify_checksum(rx_addr) == 1){
				printf(" [OK]\n");
			}else{
				printf(" [WRONG]\n");
			}
			printf("\n- Level 3 protocol: ICMP\n");
			if (ans == 0){
				printf("\n- Notes:\n");
				printf("  - ICMP packet not our IP or not a ping request, no actions performed.\n");
			}else{
				printf("\n- Notes:\n");
				printf("  - Ping to our IP, replied.\n");
			}
		break;
		case 2:
			printf("...packet #%d received!\n\n", i+1);
			printf("Packet #%d info:\n", i+1);
			printf("- Level 2 protocol: Ethernet\n");
			printf("\n- Level 3 protocol: IP\n");
			ipv4_get_source_ip(rx_addr, source_ip);
			ipv4_get_destination_ip(rx_addr, destination_ip);
			printf("  - Source IP: ");
			ipv4_print_source_ip(rx_addr);
			printf("\n  - Destination IP: ");
			ipv4_print_destination_ip(rx_addr);
			if (destination_ip[3] == 255){
				printf(" [BROADCAST]");
			}
			printf("\n  - IP checksum: %04X", ipv4_get_checksum(rx_addr));
			if (ipv4_verify_checksum(rx_addr) == 1){
				printf(" [OK]\n");
			}else{
				printf(" [WRONG]\n");
			}
			printf("\n- Level 4 protocol: UDP\n");
			printf("  - Source port: %d\n", udp_get_source_port(rx_addr));
			printf("  - Destination port %d\n", udp_get_destination_port(rx_addr));
			printf("  - UDP checksum: %04X", udp_get_checksum(rx_addr));
			if (udp_verify_checksum(rx_addr) == 1){
				printf(" [OK]\n");
			}else{
				printf(" [WRONG]\n");
			}
			printf("  - Data length %d B\n", udp_get_data_length(rx_addr));
			if (udp_verify_checksum(rx_addr) == 0 || ipv4_verify_checksum(rx_addr) == 0){
				printf("\n- Notes:\n");
				printf("  - Wrong IP and/or UDP checksum, no actions performed.\n");
			}else if (destination_ip[3] == 255){
				printf("\n- Notes:\n");
				printf("  - Broadcast UDP packet, no actions performed.\n");
			}else if(ipv4_compare_ip(my_ip, destination_ip) == 0){
				printf("\n- Notes:\n");
				printf("  - UDP packet not to our IP, no actions performed.\n");
			}else{
				printf("\n- Notes:\n");
				printf("  - UDP packet to our IP ");
				destination_port = udp_get_destination_port(rx_addr);
				if(destination_port == led_udp_port){
					printf("and with UDP destination port for LEDs.\n");
					//Drive the leds
					if (udp_get_data_length(rx_addr)<255){
						udp_get_data(rx_addr, udp_data, udp_get_data_length(rx_addr));
						udp_data[udp_get_data_length(rx_addr)] = '\0';
						printf("  - The UDP data is: %s", udp_data);
						led_value = atoi((char *)udp_data);
						printf("  - The extracted value is %d. The lower byte is 0x%02X\n", led_value, (unsigned char) (led_value & 0xFF));
						LED = (unsigned char)(led_value & 0xFF);
					}else{
						printf("  - The UDP data exceed 256 characters.\n");
					}
				}else if(destination_port == calc_udp_port){
					printf("and with UDP destination port for calculation.\n");
					//Make a calculation
					if (udp_get_data_length(rx_addr)<255){
						udp_get_data(rx_addr, udp_data, udp_get_data_length(rx_addr));
						udp_data[udp_get_data_length(rx_addr)] = '\0';
						printf("  - The UDP data is: %s", udp_data);
						opcode = '0';
						sscanf((char *)udp_data, "%d %c %d", &operand_1, &opcode, &operand_2);
						if (opcode == '+'){
							sprintf((char *)udp_data, "Answer from Patmos: %d %c %d = %d\n", operand_1, opcode, operand_2, operand_1 + operand_2);
							printf("  - Received a valid string, replied.\n");
						}else if(opcode == '-'){
							sprintf((char *)udp_data, "Answer from Patmos: %d %c %d = %d\n", operand_1, opcode, operand_2, operand_1 - operand_2);
							printf("  - Received a valid string, replied.\n");
						}else if(opcode == '*'){
							sprintf((char *)udp_data, "Answer from Patmos: %d %c %d = %d\n", operand_1, opcode, operand_2, operand_1 * operand_2);
							printf("  - Received a valid string, replied.\n");

						}else if(opcode == '/'){
							if (operand_2 == 0){
							sprintf((char *)udp_data, "Answer from Patmos: division by 0, impossible.\n");
							}else{
							sprintf((char *)udp_data, "Answer from Patmos: %d %c %d = %d, R = %d\n", operand_1, opcode, operand_2, operand_1 / operand_2, operand_1 % operand_2);
							}
							printf("  - Received a valid string, replied.\n");						
						}else{
							sprintf((char *)udp_data, "Answer from Patmos: the inserted string is not valid.\n");
							printf("  - Received string not valid, no actions performed.\n");
						}
						string_length = 0;
						do{string_length++;}while(udp_data[string_length-1] != '\n');
						udp_send (tx_addr, rx_addr, tmpip, udp_get_destination_port(rx_addr), udp_get_source_port(rx_addr), udp_data, string_length, 2000000);
					}else{
						printf("  - The UDP data exceed 256 characters.\n");
					}
				}else{
					printf("and with not used UDP destination port, no actions performed.\n");
				}
			}						
		break;
		case 3:
			ans = arp_process_received(rx_addr, tx_addr);
			printf("...packet #%d received!\n\n", i+1);
			printf("Packet #%d info:\n", i+1);
			printf("- Level 2 protocol: Ethernet\n");
			printf("\n- Level 3 protocol: ARP\n");
			arp_get_target_ip(rx_addr, target_ip);
			arp_get_target_mac(rx_addr, target_mac);
			arp_get_sender_ip(rx_addr, sender_ip);
			arp_get_sender_mac(rx_addr, sender_mac);
			printf("  - Sender MAC: ");
			mac_print_mac(sender_mac);
			printf("\n  - Sender IP: ");
			ipv4_print_ip(sender_ip);
			printf("\n  - Target MAC: ");
			mac_print_mac(target_mac);
			printf("\n  - Target IP: ");
			ipv4_print_ip(target_ip);
			printf("\n  - Operation: %d ", arp_get_operation(rx_addr));
			if (arp_get_operation(rx_addr) == 1){
				printf("[request]\n");
			}else if (arp_get_operation(rx_addr) == 2){
				printf("[reply]\n");
			}else{
				printf("\n");
			}
			printf("\n- Notes:\n");
			if (ans == 0){
				printf("  - ARP request not to our IP, no actions performed.\n");
			}else if(ans == 1){
				printf("  - ARP request to our IP, replied.\n");
			}
		break;
		default:
			printf("ERROR!\n");
		break;
		}
		
	}
	return;
}

int main(){

	char c;
	
	LED = 0x00;
	//Print the header
	printf("------------------------------------------------------------------------------\n");
	printf("-    Demo application I for the ETH-MAC contoller and library for Patmos     -\n");
	printf("-               Advanced Computer Architecture - Spring 2015                 -\n");
	printf("------------------------------------------------------------------------------\n");

	//Main loop
	for (;;){
		//Print the list of options and wait for selection
		printf("\nAvailable operations:\n  1 -> Print general information\n  2 -> Start demo (rx, tx, LED, calculator)\n  0 -> Quit\n");
		printf("\nSelect operation: ");
		scanf(" %c", &c);

		//Check if it is a valid operation
		while ((c != '1') && (c != '2') && (c != '0')){
			printf("Operation not valid! Select operation: ");
			scanf(" %c", &c);
		};
		switch (c) {
			case '1':
				print_general_info();
				break;
			case '2':
				demo_mode();
				break;
			case '0':
				printf("\nGoodbye!\n");
				return 0;
				break;
		}
		printf("\n------------------------------------------------------------------------------\n------------------------------------------------------------------------------\n");
	}
	return 0;
}


