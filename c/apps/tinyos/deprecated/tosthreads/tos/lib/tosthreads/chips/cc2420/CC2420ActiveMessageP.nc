/*
 * Copyright (c) 2005 Stanford University. All rights reserved.
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
 * Active message implementation on top of the CC2420 radio. This
 * implementation uses the 16-bit addressing mode of 802.15.4: the
 * only additional byte it adds is the AM id byte, as the first byte
 * of the data payload.
 *
 * @author Philip Levis
 * @version $Revision: 1.3 $ $Date: 2010-06-29 22:07:51 $
 */
 
#include "CC2420.h"

module CC2420ActiveMessageP @safe() {
  provides {
    interface AMSend[am_id_t id];
    interface Receive[am_id_t id];
    interface Receive as ReceiveDefault[am_id_t id];
    interface Receive as Snoop[am_id_t id];
    interface Receive as SnoopDefault[am_id_t id];
    interface AMPacket;
    interface Packet;
    interface SendNotifier[am_id_t id];
    interface RadioBackoff[am_id_t id];
  }
  
  uses {
    interface Send as SubSend;
    interface Receive as SubReceive;
    interface CC2420Packet;
    interface CC2420PacketBody;
    interface CC2420Config;
    interface ActiveMessageAddress;
    interface RadioBackoff as SubBackoff;

    interface Resource as RadioResource;
    interface Leds;
  }
}
implementation {
  uint16_t pending_length;
  message_t *pending_message = NULL;
  /***************** Resource event  ****************/
  event void RadioResource.granted() {
    uint8_t rc;
    cc2420_header_t* header = call CC2420PacketBody.getHeader( pending_message );

    signal SendNotifier.aboutToSend[header->type](header->dest, pending_message);
    rc = call SubSend.send( pending_message, pending_length );
    if (rc != SUCCESS) {
      call RadioResource.release();
      signal AMSend.sendDone[header->type]( pending_message, rc );
    }
  }

  /***************** AMSend Commands ****************/
  command error_t AMSend.send[am_id_t id](am_addr_t addr,
					  message_t* msg,
					  uint8_t len) {
    cc2420_header_t* header = call CC2420PacketBody.getHeader( msg );
    
    if (len > call Packet.maxPayloadLength()) {
      return ESIZE;
    }
    
    header->type = id;
    header->dest = addr;
    header->destpan = call CC2420Config.getPanAddr();
    header->src = call AMPacket.address();
    header->fcf = ( 1 << IEEE154_FCF_INTRAPAN ) |
      ( IEEE154_ADDR_SHORT << IEEE154_FCF_DEST_ADDR_MODE ) |
      ( IEEE154_ADDR_SHORT << IEEE154_FCF_SRC_ADDR_MODE ) ;
    header->length = len + CC2420_SIZE;
    
    if (call RadioResource.immediateRequest() == SUCCESS) {
      error_t rc;
      signal SendNotifier.aboutToSend[id](addr, msg);
      
      rc = call SubSend.send( msg, len );
      if (rc != SUCCESS) {
        call RadioResource.release();
      }

      return rc;
    } else {
      pending_length  = len;
      pending_message = msg;
      return call RadioResource.request();
    }
  }

  command error_t AMSend.cancel[am_id_t id](message_t* msg) {
    return call SubSend.cancel(msg);
  }

  command uint8_t AMSend.maxPayloadLength[am_id_t id]() {
    return call Packet.maxPayloadLength();
  }

  command void* AMSend.getPayload[am_id_t id](message_t* m, uint8_t len) {
    return call Packet.getPayload(m, len);
  }

  /***************** AMPacket Commands ****************/
  command am_addr_t AMPacket.address() {
    return call ActiveMessageAddress.amAddress();
  }
 
  command am_addr_t AMPacket.destination(message_t* amsg) {
    cc2420_header_t* header = call CC2420PacketBody.getHeader(amsg);
    return header->dest;
  }
 
  command am_addr_t AMPacket.source(message_t* amsg) {
    cc2420_header_t* header = call CC2420PacketBody.getHeader(amsg);
    return header->src;
  }

  command void AMPacket.setDestination(message_t* amsg, am_addr_t addr) {
    cc2420_header_t* header = call CC2420PacketBody.getHeader(amsg);
    header->dest = addr;
  }

  command void AMPacket.setSource(message_t* amsg, am_addr_t addr) {
    cc2420_header_t* header = call CC2420PacketBody.getHeader(amsg);
    header->src = addr;
  }

  command bool AMPacket.isForMe(message_t* amsg) {
    return (call AMPacket.destination(amsg) == call AMPacket.address() ||
	    call AMPacket.destination(amsg) == AM_BROADCAST_ADDR);
  }

  command am_id_t AMPacket.type(message_t* amsg) {
    cc2420_header_t* header = call CC2420PacketBody.getHeader(amsg);
    return header->type;
  }

  command void AMPacket.setType(message_t* amsg, am_id_t type) {
    cc2420_header_t* header = call CC2420PacketBody.getHeader(amsg);
    header->type = type;
  }
  
  command am_group_t AMPacket.group(message_t* amsg) {
    return (call CC2420PacketBody.getHeader(amsg))->destpan;
  }

  command void AMPacket.setGroup(message_t* amsg, am_group_t grp) {
    // Overridden intentionally when we send()
    (call CC2420PacketBody.getHeader(amsg))->destpan = grp;
  }

  command am_group_t AMPacket.localGroup() {
    return call CC2420Config.getPanAddr();
  }
  

  /***************** Packet Commands ****************/
  command void Packet.clear(message_t* msg) {
    memset(call CC2420PacketBody.getHeader(msg), 0x0, sizeof(cc2420_header_t));
    memset(call CC2420PacketBody.getMetadata(msg), 0x0, sizeof(cc2420_metadata_t));
  }
  
  command uint8_t Packet.payloadLength(message_t* msg) {
    return (call CC2420PacketBody.getHeader(msg))->length - CC2420_SIZE;
  }
  
  command void Packet.setPayloadLength(message_t* msg, uint8_t len) {
    (call CC2420PacketBody.getHeader(msg))->length  = len + CC2420_SIZE;
  }
  
  command uint8_t Packet.maxPayloadLength() {
    return call SubSend.maxPayloadLength();
  }
  
  command void* Packet.getPayload(message_t* msg, uint8_t len) {
    return call SubSend.getPayload(msg, len);
  }

  
  /***************** SubSend Events ****************/
  event void SubSend.sendDone(message_t* msg, error_t result) {
    call RadioResource.release();
    signal AMSend.sendDone[call AMPacket.type(msg)](msg, result);
  }

  
  /***************** SubReceive Events ****************/
  event message_t* SubReceive.receive(message_t* msg, void* payload, uint8_t len) {
    
    if (call AMPacket.isForMe(msg)) {
      return signal Receive.receive[call AMPacket.type(msg)](msg, payload, len);
    }
    else {
      return signal Snoop.receive[call AMPacket.type(msg)](msg, payload, len);
    }
  }
  

  /***************** ActiveMessageAddress Events ****************/
  async event void ActiveMessageAddress.changed() {
  }
  
  /***************** CC2420Config Events ****************/
  event void CC2420Config.syncDone( error_t error ) {
  }
  
  
  /***************** RadioBackoff ***********************/

  async event void SubBackoff.requestInitialBackoff(message_t *msg) {
    signal RadioBackoff.requestInitialBackoff[(TCAST(cc2420_header_t* ONE,
        (uint8_t*)msg + offsetof(message_t, data) - sizeof(cc2420_header_t)))->type](msg);
  }

  async event void SubBackoff.requestCongestionBackoff(message_t *msg) {
    signal RadioBackoff.requestCongestionBackoff[(TCAST(cc2420_header_t* ONE,
        (uint8_t*)msg + offsetof(message_t, data) - sizeof(cc2420_header_t)))->type](msg);
  }
  async event void SubBackoff.requestCca(message_t *msg) {
    // Lower layers than this do not configure the CCA settings
    signal RadioBackoff.requestCca[(TCAST(cc2420_header_t* ONE,
        (uint8_t*)msg + offsetof(message_t, data) - sizeof(cc2420_header_t)))->type](msg);
  }

  async command void RadioBackoff.setInitialBackoff[am_id_t amId](uint16_t backoffTime) {
    call SubBackoff.setInitialBackoff(backoffTime);
  }
  
  /**
   * Must be called within a requestCongestionBackoff event
   * @param backoffTime the amount of time in some unspecified units to backoff
   */
  async command void RadioBackoff.setCongestionBackoff[am_id_t amId](uint16_t backoffTime) {
    call SubBackoff.setCongestionBackoff(backoffTime);
  }

      
  /**
   * Enable CCA for the outbound packet.  Must be called within a requestCca
   * event
   * @param ccaOn TRUE to enable CCA, which is the default.
   */
  async command void RadioBackoff.setCca[am_id_t amId](bool useCca) {
    call SubBackoff.setCca(useCca);
  }
  
  /***************** Defaults ****************/
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
    call RadioResource.release();
  }

  default event void SendNotifier.aboutToSend[am_id_t amId](am_addr_t addr, message_t *msg) {
  }
  default async event void RadioBackoff.requestInitialBackoff[am_id_t id](
      message_t *msg) {
  }

  default async event void RadioBackoff.requestCongestionBackoff[am_id_t id](
      message_t *msg) {
  }
  
  default async event void RadioBackoff.requestCca[am_id_t id](
      message_t *msg) {
  }
  
}
