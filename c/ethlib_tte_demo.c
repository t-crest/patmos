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
 * extended to test some initial TTE stuff
 * 
 * Authors: Luca Pezzarossa (lpez@dtu.dk)
 *          Jakob Kenn Toft
 *          Jesper Lønbæk
 *          Russell Barnes
 *          Maja Lund
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
#include "ethlib/tte.h"

#define LED ( *( ( volatile _IODEV unsigned * )	0xF0090000 ) )

unsigned short int led_udp_port = 1234;
unsigned short int calc_udp_port = 1235;
unsigned int rx_addr = 0x000;
unsigned int tx_addr = 0x800;
static unsigned long long start_time;
static unsigned long long end_time;

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

unsigned char print_eth_packets(){
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
	packet_type = mac_packet_type(rx_addr);
	switch (packet_type) {
	  case 0:
		printf("- Level 2 protocol: Ethernet\n");
		printf("\n- Level 3 protocol: UNKNOWN\n");
		printf("\n- Notes:\n");
		printf("  - No actions performed.\n");
		return 0;
	  break;
	  case 1:
		ans = icmp_process_received(rx_addr, tx_addr);
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
		return 1;
	  break;
	  case 2:
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
		return 2;					
	  break;
	  case 3:
		ans = arp_process_received(rx_addr, tx_addr);
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
		return 3;
	  break;
	  default:
		printf("ERROR!\n");
	  break;
	}
	return -1;
}

void print_pcf(){
	printf("- Level 2 protocol: TTE PCF control frame\n");
	unsigned int data;
	data = mem_iord_byte(rx_addr + 12);
	data = (data<<8)|(mem_iord_byte(rx_addr + 13));
	printf("type: 0x%04X\n",data);
	data = mem_iord_byte(rx_addr + 14);
	data = (data << 8) | (mem_iord_byte(rx_addr + 15));
	data = (data << 8) | (mem_iord_byte(rx_addr + 16));
	data = (data << 8) | (mem_iord_byte(rx_addr + 17));
	printf("integration cycle: 0x%08X\n",data);
	data = mem_iord_byte(rx_addr + 18);
	data = (data << 8) | (mem_iord_byte(rx_addr + 19));
	data = (data << 8) | (mem_iord_byte(rx_addr + 20));
	data = (data << 8) | (mem_iord_byte(rx_addr + 21));
	printf("membership new: 0x%08X\n",data);
	data = mem_iord_byte(rx_addr + 26);
	printf("sync prio: 0x%02X\n",data);
	data = mem_iord_byte(rx_addr + 27);
	printf("sync domain: 0x%02X\n",data);
	data = mem_iord_byte(rx_addr + 28);
	printf("type: 0x%01X\n",(data));
	data = mem_iord_byte(rx_addr + 34);
	data = (data << 8) | (mem_iord_byte(rx_addr + 35));
	data = (data << 8) | (mem_iord_byte(rx_addr + 36));
	data = (data << 8) | (mem_iord_byte(rx_addr + 37));
	printf("transparent clock high: 0x%08X\n",data);
	data = mem_iord_byte(rx_addr + 38);
	data = (data << 8) | (mem_iord_byte(rx_addr + 39));
	data = (data << 8) | (mem_iord_byte(rx_addr + 40));
	data = (data << 8) | (mem_iord_byte(rx_addr + 41));
	printf("transparent clock low: 0x%08X\n",data);
	return;
}

