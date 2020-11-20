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
 * $Revision: 1.2 $
 * $Date: 2009-03-04 18:31:40 $
 * @author Jan Hauer <hauer@tkn.tu-berlin.de>
 * ========================================================================
 */

 /** Empty placeholder component for CoordRealignmentP. */

#include "TKN154_MAC.h"
module NoCoordRealignmentP
{
  provides
  {
    interface Init;
    interface MLME_ORPHAN;
    interface MLME_COMM_STATUS;
    interface GetSet<ieee154_txframe_t*> as GetSetRealignmentFrame;
  }
  uses
  {
    interface FrameTx as CoordRealignmentTx;
    interface FrameRx as OrphanNotificationRx;
    interface FrameUtility;
    interface MLME_GET;
    interface IEEE154Frame as Frame;
    interface Pool<ieee154_txframe_t> as TxFramePool;
    interface Pool<ieee154_txcontrol_t> as TxControlPool;
    interface Get<uint64_t> as LocalExtendedAddress;
  }
}
implementation
{

  command error_t Init.init() { return SUCCESS; }

  command ieee154_txframe_t* GetSetRealignmentFrame.get() { return NULL; }

  command void GetSetRealignmentFrame.set(ieee154_txframe_t*  frame) { }

  event message_t* OrphanNotificationRx.received(message_t* frame) { return frame; }

  command ieee154_status_t MLME_ORPHAN.response (
                          uint64_t OrphanAddress,
                          uint16_t ShortAddress,
                          bool AssociatedMember,
                          ieee154_security_t *security
                        )
  {
    return IEEE154_TRANSACTION_OVERFLOW;
  }
  
  event void CoordRealignmentTx.transmitDone(ieee154_txframe_t *txFrame, ieee154_status_t status)
  {
  }

  default event void MLME_COMM_STATUS.indication (
                          uint16_t PANId,
                          uint8_t SrcAddrMode,
                          ieee154_address_t SrcAddr,
                          uint8_t DstAddrMode,
                          ieee154_address_t DstAddr,
                          ieee154_status_t status,
                          ieee154_security_t *security
                        ){}

  default event void MLME_ORPHAN.indication (
                          uint64_t OrphanAddress,
                          ieee154_security_t *security
                        ){}
}
