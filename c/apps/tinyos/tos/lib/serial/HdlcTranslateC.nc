//$Id: HdlcTranslateC.nc,v 1.6 2010-06-29 22:07:50 scipio Exp $

/* Copyright (c) 2000-2005 The Regents of the University of California.
 * Copyright (c) 2010 Stanford University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 * - Neither the name of the copyright holder nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * This is an implementation of HDLC serial encoding, supporting framing
 * through frame delimiter bytes and escape bytes.
 *
 * @author Philip Levis
 * @author Ben Greenstein
 * @date September 30 2010
 *
 */

#include "Serial.h"

module HdlcTranslateC {
  provides interface SerialFrameComm;
  uses {
    interface UartStream;
    interface Leds;
  }
}

implementation {
  typedef struct {
    uint8_t sendEscape:1;
    uint8_t receiveEscape:1;
  } HdlcState;
  
  //norace uint8_t debugCnt = 0;
  HdlcState state = {0,0};
  uint8_t txTemp;
  uint8_t m_data;
  
  // TODO: add reset for when SerialM goes no-sync.
  async command void SerialFrameComm.resetReceive(){
    state.receiveEscape = 0;
  }
  async command void SerialFrameComm.resetSend(){
    state.sendEscape = 0;
  }
  async event void UartStream.receivedByte(uint8_t data) {
    //debugCnt++;
    // 7E 41 0E 05 04 03 02 01 00 01 8F 7E
/*     if (debugCnt == 1 && data == 0x7E) call Leds.led0On(); */
/*     if (debugCnt == 2 && data == 0x41) call Leds.led1On(); */
/*     if (debugCnt == 3 && data == 0x0E) call Leds.led2On(); */

    if (data == HDLC_FLAG_BYTE) {
      //call Leds.led1On();
      signal SerialFrameComm.delimiterReceived();
      return;
    }
    else if (data == HDLC_CTLESC_BYTE) {
      //call Leds.led1On();
      state.receiveEscape = 1;
      return;
    }
    else if (state.receiveEscape) {
      //call Leds.led1On();
      state.receiveEscape = 0;
      data = data ^ 0x20;
    }
    signal SerialFrameComm.dataReceived(data);
  }

  async command error_t SerialFrameComm.putDelimiter() {
    atomic {
      state.sendEscape = 0;
      m_data = HDLC_FLAG_BYTE;
    }
    return call UartStream.send(&m_data, 1);
  }
  
  async command error_t SerialFrameComm.putData(uint8_t data) {
    if (data == HDLC_CTLESC_BYTE || data == HDLC_FLAG_BYTE) {
      state.sendEscape = 1;
      txTemp = data ^ 0x20;
      m_data = HDLC_CTLESC_BYTE;
    }
    else {
      m_data = data;
    }
    return call UartStream.send(&m_data, 1);
  }

  async event void UartStream.sendDone( uint8_t* buf, uint16_t len, 
					error_t error ) {
    atomic {
      if (state.sendEscape) {
	state.sendEscape = 0;
	m_data = txTemp;
	call UartStream.send(&m_data, 1);
      }
      else {
	signal SerialFrameComm.putDone();
      }
    }
  }

  async event void UartStream.receiveDone( uint8_t* buf, uint16_t len, error_t error ) {}

}
