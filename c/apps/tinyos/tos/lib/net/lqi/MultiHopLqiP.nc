/*
 * Copyright (c) 2007 Stanford University.
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
 * - Neither the name of the Stanford University nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL STANFORD
 * UNIVERSITY OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
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
 * - Neither the name of the copyright holders nor the names of
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
 * @author Joe Polastre
 * @author Philip Levis (port to TinyOS 2.x)
 */

#include "MultiHopLqi.h"

configuration MultiHopLqiP {
  provides {
    interface StdControl;
    interface Send;
    interface Receive[collection_id_t id];
    interface Receive as Snoop[collection_id_t];
    interface Intercept[collection_id_t id];
    interface RouteControl;
    interface LqiRouteStats;
    interface Packet;
    interface RootControl;
    interface CollectionPacket;
  }

  uses interface CollectionDebug;

}

implementation {

  components LqiForwardingEngineP as Forwarder, LqiRoutingEngineP as Router;
  components 
    new AMSenderC(AM_LQI_BEACON_MSG) as BeaconSender,
    new AMReceiverC(AM_LQI_BEACON_MSG) as BeaconReceiver,
    new AMSenderC(AM_LQI_DATA_MSG) as DataSender,
    new AMSenderC(AM_LQI_DATA_MSG) as DataSenderMine,
    new AMReceiverC(AM_LQI_DATA_MSG) as DataReceiver,
    new TimerMilliC(), 
    NoLedsC, LedsC,
    RandomC,
    ActiveMessageC,
    MainC;

  MainC.SoftwareInit -> Forwarder;
  MainC.SoftwareInit -> Router;
  
  components CC2420ActiveMessageC as CC2420;
  
  StdControl = Router.StdControl;
  
  Receive = Forwarder.Receive;
  Send = Forwarder;
  Intercept = Forwarder.Intercept;
  Snoop = Forwarder.Snoop;
  RouteControl = Forwarder;
  LqiRouteStats = Forwarder;
  Packet = Forwarder;
  CollectionPacket = Forwarder;
  RootControl = Router;
  //CC2420.SubPacket -> DataSender;
  
  Forwarder.SplitControl -> ActiveMessageC;
  Forwarder.RouteSelectCntl -> Router.RouteControl;
  Forwarder.RouteSelect -> Router;
  Forwarder.SubSend -> DataSender;
  Forwarder.SubSendMine -> DataSenderMine;
  Forwarder.SubReceive -> DataReceiver;
  Forwarder.Leds -> LedsC;
  Forwarder.AMPacket -> ActiveMessageC;
  Forwarder.SubPacket -> ActiveMessageC;
  Forwarder.PacketAcknowledgements -> ActiveMessageC;
  Forwarder.RootControl -> Router;
  Forwarder.Random -> RandomC;
  Forwarder.CollectionDebug = CollectionDebug;
  
  Router.AMSend -> BeaconSender;
  Router.Receive -> BeaconReceiver;
  Router.Random -> RandomC;
  Router.Timer -> TimerMilliC; 
  Router.LqiRouteStats -> Forwarder;
  Router.CC2420Packet -> CC2420;
  Router.AMPacket -> ActiveMessageC;
  Router.Packet -> ActiveMessageC;
  Router.Leds -> NoLedsC;
  Router.CollectionDebug = CollectionDebug;
}
