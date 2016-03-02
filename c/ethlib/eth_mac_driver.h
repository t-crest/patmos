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

#ifndef ETH_MAC_DRIVER_H_
#define ETH_MAC_DRIVER_H_

#include <stdio.h>
#include <machine/rtc.h>
#include "eth_patmos_io.h"

///////////////////////////////////////////////////////////////
//High level functions (for the demo)
///////////////////////////////////////////////////////////////

//This function sends an ethernet frame located at tx_addr and of length frame_length.
void eth_mac_send(unsigned int tx_addr, unsigned int frame_length);

//This function receive an ethernet frame and put it in rx_addr.
int eth_mac_receive(unsigned int rx_addr, unsigned long long int timeout);

//This function initilize the ethernet controller (only for the demo).
void eth_mac_initialize();

///////////////////////////////////////////////////////////////
//Regs accessing
///////////////////////////////////////////////////////////////

void set_tx_enable();

unsigned get_tx_enable();

void set_rx_enable();

unsigned get_rx_enable();

void set_int_source_rxb();

unsigned get_int_source_rxb();

void set_tx_db_num(unsigned int data);

unsigned int get_tx_db_num();

void set_mac_address(unsigned int my_mac0, unsigned int my_mac1);

unsigned long long get_mac_address();

void set_tx_bd(unsigned int data);

unsigned int get_tx_db();

unsigned int get_tx_db_mem_pointer();

void set_rx_bd(unsigned int data);

unsigned int get_rx_db();

unsigned get_rx_db_length();

unsigned int get_rx_db_mem_pointer();

void set_rx_db_empty();

unsigned get_rx_db_empty();

void set_rx_db_irq();

unsigned get_rx_db_irq();


/////////////////////
// Help functions
/////////////////////

int eth_read_register(int addr);

void print_register(int addr);

void print_all_register();

#endif 
