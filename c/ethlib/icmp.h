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
 * ICMP section of ethlib (ethernet library)
 * 
 * Authors: Luca Pezzarossa (lpez@dtu.dk)
 *          Jakob Kenn Toft
 *          Jesper Lønbæk
 *          Russell Barnes
 */

#ifndef _ICMP_H_
#define _ICMP_H_

#include <stdio.h>
#include "eth_patmos_io.h"
#include "eth_mac_driver.h"
#include "ipv4.h"
#include "mac.h"

///////////////////////////////////////////////////////////////
//Functions to get the ICMP protocol field
///////////////////////////////////////////////////////////////

//This function return the ICMP type of a ICMP packet starting in pkt_addr.
unsigned char icmp_get_type(unsigned int pkt_addr);

//This function return the ICMP code of a ICMP packet starting in pkt_addr.
unsigned char icmp_get_code(unsigned int pkt_addr);

//This function return the ICMP Quench of a ICMP packet starting in pkt_addr.
unsigned int icmp_get_quench(unsigned int pkt_addr);

///////////////////////////////////////////////////////////////
//Support functions related to the ICMP protocol
///////////////////////////////////////////////////////////////

//This function takes the received ping request icmp paket starting in rx_addr and builds a reply packet starting in tx_addr. rx_addr and tx_addr can be the same.
unsigned int icmp_build_ping_reply(unsigned int rx_addr, unsigned int tx_addr);

//This function process a received ICMP package. If it is a ping request and we are the destination (IP) it reply the ping and returns 1. Otherwise it returns 0.
int icmp_process_received(unsigned int rx_addr, unsigned int tx_addr);

#endif
