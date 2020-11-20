/*
 * Copyright (c) 2008, Technische Universitaet Berlin
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright 
 *   notice, this list of conditions and the following disclaimer in the 
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name of the Technische Universitaet Berlin nor the names 
 *   of its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * - Revision -------------------------------------------------------------
 * $Revision: 1.1 $
 * $Date: 2009-03-05 10:07:14 $
 * @author Jan Hauer <hauer@tkn.tu-berlin.de>
 * ========================================================================
 */

 /** Placeholder component for DispatchQueueP: frames are forwarded immediately
  * without queuing, i.e. this component is not a "dead end" like most other
  * placeholders.*/

#include "TKN154_MAC.h"
generic module NoDispatchQueueP() {
  provides
  {
    interface Init as Reset;
    interface FrameTx[uint8_t client];
    interface FrameRx as FrameExtracted[uint8_t client];
    interface Purge;
  } uses {
    interface Queue<ieee154_txframe_t*>;
    interface FrameTx as FrameTxCsma;
    interface FrameRx as SubFrameExtracted;
  }
}
implementation
{
  uint8_t m_client;

  command error_t Reset.init() { return SUCCESS; }

  command ieee154_status_t FrameTx.transmit[uint8_t client](ieee154_txframe_t *txFrame)
  {
    ieee154_status_t status;
    txFrame->client = client;
    status = call FrameTxCsma.transmit(txFrame);
    if (status == IEEE154_SUCCESS)
      m_client = client;
    return status;
  }

  event void FrameTxCsma.transmitDone(ieee154_txframe_t *txFrame, ieee154_status_t status) 
  { 
    signal FrameTx.transmitDone[txFrame->client](txFrame, status);
  }

  event message_t* SubFrameExtracted.received(message_t* frame) 
  { 
    return signal FrameExtracted.received[m_client](frame);
  }

  default event void FrameTx.transmitDone[uint8_t client](ieee154_txframe_t *txFrame, ieee154_status_t status){}

  command ieee154_status_t Purge.purge(uint8_t msduHandle) { return IEEE154_INVALID_HANDLE; }
  
  default event void Purge.purgeDone(ieee154_txframe_t *txFrame, ieee154_status_t status){}
}
