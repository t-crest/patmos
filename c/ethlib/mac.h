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
 * MAC section of ethlib (ethernet library)
 * 
 * Authors: Luca Pezzarossa (lpez@dtu.dk)
 *          Jakob Kenn Toft
 *          Jesper Lønbæk
 *          Russell Barnes
 */

#ifndef _MAC_H_
#define _MAC_H_

#include <stdio.h>
#include "eth_patmos_io.h"

extern unsigned char my_mac[6];

///////////////////////////////////////////////////////////////
//Function to decode packet type (for the demo)
///////////////////////////////////////////////////////////////

//This function returns 1 if ICMP, returns 2 if UDP, returns 3 if ARP, otherwise 0.
unsigned char mac_packet_type(unsigned int addr);

//This function retrieves the mac of the sender
void mac_addr_sender(unsigned int rx_addr, unsigned char source_mac[]);

///////////////////////////////////////////////////////////////
//Support functions related to the MAC layer
///////////////////////////////////////////////////////////////

//This function prints an MAC addrs. If they are the same, it returns 1, otherwise 0.
unsigned char mac_compare_mac(unsigned char mac1[], unsigned char mac2[]);

///////////////////////////////////////////////////////////////
//Print functions related to the MAC layer
///////////////////////////////////////////////////////////////

//This function prints an MAC addr received as argument. No special caracters or spaces are appended before or after.
void mac_print_mac(unsigned char mac_addr[]);

//This function prints the system MAC addr. No special caracters or spaces are appended before or after.
void mac_print_my_mac();

#endif
