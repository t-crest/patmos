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
 * EthMac section of ethlib (ethernet library)
 * 
 * Authors: Luca Pezzarossa (lpez@dtu.dk)
 *          Jakob Kenn Toft
 *          Jesper Lønbæk
 *          Russell Barnes
 */

#include "eth_mac_driver.h"

///////////////////////////////////////////////////////////////
//High level functions (for the demo)
///////////////////////////////////////////////////////////////

//This function sends an ethernet frame located at tx_addr and of length frame_length.
void eth_mac_send(unsigned int tx_addr, unsigned int frame_length){
    unsigned char done = 0;
    unsigned char free = 1; 
    //Wait until buffer is free 
    _Pragma("loopbound min 0 max 1")
	while (free==1){
         free = (eth_iord(0x400) & 0x8000);
    };
    eth_iowr(INT_SOURCE, 0x00000001);
	eth_iowr(0x404, tx_addr);
	eth_iowr(0x400, ((frame_length<<16)|(0xF000)));
    //Wait until is is done
    _Pragma("loopbound min 0 max 1")
    while (done==0){
        done = (eth_iord(0x04) & 0x1);
    }; 
return;
}

//This function sends an ethernet frame located at tx_addr and of length frame_length (NON-BLOCKING call).
unsigned eth_mac_send_nb(unsigned int tx_addr, unsigned int frame_length){
    if((eth_iord(TX_BD_ADDR_BASE) & TX_BD_READY_BIT)==1){
        return 0;
    } else {
	    eth_iowr((TX_BD_ADDR_BASE+4), tx_addr);
	    eth_iowr(TX_BD_ADDR_BASE, ((frame_length<<16)|(0xF000)));
	    eth_iowr(INT_SOURCE, INT_SOURCE_TXB_BIT);
	    eth_iowr(TX_BD_ADDR_BASE, TX_BD_READY_BIT | TX_BD_IRQEN_BIT | TX_BD_WRAP_BIT | TX_BD_PAD_EN_BIT);
    }
    return 1;
}

//This function receive an ethernet frame and put it in rx_addr.
unsigned eth_mac_receive(unsigned int rx_addr, unsigned long long int timeout){
    eth_iowr(RX_BD_ADDR_BASE(eth_iord(TX_BD_NUM))+4, rx_addr);
	if (timeout == 0){
        _Pragma("loopbound min 1 max 1")   //1us - 10ms
		while ((eth_iord(INT_SOURCE) & INT_SOURCE_RXB_BIT)==0){;};
		
	}else{
		unsigned long long int start_time = get_cpu_usecs();
        _Pragma("loopbound min 1 max 1")   //1us
		while (((eth_iord(INT_SOURCE) & INT_SOURCE_RXB_BIT)==0)){
            if((get_cpu_usecs()-start_time >= timeout))
                return 0;
        }
	}
    eth_iowr(INT_SOURCE, INT_SOURCE_RXB_BIT);
    eth_iowr(RX_BD_ADDR_BASE(eth_iord(TX_BD_NUM)), RX_BD_EMPTY_BIT | RX_BD_IRQEN_BIT | RX_BD_WRAP_BIT);
    return 1;
}

//This function receive an ethernet frame and put it in rx_addr (NON-BLOCKING call).
unsigned eth_mac_receive_nb(unsigned int rx_addr){
    unsigned ans = 0;
    eth_iowr(0x04, 0x00000004);
	eth_iowr(0x604, rx_addr);
	eth_iowr(0x600, 0x0000E000);
    if ((eth_iord(0x04) & 0x4)==0){
        ans = 0;
    }else{
        ans = 1;
    }
    return ans;
}

//This function initilize the ethernet controller (only for the demo).
void eth_mac_initialize(){ 
	eth_iowr(0x40, 0xEEF0DA42);
	eth_iowr(0x44, 0x000000FF);
	//MODEREG: PAD|HUGEN|CRCEN|DLYCRCEN|-|FULLD|EXDFREN|NOBCKOF|LOOPBCK|IFG|PRO|IAM|BRO|NOPRE|TXEN|RXEN
	eth_iowr(0x00, 0x0000A423);
	return;
}

///////////////////////////////////////////////////////////////
//Regs accessing
///////////////////////////////////////////////////////////////

void set_tx_enable() {
    unsigned int data = eth_iord(0x00);
    eth_iowr(0x00, data | 0x0002);
}

unsigned get_tx_enable() {
    unsigned int data = eth_iord(0x00);
    return (data & 0x0002);
}

void set_rx_enable() {
    unsigned int data = eth_iord(0x00);
    eth_iowr(0x00, data | 0x0001);
}

unsigned get_rx_enable() {
    unsigned int data = eth_iord(0x00);
    return (data & 0x0001);
}

void set_int_source_rxb() {
    unsigned int data = eth_iord(0x04);
    eth_iowr(0x04, data | (1<<3));
}

