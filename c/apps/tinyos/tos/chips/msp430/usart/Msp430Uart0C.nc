/**
 * Copyright (c) 2005-2006 Arched Rock Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 * - Neither the name of the Arched Rock Corporation nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * ARCHED ROCK OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE
 */

/**
 * An implementation of the UART on USART0 for the MSP430.
 * @author Vlado Handziski <handzisk@tkn.tu-berlin.de>
 * @author Jonathan Hui <jhui@archedrock.com>
 * @author Eric B. Decker <cire831@gmail.com>
 * @version $Revision: 1.6 $ $Date: 2008-05-21 22:11:57 $
 */

#include "msp430usart.h"

generic configuration Msp430Uart0C() {

  provides interface Resource;
  provides interface ResourceRequested;
  provides interface UartStream;
  provides interface UartByte;

  uses interface Msp430UartConfigure;
}

implementation {

  enum {
    CLIENT_ID = unique( MSP430_UARTO_BUS ),
  };

  components Msp430Uart0P as UartP;
  Resource = UartP.Resource[ CLIENT_ID ];
  UartStream = UartP.UartStream[ CLIENT_ID ];
  UartByte = UartP.UartByte[ CLIENT_ID ];
  Msp430UartConfigure = UartP.Msp430UartConfigure[ CLIENT_ID ];

  components new Msp430Usart0C() as UsartC;
  ResourceRequested = UsartC;
  UartP.ResourceConfigure[ CLIENT_ID ] <- UsartC.ResourceConfigure;
  UartP.UsartResource[ CLIENT_ID ] -> UsartC.Resource;
  UartP.UsartInterrupts[ CLIENT_ID ] -> UsartC.HplMsp430UsartInterrupts;
}
