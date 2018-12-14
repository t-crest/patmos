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

//Hardware
#define PTP_CHAN_VALID_TS_MASK 0x100
#define PTP_CHAN_MSG_TYPE_MASK 0x0FF

#define PTP_RXCHAN_TIMESTAMP_NS *((volatile _SPM unsigned int *) 0xF00DE000)
#define PTP_RXCHAN_TIMESTAMP_SEC *((volatile _SPM unsigned int *) 0xF00DE004)
#define PTP_RXCHAN_STATUS *((volatile _SPM unsigned int *) 0xF00DE008)
#define PTP_RXCHAN_INTERRUPT_FILTER *((volatile _SPM unsigned int *) 0xF00DE00C)

#define PTP_TXCHAN_TIMESTAMP_NS *((volatile _SPM unsigned int *) 0xF00DE400)
#define PTP_TXCHAN_TIMESTAMP_SEC *((volatile _SPM unsigned int *) 0xF00DE404)
#define PTP_TXCHAN_STATUS *((volatile _SPM unsigned int *) 0xF00DE408)
#define PTP_TXCHAN_INTERRUPT_FILTER *((volatile _SPM unsigned int *) 0xF00DE40C)

#define RTC_TIME_NS *((volatile _SPM unsigned int *) 0xF00DE800)
#define RTC_TIME_SEC *((volatile _SPM unsigned int *) 0xF00DE804)
#define RTC_PERIOD_LO *((volatile _SPM unsigned int *) 0xF00DE810)
#define RTC_PERIOD_HI *((volatile _SPM unsigned int *) 0xF00DE814)
#define RTC_CORRECTION_OFFSET *((volatile _SPM int *) 0xF00DE820)

//Protocol types
#define PTP_EVENT_PORT 319
#define PTP_GENERAL_PORT 320

#define PTP_SYNC_MSGTYPE 0x00
#define PTP_FOLLOW_MSGTYPE 0x08
#define PTP_DLYREQ_MSGTYPE 0x01
#define PTP_DLYRPLY_MSGTYPE 0x09
#define PTP_ANNOUNCE_MSGTYPE 0xb

#define PTP_SYNC_CTRL 0x0
#define PTP_FOLLOW_CTRL 0x2
#define PTP_DLYREQ_CTRL 0x1
#define PTP_DLYRPLY_CTRL 0x3

#define FLAG_PTP_SECURITY_MASK(value) (value & 0x8000)
#define FLAG_PTP_PROF_SPEC_2_MASK(value) (value & 0x4000)
#define FLAG_PTP_PROF_SPEC_1_MASK(value) (value & 0x2000)
#define FLAG_PTP_UNICAST_MASK 0x400
#define FLAG_PTP_TWO_STEP_MASK 0x0200
#define FLAG_PTP_ALT_MASTER_MASK(value) (value & 0x0100)
#define FLAG_FREQ_TRACE_MASK(value) (value & 0x0020)
#define FLAG_TIME_TRACE_MASK(value) (value & 0x0010)
#define FLAG_PTP_TIMESCALE_MASK(value) (value & 0x0008)
#define FLAG_PTP_UTC_REASONABLE_MASK(value) (value & 0x0004)
#define FLAG_PTP_LI_59_MASK(value) (value & 0x0002)
#define FLAG_PTP_LI_61_MASK(value) (value & 0x0001)

//Addresses
#define PTP_MULTICAST_MAC (unsigned char[6]) {0x01,0x00,0x5E,0x00,0x01,0x81}
#define PTP_BROADCAST_MAC (unsigned char[6]) {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}
#define PTP_MULTICAST_IP (unsigned char[4]) {224,0,1,129}

//Time in us
#define PTP_SYNC_PERIOD 15625
#define PTP_SYNC_TIMEOUT 0
#define PTP_REQ_TIMEOUT 1000
#define PTP_RPLY_TIMEOUT 1000
#define NS_TO_SEC 0.000000001
#define NS_TO_USEC 0.001
#define USEC_TO_NS 1000
#define USEC_TO_SEC 0.000001
#define SEC_TO_NS 1000000000
#define SEC_TO_USEC 1000000
#define SEC_TO_HOUR 0.000277777778

//Thresholds
#define PTP_NS_OFFSET_THRESHOLD 500000*USEC_TO_NS
#define PTP_SEC_OFFSET_THRESHOLD 0

//Drift
#define WCET_COMPENSATION 787
#define DRIFT_RATE 9.82800f
#define PTP_DRIFT_AMOUNT(syncInterval) (int) (syncInterval*DRIFT_RATE/SEC_TO_USEC)*USEC_TO_NS

//Constants & Options
#define USE_HW_TIMESTAMP
#define PTP_RATE_CONTROL 1
#define PTP_CORRECTION_EN 1

static const unsigned SYNC_INTERVAL_OPTIONS[] = {1000000, 500000, 250000, 125000, 62500, 31250, 15625, 7812, 3906, 1935, 976};

typedef struct {
	unsigned char transportSpec_msgType;
	unsigned char reserved_versionPTP;
	unsigned short messageLength;
	unsigned char domainNumber;
	unsigned char reserved1;
	unsigned short flagField;
	unsigned long long correctionField;
	unsigned int reserved2;
	unsigned char portIdentity[8];
	unsigned short portId;
	unsigned short sequenceId;
	unsigned char controlField;
	unsigned char logMessageInterval;
} PTPv2MsgHeader;

typedef struct {
	unsigned int seconds;
	unsigned int nanoseconds;
  unsigned char requestingSourcePort[8];
  unsigned short requestingSourceId;
} PTPMsgBody;

typedef struct {
	PTPv2MsgHeader head;
	PTPMsgBody body;
} PTPv2Msg;

typedef struct {
  unsigned char syncInterval;
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
  unsigned char clockId[8];
  unsigned short id;
} PTPPortInfo;

PTPv2Msg txPTPMsg;
PTPv2Msg rxPTPMsg;
PTPv2TimeRecord ptpTimeRecord;
PTPPortInfo thisPortInfo;
PTPPortInfo lastMasterInfo;
PTPPortInfo lastSlaveInfo;

///////////////////////////////////////////////////////////////
//Functions for PTP 1588 protocol
///////////////////////////////////////////////////////////////

//Intialiaze PTP port
PTPPortInfo ptpv2_intialize_local_port(unsigned char mac[6], unsigned char ip[4], unsigned short portId);

//Issues a PTPv2 Message
int ptpv2_issue_msg(unsigned tx_addr, unsigned rx_addr, unsigned char destination_mac[6], unsigned char destination_ip[4], unsigned seqId, unsigned msgType, unsigned char syncInterval);

//Handles a PTPv2 Message
int ptpv2_handle_msg(unsigned tx_addr, unsigned rx_addr, unsigned char source_mac[6]);

//Applies the correction mechanism based on the calculated offset and acceptable threshold value
void ptp_correct_offset();

//Calculates the offset from the master clock based on timestamps T1, T2
int ptp_calc_offset(int t1, int t2, int delay);

//Calculates the delay from the master clock based on timestamps T1, T2, T3, T4
int ptp_calc_delay(int t1, int t2, int t3, int t4);

//Returns 1 if the source clock port identity matches the filter identity 
unsigned char ptp_filter_clockport(unsigned char sourceId[8], unsigned short sourcePortId, unsigned char matchId[8], unsigned short matchPortId);

///////////////////////////////////////////////////////////////
//Help Functions
///////////////////////////////////////////////////////////////

unsigned long long get_rtc_usecs();

unsigned int get_rtc_secs();

void print_bytes(unsigned char byte_buffer[], unsigned int len);

#endif
