/*
 * Copyright (c) 2005 Arch Rock Corporation 
 * All rights reserved. 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *	Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *	Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *  
 *   Neither the name of the Arch Rock Corporation nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE ARCHED
 * ROCK OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */
/**
 *
 *
 * @author Kaisen Lin
 * @author Phil Buonadonna
 */

#include "DMA.h"

interface HalPXA27xDMAChannel 
{
  
  command error_t requestChannel(DMAPeripheralID_t peripheralID, 
				  DMAPriority_t priority, bool permanent);
  event error_t requestChannelDone();
  command error_t returnChannel(DMAPeripheralID_t peripheralID);
  
  command error_t setSourceAddr(uint32_t val);
  command error_t setTargetAddr(uint32_t val);
  command error_t enableSourceAddrIncrement(bool enable);
  command error_t enableTargetAddrIncrement(bool enable);
  command error_t enableSourceFlowControl(bool enable);
  command error_t enableTargetFlowControl(bool enable);
  command error_t setMaxBurstSize(DMAMaxBurstSize_t size);
  command error_t setTransferLength(uint16_t length);
  command error_t setTransferWidth(DMATransferWidth_t width);
  command error_t run(bool InterruptEn);
  command error_t stop();
  async event void Interrupt();
}
