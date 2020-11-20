/*
 * Copyright (c) 2008 Stanford University.
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

/**
 * BaseStation is a reimplementation of the standard BaseStation application using
 * the TOSThreads thread library.  It transparently forwards any AM messages it
 * receives from its radio interface to its serial interface and vice versa. 
 *
 * <p>On the serial link, BaseStation sends and receives simple active
 * messages (not particular radio packets): on the radio link, it
 * sends radio active messages, whose format depends on the network
 * stack being used. BaseStation will copy its compiled-in group ID to
 * messages moving from the serial link to the radio, and will filter
 * out incoming radio messages that do not contain that group ID.</p>
 *
 * <p>BaseStation includes queues in both directions, with a guarantee
 * that once a message enters a queue, it will eventually leave on the
 * other interface. The queues allow the BaseStation to handle load
 * spikes.</p>
 *
 * <p>BaseStation acknowledges a message arriving over the serial link
 * only if that message was successfully enqueued for delivery to the
 * radio link.</p>
 *
 * <p>The LEDS are programmed to toggle as follows:</p>
 * <ul>
 * <li><b>LED0:</b> Message bridged from serial to radio</li>
 * <li><b>LED1:</b> Message bridged from radio to serial</li>
 * <li><b>LED2:</b> Dropped message due to queue overflow in either direction</li>
 * </ul>
 *
 * @author Kevin Klues <klueska@cs.stanford.edu>
 * @author Chieh-Jan Mike Liang <cliang4@cs.jhu.edu>
 */

#include "base_station.h"
#include "stack.h"
#include "message.h"

configuration BaseStationAppC {}

implementation
{
  components MainC,
             BaseStationC,
             new BaseSendReceiveP() as RadioReceiveSerialSendP,
             new BaseSendReceiveP() as SerialReceiveRadioSendP,
             
             new ThreadC(BOOT_THREAD_STACK_SIZE) as BootThread,
             new ThreadC(RADIO_RECEIVE_THREAD_STACK_SIZE) as RadioReceiveThread,
             new ThreadC(RADIO_SNOOP_THREAD_STACK_SIZE) as RadioSnoopThread,
             new ThreadC(SERIAL_SEND_THREAD_STACK_SIZE) as SerialSendThread,
             new ThreadC(SERIAL_RECEIVE_THREAD_STACK_SIZE) as SerialReceiveThread,
             new ThreadC(RADIO_SEND_THREAD_STACK_SIZE) as RadioSendThread,
             
             new PoolC(message_t, BASE_STATION_MSG_QUEUE_SIZE) as RadioReceivePool,
             new QueueC(message_t*, BASE_STATION_MSG_QUEUE_SIZE) as RadioReceiveQueue,
             new PoolC(message_t, BASE_STATION_MSG_QUEUE_SIZE) as SerialReceivePool,
             new QueueC(message_t*, BASE_STATION_MSG_QUEUE_SIZE) as SerialReceiveQueue,
             
             ThreadSynchronizationC,             
             LedsC;
  
  BaseStationC.Boot -> MainC;
  RadioReceiveSerialSendP.Boot -> BaseStationC;
  SerialReceiveRadioSendP.Boot -> BaseStationC;

  BaseStationC.BootThread -> BootThread;
  RadioReceiveSerialSendP.ReceiveThread -> RadioReceiveThread;
  RadioReceiveSerialSendP.SnoopThread -> RadioSnoopThread;
  RadioReceiveSerialSendP.SendThread -> SerialSendThread;
  SerialReceiveRadioSendP.ReceiveThread -> SerialReceiveThread;
  SerialReceiveRadioSendP.SendThread -> RadioSendThread;  
  
  RadioReceiveSerialSendP.Pool -> RadioReceivePool;
  RadioReceiveSerialSendP.Queue -> RadioReceiveQueue;
  SerialReceiveRadioSendP.Pool -> SerialReceivePool;  
  SerialReceiveRadioSendP.Queue -> SerialReceiveQueue;
  
  RadioReceiveSerialSendP.ConditionVariable -> ThreadSynchronizationC;
  RadioReceiveSerialSendP.Mutex -> ThreadSynchronizationC;
  RadioReceiveSerialSendP.Leds -> LedsC;
  SerialReceiveRadioSendP.ConditionVariable -> ThreadSynchronizationC;
  SerialReceiveRadioSendP.Mutex -> ThreadSynchronizationC;
  SerialReceiveRadioSendP.Leds -> LedsC;
  
  components BlockingActiveMessageC as BlockingRadioActiveMessageC,             
             BlockingSerialActiveMessageC;
             
  BaseStationC.BlockingRadioAMControl -> BlockingRadioActiveMessageC;
  BaseStationC.BlockingSerialAMControl -> BlockingSerialActiveMessageC;
  
  RadioReceiveSerialSendP.ReceivePacket -> BlockingRadioActiveMessageC;
  RadioReceiveSerialSendP.SendPacket -> BlockingSerialActiveMessageC;
  RadioReceiveSerialSendP.ReceiveAMPacket -> BlockingRadioActiveMessageC;
  RadioReceiveSerialSendP.SendAMPacket -> BlockingSerialActiveMessageC;             
  RadioReceiveSerialSendP.BlockingReceiveAny -> BlockingRadioActiveMessageC.BlockingReceiveAny;
  RadioReceiveSerialSendP.BlockingSnoopAny -> BlockingRadioActiveMessageC.BlockingSnoopAny;
  RadioReceiveSerialSendP.BlockingAMSend -> BlockingSerialActiveMessageC;
  
  SerialReceiveRadioSendP.ReceivePacket -> BlockingSerialActiveMessageC;
  SerialReceiveRadioSendP.SendPacket -> BlockingRadioActiveMessageC;
  SerialReceiveRadioSendP.ReceiveAMPacket -> BlockingSerialActiveMessageC;
  SerialReceiveRadioSendP.SendAMPacket -> BlockingRadioActiveMessageC;             
  SerialReceiveRadioSendP.BlockingReceiveAny -> BlockingSerialActiveMessageC.BlockingReceiveAny;
  SerialReceiveRadioSendP.BlockingAMSend -> BlockingRadioActiveMessageC;
}
