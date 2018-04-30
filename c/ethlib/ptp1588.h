/*
   Copyright 2014 Technical University of Denmark, DTU Compute. 
   All rights reserved.
   
   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

/*
 * PTP1588 section of ethlib (ethernet library)
 * 
 * Authors: Eleftherios Kyriakakis (elky@dtu.dk)
 */

#ifndef _PTP_1588_H
#define _PTP_1588_H

#include <machine/exceptions.h>
#include <machine/spm.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "udp.h"

#define PTP_RXCHAN_TIMESTAMP_NS *((volatile _SPM unsigned int *) 0xF00DE000)
#define PTP_RXCHAN_TIMESTAMP_SEC *((volatile _SPM unsigned int *) 0xF00DE004)
#define PTP_RXCHAN_VALID *((volatile _SPM unsigned int *) 0xF00DE008) & 0x100
#define PTP_RXCHAN_MESSAGE_TYPE *((volatile _SPM unsigned int *) 0xF00DE008) & 0xFF
#define PTP_RXCHAN_INTERRUPT_FILTER *((volatile _SPM unsigned int *) 0xF00DE00C)

#define PTP_TXCHAN_TIMESTAMP_NS *((volatile _SPM unsigned int *) 0xF00DE400)
#define PTP_TXCHAN_TIMESTAMP_SEC *((volatile _SPM unsigned int *) 0xF00DE404)
#define PTP_TXCHAN_VALID *((volatile _SPM unsigned int *) 0xF00DE408) & 0x100
#define PTP_TXCHAN_MESSAGE_TYPE *((volatile _SPM unsigned int *) 0xF00DE408) & 0xFF
#define PTP_TXCHAN_INTERRUPT_FILTER *((volatile _SPM unsigned int *) 0xF00DE40C)

#define RTC_TIME_NS *((volatile _SPM unsigned int *) 0xF00DE800)
#define RTC_TIME_SEC *((volatile _SPM unsigned int *) 0xF00DE804)
#define RTC_PERIOD_LO *((volatile _SPM unsigned int *) 0xF00DE810)
#define RTC_PERIOD_HI *((volatile _SPM unsigned int *) 0xF00DE814)
#define RTC_STEP *((volatile _SPM unsigned int *) 0xF00DE820)

#define PTP_EVENT_PORT 319
#define PTP_GENERAL_PORT 320

#define PTP_SYNC_MSGTYPE 0x00
#define PTP_FOLLOW_MSGTYPE 0x08
#define PTP_DLYREQ_MSGTYPE 0x01
#define PTP_DLYRPLY_MSGTYPE 0x09

#define PTP_SYNC_CTRL 0x0
#define PTP_FOLLOW_CTRL 0x2
#define PTP_DLYREQ_CTRL 0x1
#define PTP_DLYRPLY_CTRL 0x3

#define FLAG_PTP_SECURITY_MASK(value) (value & 0x8000)
#define FLAG_PTP_PROF_SPEC_2_MASK(value) (value & 0x4000)
#define FLAG_PTP_PROF_SPEC_1_MASK(value) (value & 0x2000)
#define FLAG_PTP_UNICAST_MASK(value) (value & 0x0400)
#define FLAG_PTP_TWO_STEP_MASK 0x0200
#define FLAG_PTP_ALT_MASTER_MASK(value) (value & 0x0100)
#define FLAG_FREQ_TRACE_MASK(value) (value & 0x0020)
#define FLAG_TIME_TRACE_MASK(value) (value & 0x0010)
#define FLAG_PTP_TIMESCALE_MASK(value) (value & 0x0008)
#define FLAG_PTP_UTC_REASONABLE_MASK(value) (value & 0x0004)
#define FLAG_PTP_LI_59_MASK(value) (value & 0x0002)
#define FLAG_PTP_LI_61_MASK(value) (value & 0x0001)

