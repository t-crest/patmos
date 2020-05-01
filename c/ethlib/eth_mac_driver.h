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

#define MODER        0x00  //Mode
#define INT_SOURCE   0x04  //Interrupt source
#define INT_MASK     0x08  //Interrupt mask
#define IPGT         0x0C  //Back to back inter packet gap
#define IPGR1        0x10  //Non back to back inter packet gap
#define IPGR2        0x14  //Non back to back inter packet gap
#define PACKETLEN    0x18  //Packet length (minimum and maximum)
#define COLLCONF     0x1C  //Collision and retry configuration
#define TX_BD_NUM    0x20  //Transmit buffer descriptor number
#define CTRLMODER    0x24  //Control module mode
#define MIIMODER     0x28  //MII mode register
#define MIICOMMAND   0x2C  //MII command
#define MIIADDRESS   0x30  //MII address register containts the phy address 
                           //and the register with the phy address

#define RECSMALL_BIT 0x10000
#define PAD_BIT      0x08000
#define HUGEN_BIT    0x04000
#define CRCEN_BIT    0x02000
#define DLYCRCEN_BIT 0x01000
#define FULLD_BIT    0x00400
#define EXDFREN_BIT  0x00200
#define NOBCKOF_BIT  0x00100
#define LOOPBCK_BIT  0x00080
#define IFG_BIT      0x00040
#define PRO_BIT      0x00020
#define IAM_BIT      0x00010
#define BRO_BIT      0x00008
#define NOPRE_BIT    0x00004
#define TXEN_BIT     0x00002
#define RXEN_BIT     0x00001

#define TX_BD_ADDR_BASE             0x400
#define TX_BD_ADDR_END(TX_BD_NUM)   TX_BD_ADDR_BASE + TX_BD_NUM * 8

#define RX_BD_ADDR_BASE(TX_BD_NUM)  TX_BD_ADDR_BASE + TX_BD_NUM * 8
#define RX_BD_ADDR_END              0x7FF

#define INT_SOURCE_RXC_BIT          0x0040
#define INT_SOURCE_TXC_BIT          0x0020
#define INT_SOURCE_BUSY_BIT         0x0010
#define INT_SOURCE_RXE_BIT          0x0008
#define INT_SOURCE_RXB_BIT          0x0004
#define INT_SOURCE_TXE_BIT          0x0002
#define INT_SOURCE_TXB_BIT          0x0001

#define RX_BD_EMPTY_BIT             0x8000
#define RX_BD_IRQEN_BIT             0x4000
#define RX_BD_WRAP_BIT              0x2000
#define RX_BD_CF_BIT                0x0100
#define RX_BD_MISS_BIT              0x0080
#define RX_BD_OR_BIT                0x0040
#define RX_BD_IS_BIT                0x0020
#define RX_BD_DN_BIT                0x0010
#define RX_BD_TL_BIT                0x0008
#define RX_BD_SF_BIT                0x0004
#define RX_BD_CRCERR_BIT            0x0002
#define RX_BD_LC_BIT                0x0001     

#define TX_BD_READY_BIT             0x8000
#define TX_BD_IRQEN_BIT             0x4000
#define TX_BD_WRAP_BIT              0x2000
#define TX_BD_PAD_EN_BIT            0x1000
#define TX_BD_CRC_EN_BIT            0x0800
#define TX_BD_UR_BIT                0x0100
#define TX_BD_RTRY_BIT              0x0010
#define TX_BD_LC_BIT                0x0008
#define TX_BD_DF_BIT                0x0004
#define TX_BD_CS_BIT                0x0001              

// Pointers to the base addresses, all the addressing (addr as arguments)
// in the library are an offset on these addresses
#define ETH_BASE  ((volatile _IODEV unsigned *) (PATMOS_IO_ETH + 0xF000))
#define BUFF_BASE ((volatile _IODEV unsigned *) (PATMOS_IO_ETH + 0x0000))

// Base addresses of second Ethernet controller (when present)
#define ETH1_BASE  ((volatile _IODEV unsigned *) (PATMOS_IO_ETH1 + 0xF000))
#define BUFF1_BASE ((volatile _IODEV unsigned *) (PATMOS_IO_ETH1 + 0x0000))

///////////////////////////////////////////////////////////////
//High level functions (for the demo)
///////////////////////////////////////////////////////////////

//This function sends an ethernet frame located at tx_addr and of length frame_length.
void eth_mac_send(unsigned int tx_addr, unsigned int frame_length);

//This function sends an ethernet frame located at tx_addr and of length frame_length (NON-BLOCKING call).
unsigned eth_mac_send_nb(unsigned int tx_addr, unsigned int frame_length);

//This function receive an ethernet frame and put it in rx_addr.
unsigned eth_mac_receive(unsigned int rx_addr, unsigned long long int timeout);

//This function receive an ethernet frame and put it in rx_addr (Non-blocking call).
unsigned eth_mac_receive_nb(unsigned int rx_addr);

//This function initilize the ethernet controller (only for the demo).
void eth_mac_initialize();

///////////////////////////////////////////////////////////////
//Regs accessing
///////////////////////////////////////////////////////////////

void set_tx_enable();

void clear_tx_enable();

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
