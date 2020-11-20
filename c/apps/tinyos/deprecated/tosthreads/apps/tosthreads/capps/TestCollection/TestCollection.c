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
 * TestCollection is a reimplementation of the Multihop Oscilloscope application
 * using TOSThreads. It periodically samples a universal software-based SineSensor
 * and broadcasts a message every few readings. These readings can be displayed by
 * the Java "Oscilloscope" application found in the the TestCollection/java
 * subdirectory. The sampling rate starts at 4Hz, but can be changed from the Java
 * application.
 * 
 * At least two motes must be used by this application, with one of them installed
 * as a base station.  Base station motes can be created by installing them with
 * NODE_ID % 500 == 0.
 *   i.e. make <platform> cthreads install.0
 *        make <platform> cthreads install.500
 *        make <platform> cthreads install.1000
 * 
 * All other nodes can be installed with arbitrary NODE_IDs.
 *   make <platform> cthreads install.123
 * 
 * Successful running of this application is verified by all NON-base station motes
 * periodically flashing LED1 upon sending a message, and the base station mote,
 * flashing LED2 upon successful reception of a message.  Additionally, correct
 * operation should be verified by running the java tool described in the following
 * section.
 *
 * @author Kevin Klues <klueska@cs.stanford.edu>
 * @author Chieh-Jan Mike Liang <cliang4@cs.jhu.edu>
 */

#include "tosthread.h"
#include "tosthread_amradio.h"
#include "tosthread_amserial.h"
#include "tosthread_leds.h"
#include "tosthread_collection.h"
#include "tosthread_sinesensor.h"
#include "MultihopOscilloscope.h"

void fatal_problem();
void report_problem();
void report_sent();
void report_received();

oscilloscope_t local;
uint8_t reading = 0;   /* 0 to NREADINGS */
message_t sendbuf;
message_t recvbuf;

enum {
  MY_COLLECTION_ID = NEW_COLLECTION_CLIENT_ID(),   // Gets a collection sender instance
};

void tosthread_main(void* arg)
{
  local.interval = DEFAULT_INTERVAL;
  local.id = TOS_NODE_ID;
  local.version = 0;
  
  while ( amRadioStart() != SUCCESS );
  while ( collectionRoutingStart() != SUCCESS );
  
  collectionSetCollectionId(MY_COLLECTION_ID, AM_OSCILLOSCOPE);   // Associates the collection sender
                                                                  //  with AM_OSCILLOSCOPE collection ID
  
  if (local.id % 500 == 0) {
    while ( amSerialStart() != SUCCESS );
    collectionSetRoot();
    for (;;) {
      // Waits for incoming packets with AM_OSCILLOSCOPE collection ID
      if (collectionReceive(&recvbuf, 0, AM_OSCILLOSCOPE) == SUCCESS) {
        oscilloscope_t *recv_o = (oscilloscope_t *) collectionGetPayload(&recvbuf, sizeof(oscilloscope_t));
        oscilloscope_t *send_o = (oscilloscope_t *) serialGetPayload(&sendbuf, sizeof(oscilloscope_t));
        memcpy(send_o, recv_o, sizeof(oscilloscope_t));
        amSerialSend(AM_BROADCAST_ADDR, &sendbuf, sizeof(local), AM_OSCILLOSCOPE);
        report_received();
      }
    }
  } else {
    uint16_t var;

    for (;;) {
      if (reading == NREADINGS) {
        oscilloscope_t *o = (oscilloscope_t *) collectionGetPayload(&sendbuf, sizeof(oscilloscope_t));
        if (o == NULL) {
          fatal_problem();
          return;
        }
        memcpy(o, &local, sizeof(local));
        if (collectionSend(&sendbuf, sizeof(local), MY_COLLECTION_ID) == SUCCESS) {
          local.count++;
          report_sent();
        } else {
          report_problem();
        }
        
        reading = 0;
      }
        
      if (sinesensor_read(&var) == SUCCESS) {
        local.readings[reading++] = var;
      }
      
      tosthread_sleep(local.interval);
    }
  }
}
  
// Use LEDs to report various status issues.
void fatal_problem()
{
  led0On(); 
  led1On();
  led2On();
}

void report_problem() { led0Toggle(); }
void report_sent() { led1Toggle(); }
void report_received() { led2Toggle(); }
