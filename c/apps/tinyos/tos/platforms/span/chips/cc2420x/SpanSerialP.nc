/*
 * Copyright (c) 2008
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/**
 * Conversion of Konrad's conversion
 *
 * @author Konrad Lorincz
 * @date May 14, 2008
 * @author Steve Ayer
 * @date December 2009
 */

module SpanSerialP 
{
    provides interface StdControl;
    provides interface Msp430UartConfigure;
    uses interface Resource;
}
implementation 
{
  enum {
// from http://www.daycounter.com/Calculators/MSP430-Uart-Calculator.phtml
  UBR_4MHZ_4800=0x0369,   UMCTL_4MHZ_4800=0xfb,
  UBR_4MHZ_9600=0x01b4,   UMCTL_4MHZ_9600=0xdf,
  UBR_4MHZ_57600=0x0048,  UMCTL_4MHZ_57600=0xfb,
  UBR_4MHZ_115200=0x0024, UMCTL_4MHZ_115200=0x29,

  UBR_3_7MHZ_115200=0x0020, UMCTL_3_7MHZ_115200=0x00,

 };  	
  
    msp430_uart_union_config_t msp430_uart_telos_config = { {ubr: UBR_4MHZ_115200, umctl: UMCTL_4MHZ_115200, ssel: 0x02, pena: 0, pev: 0, spb: 0, clen: 1, listen: 0, mm: 0, ckpl: 0, urxse: 0, urxeie: 1, urxwie: 0, utxe : 1, urxe : 1} };

    command error_t StdControl.start()
    {
        return call Resource.immediateRequest();
    }

    command error_t StdControl.stop()
    {
        call Resource.release();
        return SUCCESS;
    }

    event void Resource.granted(){}

    async command msp430_uart_union_config_t* Msp430UartConfigure.getConfig() 
    {
        return &msp430_uart_telos_config;
    }
}