void demo_mode(){
	int n = 2000;
	unsigned long long r_pit[n];
	unsigned long long p_pit[n];
	unsigned long long s_pit[n];
	unsigned int int_pd[n];
	unsigned long long trans_clk[n];
	unsigned long long rec_start;
  	unsigned char CT[] = {0xAB,0xAD,0xBA,0xBE};
 	unsigned char VL0[] = {0x0F,0xA1};
 	unsigned char VL1[] = {0x0F,0xA2};

	printf("\nDemo (rx, tx, LED, calculator)\n");
	tte_initialize(0xC3500,CT); //0xC3500 = 10ms in clock cycles // 0x4C4B400 = 1s in clock cycles
	/*tte_prepare_test_data(0x800,VL0,0xaa,200);
	tte_prepare_test_data(0xE00,VL0,0xbb,400);
	tte_prepare_test_data(0x1400,VL0,0xcc,300);
	tte_prepare_test_data(0x1A00,VL0,0xdd,800);
	tte_prepare_test_data(0x2000,VL0,0xee,1514);*/
	arp_table_init();
	for (int i =0; i<n; i++){
		eth_mac_receive(rx_addr, 0);
		rec_start = get_cpu_cycles();
		//printf("...packet #%d received!\n\n", i+1);
		//printf("Packet #%d info:\n", i+1);
		if(is_pcf(rx_addr)){
			//print_pcf();
			if((mem_iord_byte(rx_addr + 28)) == 0x2){
				//printf("integration_frame\n");
				//printf("%d",handle_integration_frame(rx_addr,r_pit,p_pit,s_pit,i));
				if(!handle_integration_frame(rx_addr,rec_start,r_pit,p_pit,s_pit,int_pd,trans_clk,i)){
					n=i+1;
					break;
				}
				if(i>10){
		  		  if(i%2==0){ //max allowed to queue 5 at a time (for this schedule)
		    		    tte_prepare_test_data(0x800,VL0,0xaa,200);
				    tte_schedule_send(0x800,200,0);
				    tte_prepare_test_data(0x2600,VL1,0x11,400);
				    tte_schedule_send(0x2600,400,1);
				    tte_prepare_test_data(0x2C00,VL1,0x22,300);
				    tte_schedule_send(0x2C00,300,1);
				    tte_prepare_test_data(0x3200,VL1,0x33,800);
				    tte_schedule_send(0x3200,800,1);
				    tte_prepare_test_data(0x3800,VL1,0x44,1514);
				    tte_schedule_send(0x3800,1514,1);
				    tte_prepare_test_data(0x3E00,VL1,0x55,400);
				    tte_schedule_send(0x3E00,400,1);
				    tte_prepare_test_data(0x4400,VL1,0x66,300);
				    tte_schedule_send(0x4400,300,1);
				    tte_prepare_test_data(0x4A00,VL1,0x77,800);
				    tte_schedule_send(0x4A00,800,1);
				    tte_prepare_test_data(0x5000,VL1,0x88,1514);
				    tte_schedule_send(0x5000,1514,1);
				    tte_prepare_test_data(0x5600,VL1,0x99,400);
				    tte_schedule_send(0x5600,400,1);
				    tte_prepare_test_data(0x5C00,VL1,0x10,300);
				    tte_schedule_send(0x5C00,300,1);
			  	  }
				  else{
				    tte_prepare_test_data(0x2000,VL0,0xee,1514);
				    tte_schedule_send(0x2000,1514,0);
				    tte_prepare_test_data(0x1A00,VL0,0xdd,800);
				    tte_schedule_send(0x1A00,800,0);
				    tte_prepare_test_data(0x1400,VL0,0xcc,300);
				    tte_schedule_send(0x1400,300,0);
				    tte_prepare_test_data(0xE00,VL0,0xbb,400);
				    tte_schedule_send(0xE00,400,0);
				    //tte_schedule_send(0x800,200,0);
				  }
				}
			}
		} /*else if (is_tte(rx_addr)){
			printf("TTE VERSION!\n");
			printf("CT_marker: 0x%04X\n",mem_iord(rx_addr));
			print_eth_packets();
		}
		else{
			//printf("NOT THE TTE VERSION!\n");
			//print_eth_packets();
		}*/
	}
	tte_stop_ticking();
	/*for (int i =0; i<n; i++){
		printf("%llu %llu %llu %d %llu\n",r_pit[i],p_pit[i],s_pit[i],int_pd[i],trans_clk[i]);
	}*/
	return;
}

void spam_demo(){
  unsigned char CT[] = {0xAB,0xAD,0xBA,0xBE};
  unsigned char VL[] = {0x0F,0xA1};

  tte_initialize(0xC3500,CT);
  tte_prepare_test_data(tx_addr,VL,0xaa,1514);
  while(1!=0){
    //tte_send_test_data(tx_addr);
    printf(".");
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
				//spam_demo();
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


