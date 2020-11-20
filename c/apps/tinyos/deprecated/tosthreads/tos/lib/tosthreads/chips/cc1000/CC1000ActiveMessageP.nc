// $Id: CC1000ActiveMessageP.nc,v 1.2 2010-06-29 22:07:51 scipio Exp $

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
 * Implementation component for CC1000ActiveMessageC.
 *
 * @author Philip Levis
 * @date June 19 2006
 */

module CC1000ActiveMessageP @safe() {
  provides {
    interface AMSend[am_id_t id];
    interface Receive[am_id_t id];
    interface Receive as ReceiveDefault[am_id_t id];
    interface Receive as Snoop[am_id_t id];
    interface Receive as SnoopDefault[am_id_t id];
    interface AMPacket;
  }
  uses {
    interface Send as SubSend;
    interface Receive as SubReceive;
    interface Packet as Packet;
    command am_addr_t amAddress();
  }
}
implementation {

  cc1000_header_t* ONE getHeader(message_t* ONE amsg) {
    return TCAST(cc1000_header_t* ONE, (uint8_t*)amsg + offsetof(message_t, data) - sizeof(cc1000_header_t));
  }

  cc1000_footer_t *getFooter(message_t *amsg) {
    return (cc1000_footer_t *)(amsg->footer);
  }
  
  command error_t AMSend.send[am_id_t id](am_addr_t addr,
					  message_t* amsg,
					  uint8_t len) {
    cc1000_header_t* header = getHeader(amsg);
    header->type = id;
    header->dest = addr;
    header->source = call AMPacket.address();
    header->group = TOS_AM_GROUP;
    return call SubSend.send(amsg, len);
  }

  command error_t AMSend.cancel[am_id_t id](message_t* msg) {
    return call SubSend.cancel(msg);
  }

  event void SubSend.sendDone(message_t* msg, error_t result) {
    signal AMSend.sendDone[call AMPacket.type(msg)](msg, result);
  }

  command uint8_t AMSend.maxPayloadLength[am_id_t id]() {
    return call Packet.maxPayloadLength();
  }

  command void* AMSend.getPayload[am_id_t id](message_t* m, uint8_t len) {
    return call Packet.getPayload(m, len);
  }

  /* Receiving a packet */

  event message_t* SubReceive.receive(message_t* msg, void* payload, uint8_t len) {
    cc1000_footer_t* msg_footer = getFooter(msg);
    if(msg_footer->crc == 1) {
      if (call AMPacket.isForMe(msg)) {
        return signal Receive.receive[call AMPacket.type(msg)](msg, payload, len);
      }
      else {
        return signal Snoop.receive[call AMPacket.type(msg)](msg, payload, len);
      }
    }
    return msg;
  }
  
  command am_addr_t AMPacket.address() {
    return call amAddress();
  }
 
  command am_addr_t AMPacket.destination(message_t* amsg) {
    cc1000_header_t* header = getHeader(amsg);
    return header->dest;
  }

  command am_addr_t AMPacket.source(message_t* amsg) {
    cc1000_header_t* header = getHeader(amsg);
    return header->source;
  }

  command void AMPacket.setDestination(message_t* amsg, am_addr_t addr) {
    cc1000_header_t* header = getHeader(amsg);
    header->dest = addr;
  }

  command void AMPacket.setSource(message_t* amsg, am_addr_t addr) {
    cc1000_header_t* header = getHeader(amsg);
    header->source = addr;
  }
  
  command bool AMPacket.isForMe(message_t* amsg) {
    return (call AMPacket.destination(amsg) == call AMPacket.address() ||
	    call AMPacket.destination(amsg) == AM_BROADCAST_ADDR);
  }

  command am_id_t AMPacket.type(message_t* amsg) {
    cc1000_header_t* header = getHeader(amsg);
    return header->type;
  }

  command void AMPacket.setType(message_t* amsg, am_id_t type) {
    cc1000_header_t* header = getHeader(amsg);
    header->type = type;
  }
  
  command void AMPacket.setGroup(message_t* msg, am_group_t group) {
    cc1000_header_t* header = getHeader(msg);
    header->group = group;
  }

  command am_group_t AMPacket.group(message_t* msg) {
    cc1000_header_t* header = getHeader(msg);
    return header->group;
  }

  command am_group_t AMPacket.localGroup() {
    return TOS_AM_GROUP;
  }
  
  default event message_t* Receive.receive[am_id_t id](message_t* msg, void* payload, uint8_t len) {
    return signal ReceiveDefault.receive[id](msg, payload, len);
  }

  default event message_t* ReceiveDefault.receive[am_id_t id](message_t* msg, void* payload, uint8_t len) {
    return msg;
  }
  
  default event message_t* Snoop.receive[am_id_t id](message_t* msg, void* payload, uint8_t len) {
    return signal SnoopDefault.receive[id](msg, payload, len);
  }

  default event message_t* SnoopDefault.receive[am_id_t id](message_t* msg, void* payload, uint8_t len) {
    return msg;
  }

  default event void AMSend.sendDone[uint8_t id](message_t* msg, error_t err) {
    return;
  }

  

}
