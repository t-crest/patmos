/*
 * Copyright (c) 2000-2003 The Regents of the University  of California.  
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
 * Copyright (c) 2002-2003 Intel Corporation
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE     
 * file. If you do not find these files, copies can be found by writing to
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300, Berkeley, CA, 
 * 94704.  Attention:  Intel License Inquiry.
 */



/**
 *
 * @author Gilman Tolle
 * @author Philip Levis (port to TinyOS 2.x)
 */


#include "MultiHopLqi.h"
#include "CollectionDebugMsg.h"

module LqiRoutingEngineP {

  provides {
    interface Init;
    interface StdControl;
    interface RouteSelect;
    interface RouteControl;
    interface RootControl;
  }

  uses {
    interface Timer<TMilli>;
    interface AMSend;
    interface Receive;
    interface Random;
    interface Packet;
    interface AMPacket;
    interface LqiRouteStats;
    interface CC2420Packet;
    interface Leds;
    interface CollectionDebug;
  }
}

implementation {

  enum {
    BASE_STATION_ADDRESS = 0,
    BEACON_PERIOD        = 32,
    BEACON_TIMEOUT       = 8,
  };

  enum {
    ROUTE_INVALID    = 0xff
  };

  bool isRoot = FALSE;
  
  message_t msgBuf;
  bool msgBufBusy;

  uint16_t gbCurrentParent;
  uint16_t gbCurrentParentCost;
  uint16_t gbCurrentLinkEst;
  uint8_t  gbCurrentHopCount;
  uint16_t gbCurrentCost;

  uint8_t gLastHeard;

  int16_t gCurrentSeqNo;
  int16_t gOriginSeqNo;
  
  uint16_t gUpdateInterval;

  uint8_t gRecentIndex;
  uint16_t gRecentPacketSender[MHOP_HISTORY_SIZE];
  int16_t gRecentPacketSeqNo[MHOP_HISTORY_SIZE];

  uint8_t gRecentOriginIndex;
  uint16_t gRecentOriginPacketSender[MHOP_HISTORY_SIZE];
  int16_t gRecentOriginPacketSeqNo[MHOP_HISTORY_SIZE];

  uint16_t adjustLQI(uint8_t val) {
    uint16_t result = (80 - (val - 50));
    result = (((result * result) >> 3) * result) >> 3;
    return result;
  }

  lqi_header_t* getHeader(message_t* msg) {
    return (lqi_header_t*)call Packet.getPayload(msg, sizeof(lqi_header_t));
  }
  
  lqi_beacon_msg_t* getBeacon(message_t* msg) {
    return (lqi_beacon_msg_t*)call Packet.getPayload(msg, sizeof(lqi_beacon_msg_t));
  }

  task void SendRouteTask() {
    lqi_beacon_msg_t* bMsg = getBeacon(&msgBuf);
    uint8_t length = sizeof(lqi_beacon_msg_t);
    
    dbg("LQI","MultiHopRSSI Sending route update msg.\n");

    if (gbCurrentParent != TOS_BCAST_ADDR) {
      dbg("LQI","MultiHopRSSI: Parent = %d\n", gbCurrentParent);
    }
    
    if (msgBufBusy) {
      post SendRouteTask();
      return;
    }

    dbg("LQI","MultiHopRSSI: Current cost: %d.\n", 
	gbCurrentParentCost + gbCurrentLinkEst);

    if (isRoot) {
      bMsg->parent = TOS_NODE_ID;
      bMsg->cost = 0;
      bMsg->originaddr = TOS_NODE_ID;
      bMsg->hopcount = 0;
      bMsg->seqno = gCurrentSeqNo++;
    }
    else {
      bMsg->parent = gbCurrentParent;
      bMsg->cost = gbCurrentParentCost + gbCurrentLinkEst;
      bMsg->originaddr = TOS_NODE_ID;
      bMsg->hopcount = gbCurrentHopCount;
      bMsg->seqno = gCurrentSeqNo++;
    }
    
    if (call AMSend.send(TOS_BCAST_ADDR, &msgBuf, length) == SUCCESS) {
      msgBufBusy = TRUE;
      call CollectionDebug.logEventRoute(NET_C_TREE_SENT_BEACON, bMsg->parent, 0, bMsg->cost);
    }
  }

  task void TimerTask() {
    uint8_t val;
    val = ++gLastHeard;
    if (!isRoot && (val > BEACON_TIMEOUT)) {
      gbCurrentParent = TOS_BCAST_ADDR;
      gbCurrentParentCost = 0x7fff;
      gbCurrentLinkEst = 0x7fff;
      gbCurrentHopCount = ROUTE_INVALID;
      gbCurrentCost = 0xfffe;
    }
    post SendRouteTask();
  }

  command error_t Init.init() {
    int n;

    gRecentIndex = 0;
    for (n = 0; n < MHOP_HISTORY_SIZE; n++) {
      gRecentPacketSender[n] = TOS_BCAST_ADDR;
      gRecentPacketSeqNo[n] = 0;
    }

    gRecentOriginIndex = 0;
    for (n = 0; n < MHOP_HISTORY_SIZE; n++) {
      gRecentOriginPacketSender[n] = TOS_BCAST_ADDR;
      gRecentOriginPacketSeqNo[n] = 0;
    }

    gbCurrentParent = TOS_BCAST_ADDR;
    gbCurrentParentCost = 0x7fff;
    gbCurrentLinkEst = 0x7fff;
    gbCurrentHopCount = ROUTE_INVALID;
    gbCurrentCost = 0xfffe;

    gOriginSeqNo = 0;
    gCurrentSeqNo = 0;
    gUpdateInterval = BEACON_PERIOD;
    msgBufBusy = FALSE;

    return SUCCESS;
  }

  command error_t RootControl.setRoot() {
    call Leds.led2On();
    call CollectionDebug.logEventRoute(NET_C_TREE_NEW_PARENT, TOS_NODE_ID, 0, 0);
    isRoot = TRUE;
    return SUCCESS;
  }

  command error_t RootControl.unsetRoot() {
    isRoot = FALSE;
    return SUCCESS;
  }

  command bool RootControl.isRoot() {
    return isRoot;
  }
  
  command error_t StdControl.start() {
    gLastHeard = 0;
    call Timer.startOneShot(call Random.rand32() % (1024 * gUpdateInterval));
    return SUCCESS;
  }
  
  command error_t StdControl.stop() {
    call Timer.stop();
    return SUCCESS;
  }

  command bool RouteSelect.isActive() {
    return TRUE;
  }

  command error_t RouteSelect.selectRoute(message_t* msg, uint8_t resend) {
    int i;
    lqi_header_t* hdr = getHeader(msg);
    if (isRoot) {
      return FAIL;
    }

    if (hdr->originaddr != TOS_NODE_ID && resend == 0) {
      // supress duplicate packets
      for (i = 0; i < MHOP_HISTORY_SIZE; i++) {
        if ((gRecentPacketSender[i] == call AMPacket.source(msg)) &&
            (gRecentPacketSeqNo[i] == hdr->seqno)) {
	  call CollectionDebug.logEvent(NET_C_FE_DUPLICATE_CACHE_AT_SEND);
	  dbg("LQI", "%s no route as this is a duplicate!\n", __FUNCTION__);
          return FAIL;
        }
      }
    
      gRecentPacketSender[gRecentIndex] = call AMPacket.source(msg);
      gRecentPacketSeqNo[gRecentIndex] = hdr->seqno;
      gRecentIndex = (gRecentIndex + 1) % MHOP_HISTORY_SIZE;

      // supress multihop cycles and try to break out of it
      for (i = 0; i < MHOP_HISTORY_SIZE; i++) {
        if ((gRecentOriginPacketSender[i] == hdr->originaddr) &&
            (gRecentOriginPacketSeqNo[i] == hdr->originseqno)) {
          gbCurrentParentCost = 0x7fff;
          gbCurrentLinkEst = 0x7fff;
          gbCurrentParent = TOS_BCAST_ADDR;
          gbCurrentHopCount = ROUTE_INVALID;
	  dbg("LQI", "%s no route as we are in a cycle!\n", __FUNCTION__);
          return FAIL;
        }
      }
      gRecentOriginPacketSender[gRecentOriginIndex] = hdr->originaddr;
      gRecentOriginPacketSeqNo[gRecentOriginIndex] = hdr->originseqno;
      gRecentOriginIndex = (gRecentOriginIndex + 1) % MHOP_HISTORY_SIZE;
    }

    if (resend == 0) {
      hdr->seqno = gCurrentSeqNo++;
    }
    
    dbg("LQI", "LQI setting destination to %hu and link quality ?\n", gbCurrentParent);
    call AMPacket.setDestination(msg, gbCurrentParent);

    return SUCCESS;
  }

  command error_t RouteSelect.initializeFields(message_t* msg) {
    lqi_header_t* header = getHeader(msg);

    header->originaddr = TOS_NODE_ID;
    header->originseqno = gOriginSeqNo++;
    header->seqno = gCurrentSeqNo;
    
    if (isRoot) {
      header->hopcount = 0;
    }
    else {
      header->hopcount = gbCurrentHopCount;
    }

    dbg("LQI", "LQI setting hopcount to %hhu\n", gbCurrentHopCount);
    return SUCCESS;
  }

  command uint8_t* RouteSelect.getBuffer(message_t* Msg, uint16_t* Len) {

  }


  command uint16_t RouteControl.getParent() {
    return gbCurrentParent;
  }

  command uint8_t RouteControl.getQuality() {
    return gbCurrentLinkEst;
  }

  command uint8_t RouteControl.getDepth() {
    return gbCurrentHopCount;
  }

  command uint8_t RouteControl.getOccupancy() {
    return 0;
  }

  command error_t RouteControl.setUpdateInterval(uint16_t Interval) {

    gUpdateInterval = Interval;
    return SUCCESS;
  }

  command error_t RouteControl.manualUpdate() {
    post SendRouteTask();
    return SUCCESS;
  }


  event void Timer.fired() {
    call Leds.led0Toggle();
    post TimerTask();
    call Timer.startOneShot((uint32_t)1024 * gUpdateInterval + 1);
  }

  event message_t* Receive.receive(message_t* msg, void* payload, uint8_t len) {
    lqi_beacon_msg_t* bMsg = (lqi_beacon_msg_t*)payload;
    am_addr_t source = call AMPacket.source(msg);
    uint8_t lqi = call CC2420Packet.getLqi(msg);
    
    call CollectionDebug.logEventRoute(NET_C_TREE_RCV_BEACON, source, 0, bMsg->cost);
    
    if (isRoot) {
      return msg;
    }
    else {
      dbg("LQI,LQIRoute", "LQI receiving routing beacon from %hu with LQI %hhu that advertises %hu.\n", source, lqi, bMsg->cost);
      if (source == gbCurrentParent) {
	// try to prevent cycles
	if (bMsg->parent != TOS_NODE_ID) {
	  gLastHeard = 0;
	  gbCurrentParentCost = bMsg->cost;
	  gbCurrentLinkEst = adjustLQI(lqi);
	  gbCurrentHopCount = bMsg->hopcount + 1;
	  dbg("LQI,LQIRoute", "  -- Not a loop\n");
	}
	else {
	  gLastHeard = 0;
	  gbCurrentParentCost = 0x7fff;
	  gbCurrentLinkEst = 0x7fff;
	  gbCurrentParent = TOS_BCAST_ADDR;
	  gbCurrentHopCount = ROUTE_INVALID;
	  dbg("LQI,LQIRoute", "  -- Detected a loop\n");
	}
	
      } else {
	
	/* if the message is not from my parent, 
	   compare the message's cost + link estimate to my current cost,
	   switch if necessary */
	
	// make sure you don't pick a parent that creates a cycle
	if (((uint32_t) bMsg->cost + (uint32_t) adjustLQI(lqi) 
	     <
	     ((uint32_t) gbCurrentParentCost + (uint32_t) gbCurrentLinkEst) -
	     (((uint32_t) gbCurrentParentCost + (uint32_t) gbCurrentLinkEst) >> 2)
	     ) &&
	    (bMsg->parent != TOS_NODE_ID)) {
	  gLastHeard = 0;
	  gbCurrentParent = call AMPacket.source(msg);
	  gbCurrentParentCost = bMsg->cost;
	  gbCurrentLinkEst = adjustLQI(lqi);	
	  gbCurrentHopCount = bMsg->hopcount + 1;
	  call CollectionDebug.logEventRoute(NET_C_TREE_NEW_PARENT, gbCurrentParent, 0, gbCurrentParentCost + gbCurrentLinkEst);
	  dbg("LQI,LQIRoute", "  -- Not a cycle.\n");
	}
	else {
	  dbg("LQI,LQIRoute", "  -- CYCLE.\n");
	}
      }
    }
    dbg("LQI,LQIRoute", "Set my count to %hhu, my link to %hu and my cost to %hu.\n", gbCurrentHopCount, gbCurrentLinkEst, gbCurrentParentCost);

    return msg;
  }

  event void AMSend.sendDone(message_t* msg, error_t success) {
    msgBufBusy = FALSE;
  }

    /* Default implementations for CollectionDebug calls.
     * These allow CollectionDebug not to be wired to anything if debugging
     * is not desired. */

  default command error_t CollectionDebug.logEvent(uint8_t type) {
    return SUCCESS;
  }
  default command error_t CollectionDebug.logEventSimple(uint8_t type, uint16_t arg) {
    return SUCCESS;
  }
  default command error_t CollectionDebug.logEventDbg(uint8_t type, uint16_t arg1, uint16_t arg2, uint16_t arg3) {
    return SUCCESS;
  }
  default command error_t CollectionDebug.logEventMsg(uint8_t type, uint16_t msg, am_addr_t origin, am_addr_t node) {
    return SUCCESS;
  }
  default command error_t CollectionDebug.logEventRoute(uint8_t type, am_addr_t parent, uint8_t hopcount, uint16_t etx) {
    return SUCCESS;
  }
  
}
  
