// $Id: CC1000ActiveMessageC.nc,v 1.2 2010-06-29 22:07:51 scipio Exp $

/*
 * Copyright (c) 2004-2005 The Regents of the University  of California.  
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
 * - Neither the name of the University of California nor the names of
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
 *
 * Copyright (c) 2004-2005 Intel Corporation
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE     
 * file. If you do not find these files, copies can be found by writing to
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300, Berkeley, CA, 
 * 94704.  Attention:  Intel License Inquiry.
 */

/**
 *
 * The Active Message layer for the CC1000 radio. This configuration
 * just layers the AM dispatch (CC1000ActiveMessageM) on top of the
 * underlying CC1000 radio packet (CC1000CsmaRadioC), which is
 * inherently an AM packet (acknowledgements based on AM destination
 * addr and group).
 * 
 * @author Philip Levis
 * @date June 19 2005
 */

configuration CC1000ActiveMessageC {
  provides {
    interface SplitControl;
    interface AMSend[am_id_t id];
    interface Receive[am_id_t id];
    interface Receive as ReceiveDefault[am_id_t id];
    interface Receive as Snoop[am_id_t id];
    interface Receive as SnoopDefault[am_id_t id];
    interface AMPacket;
    interface Packet;
    interface PacketAcknowledgements;
    interface LinkPacketMetadata;
  }
}
implementation {

  components CC1000ActiveMessageP as AM, CC1000CsmaRadioC as Radio;
  components ActiveMessageAddressC as Address;
  
  SplitControl = Radio;
  Packet       = Radio;
  PacketAcknowledgements = Radio;
  LinkPacketMetadata = Radio;
  
  AMSend   = AM;
  Receive = AM.Receive;
  ReceiveDefault = AM.ReceiveDefault;
  Snoop = AM.Snoop;
  SnoopDefault = AM.SnoopDefault;
  AMPacket = AM;

  AM.SubSend    -> Radio.Send;
  AM.SubReceive -> Radio.Receive;
  AM.amAddress -> Address;
  AM.Packet     -> Radio;
  
}
