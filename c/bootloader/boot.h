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
 * Definitions for boot loaders.
 * 
 * Authors: Tórur Biskopstø Strøm (torur.strom@gmail.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 */

#ifndef _BOOT_H_
#define _BOOT_H_

#include <machine/patmos.h>
#include <machine/spm.h>

#ifndef __ENTRYPOINT_T
typedef volatile int (*entrypoint_t)(void);
#define __ENTRYPOINT_T
#endif

entrypoint_t download(void) __attribute__((noinline));

// Defines that determine how applications are downloaded
//#define ETHMAC
#define COMPRESSION

#if defined(ETHMAC) && defined(COMPRESSION)
#error "Download via Ethernet does not support compression"
#endif

#ifdef ETHMAC

#include "ethlib/udp.h"
#define RX_ADDR  0x000
#define TX_ADDR  0x800
#define ARP_ADDR 0xc00

#define TARGET_PORT 8888
#define HOST_PORT 8889
extern unsigned char host_ip[];

void ethmac_init(void);
int ethmac_get_byte(void);
void ethmac_put_byte(unsigned char c);

#else /* !ETHMAC */

int uart_read(void);
void uart_write(unsigned char c);

#endif /* !ETHMAC */

#ifdef COMPRESSION

void decompress_init(void);
int decompress_get_byte(void);

#endif /* COMPRESSION */

#endif /* _BOOT_H_ */