unsigned get_int_source_rxb() {
    unsigned int data = eth_iord(0x04);
    return ((data >> 3) & 1);
}

void set_tx_db_num(unsigned int data) {
    if (data < 0x80) {
        eth_iowr(0x20, data);
    } else {
        printf("ERROR: tx_db_num is to high.\n");
        printf("maximum is 0x80.\n");
    }
}

unsigned int get_tx_db_num(){
    return eth_iord(0x20);
}

void set_mac_address(unsigned int my_mac0, unsigned int my_mac1) {
    eth_iowr(0x40, my_mac0);
    eth_iowr(0x44, my_mac1);
}

unsigned long long get_mac_address() {
    unsigned long long my_mac;
    my_mac = eth_iord(0x44); // Getting MSB 2 byte
    my_mac = (my_mac<<32) + eth_iord(0x40);
    return my_mac;
}

void set_tx_bd(unsigned int data) {
    unsigned int base_addr_tx_db = 0x400;
    unsigned int cur_addr_tx_db = base_addr_tx_db;
    eth_iowr(cur_addr_tx_db, data);
}

unsigned int get_tx_db() {
    unsigned int base_addr_tx_db = 0x400;
    unsigned int cur_addr_tx_db = base_addr_tx_db;
    return eth_iord(cur_addr_tx_db);
}

unsigned int get_tx_db_mem_pointer() {
    unsigned int base_addr_tx_db = 0x400;
    unsigned int cur_addr_tx_db = base_addr_tx_db + 0x04;
    return eth_iord(cur_addr_tx_db);
}

void set_rx_bd(unsigned int data) {
    unsigned int tx_db_num = get_tx_db_num();
    unsigned int base_addr_rx_db = 0x400 + (tx_db_num * 0x08);
    unsigned int cur_addr_rx_db = base_addr_rx_db;
    eth_iowr(cur_addr_rx_db, data);
}

unsigned int get_rx_db() {
    unsigned int tx_db_num = get_tx_db_num();
    unsigned int base_addr_rx_db = 0x400 + (tx_db_num * 0x08);
    unsigned int cur_addr_rx_db = base_addr_rx_db;
    return eth_iord(cur_addr_rx_db);
}

unsigned get_rx_db_length() {
    unsigned int tx_db_num = get_tx_db_num();
    unsigned int base_addr_rx_db = 0x400 + (tx_db_num * 0x08);
    unsigned int cur_addr_rx_db = base_addr_rx_db;
    unsigned rx_db = get_rx_db();
    return (rx_db>>16);
}

unsigned int get_rx_db_mem_pointer() {
    unsigned int tx_db_num = get_tx_db_num();
    unsigned int base_addr_rx_db = 0x400 + (tx_db_num * 0x08);
    unsigned int cur_addr_rx_db = base_addr_rx_db + 0x04;
    return eth_iord(cur_addr_rx_db);
}

void set_rx_db_empty() {
    unsigned int tx_db_num = get_tx_db_num();
    unsigned int base_addr_rx_db = 0x400 + (tx_db_num * 0x08);
    unsigned int cur_addr_rx_db = base_addr_rx_db;
    unsigned cur_data = eth_iord(cur_addr_rx_db);
    eth_iowr(cur_addr_rx_db, cur_data | (1<<15));
}

unsigned get_rx_db_empty() {
    unsigned int tx_db_num = get_tx_db_num();
    unsigned int base_addr_rx_db = 0x400 + (tx_db_num * 0x08);
    unsigned int cur_addr_rx_db = base_addr_rx_db;
    unsigned cur_data = eth_iord(cur_addr_rx_db);
    return (cur_data>>15) & 1;
}

void set_rx_db_irq() {
    unsigned int tx_db_num = get_tx_db_num();
    unsigned int base_addr_rx_db = 0x400 + (tx_db_num * 0x08);
    unsigned int cur_addr_rx_db = base_addr_rx_db;
    unsigned cur_data = eth_iord(cur_addr_rx_db);
    eth_iowr(cur_addr_rx_db, cur_data | (1<<14));
}

unsigned get_rx_db_irq() {
    unsigned int tx_db_num = get_tx_db_num();
    unsigned int base_addr_rx_db = 0x400 + (tx_db_num * 0x08);
    unsigned int cur_addr_rx_db = base_addr_rx_db;
    unsigned cur_data = eth_iord(cur_addr_rx_db);
    return (cur_data>>14) & 1;
}

/////////////////////
// Help functions
/////////////////////

int eth_read_register(int addr){
    return eth_iord(addr);
}    

void print_register(int addr) {
    unsigned int data = eth_read_register(addr);
    printf("Addr: 0x%02X \t: 0x%08X\n", addr, data);
}

void print_all_register(){
    unsigned int i;
    for (i = 0; i<=0x50; i=i+0x04) {
        print_register(i);
    } 
}