#define PTP_MULTICAST_MAC (unsigned char[6]) {0x01,0x00,0x5E,0x00,0x01,0x81}
#define PTP_BROADCAST_MAC (unsigned char[6]) {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}
#define PTP_MULTICAST_IP (unsigned char[4]) {224,0,1,129}

// Time in us
#define PTP_SYNC_TIMEOUT 0
#define PTP_FOLLOW_DELAY 5000
#define PTP_REQ_TIMEOUT 100000
#define PTP_RPLY_TIMEOUT 1000000

#define NS_TO_SEC 0.000001

typedef struct {
	unsigned char transportSpec_msgType;
	unsigned char reserved_versionPTP;
	unsigned short messageLength;
	unsigned char domainNumber;
	unsigned char reserved1;
	unsigned short flagField;
	unsigned char correctionField[8];
	unsigned int reserved2;
	unsigned char sourcePortIdentity[10];
	unsigned short sequenceId;
	unsigned char controlField;
	unsigned char logMessageInterval;
} PTPv2MsgHeader;

typedef struct {
	unsigned int seconds;
	unsigned int nanoseconds;
	unsigned char portIdentity[8];
	unsigned short portId;
} PTPMsgBody;

typedef struct {
	PTPv2MsgHeader head;
	PTPMsgBody body;
} PTPv2Msg;

// typedef struct{
//   int seconds;
//   int nanoseconds;
// } PTPTime;

// typedef struct {
//   PTPTime t1PreciseSync;
//   PTPTime t2Sync;
//   PTPTime t3PreciseDelReq;
//   PTPTime t4DelReq;
//   PTPTime offset;
//   PTPTime delay;
// } PTPv2TimeRecord;

typedef struct {
  int offsetSeconds;
  int offsetNanoseconds;
  unsigned int t1Seconds;
  unsigned int t1Nanoseconds;
  unsigned int t2Seconds;
  unsigned int t2Nanoseconds;
  int delaySeconds;
  int delayNanoseconds;
  unsigned int t3Seconds;
  unsigned int t3Nanoseconds;
  unsigned int t4Seconds;
  unsigned int t4Nanoseconds;
} PTPv2TimeRecord;

typedef struct{
  unsigned char ip[4];
  unsigned char mac[6];
} PTPAddrInfo;

PTPv2Msg ptpMsg;
PTPv2TimeRecord ptpTimeRecord;
PTPAddrInfo lastMasterInfo;
PTPAddrInfo lastSlaveInfo;

///////////////////////////////////////////////////////////////
//Functions for PTP 1588 protocol
///////////////////////////////////////////////////////////////

//Interrupts to register timestamps as soon as they happen
void ptpv2_intr_rx_handler(void) __attribute__((naked));
void ptpv2_intr_tx_handler(void) __attribute__((naked));

//Serializes a PTPv2 message structure into buffer byte array
void ptpv2_serialize(PTPv2Msg msg, unsigned char buffer[]);

//Deserializes a buffer byte array to a PTPv2 message structure
PTPv2Msg ptpv2_deserialize(unsigned char buffer[]);

//Issues a PTPv2 Message
int ptpv2_issue_msg(unsigned tx_addr, unsigned rx_addr, unsigned char destination_mac[6], unsigned char destination_ip[4], unsigned seqId, unsigned msgType, unsigned ctrlField, unsigned short eventPort);

//Handles a PTPv2 Message
int ptpv2_handle_msg(unsigned tx_addr, unsigned rx_addr, unsigned char source_mac[6], unsigned char source_ip[4]);

//Calculates the offset from the master clock based on timestamps T1, T2
int ptp_calc_offset(unsigned int t1, unsigned int t2, int delay);

//Calculates the delay from the master clock based on timestamps T1, T2, T3, T4
int ptp_calc_one_way_delay(unsigned int t1, unsigned int t2, unsigned int t3, unsigned int t4);

///////////////////////////////////////////////////////////////
//Help Functions
///////////////////////////////////////////////////////////////

void print_bytes(unsigned char byte_buffer[], unsigned int len);

#endif