/*
 * PTP1588 section of ethlib (ethernet library)
 * 
 * Authors: Eleftherios Kyriakakis (elky@dtu.dk)
 */

#ifndef _PTP_1588_H
#define _PTP_1588_H

#include <machine/patmos.h>
#include <machine/exceptions.h>
#include <machine/spm.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "udp.h"

//Hardware
#define PTP_CHAN_VALID_TS_MASK 0x100
#define PTP_CHAN_MSG_TYPE_MASK 0x0FF


#define PTP_RXCHAN_TIMESTAMP_NS(base)     *((volatile _SPM unsigned int *) (base+0xE000))
#define PTP_RXCHAN_TIMESTAMP_SEC(base)    *((volatile _SPM unsigned int *) (base+0xE004))
#define PTP_RXCHAN_STATUS(base)           *((volatile _SPM unsigned int *) (base+0xE008))
#define PTP_RXCHAN_DSTMACLO_FILTER(base)    *((volatile _SPM unsigned int *) (base+0xE00C))
#define PTP_RXCHAN_DSTMACHI_FILTER(base)    *((volatile _SPM unsigned int *) (base+0xE010))

#define PTP_TXCHAN_TIMESTAMP_NS(base)     *((volatile _SPM unsigned int *) (base+0xE400))
#define PTP_TXCHAN_TIMESTAMP_SEC(base)    *((volatile _SPM unsigned int *) (base+0xE404))
#define PTP_TXCHAN_STATUS(base)           *((volatile _SPM unsigned int *) (base+0xE408))
#define PTP_TXCHAN_DSTMACLO_FILTER(base)    *((volatile _SPM unsigned int *) (base+0xE40C))
#define PTP_TXCHAN_DSTMACHI_FILTER(base)    *((volatile _SPM unsigned int *) (base+0xE410))

#define RTC_TIME_NS(base)       *((volatile _SPM unsigned int *) (base+0xE800))
#define RTC_TIME_SEC(base)      *((volatile _SPM unsigned int *) (base+0xE804))
#define RTC_PERIOD_LO(base)     *((volatile _SPM unsigned int *) (base+0xE810))
#define RTC_PERIOD_HI(base)     *((volatile _SPM unsigned int *) (base+0xE814))
#define RTC_ADJUST_OFFSET(base) *((volatile _SPM signed int *)   (base+0xE820))

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
#define PTP_SYNC_PERIOD 3906
#define PTP_SYNC_TIMEOUT 1000000
#define PTP_REQ_TIMEOUT 1000
#define PTP_RPLY_TIMEOUT 1000
#define NS_TO_SEC 0.000000001
#define NS_TO_USEC 0.001
#define USEC_TO_NS 1000
#define USEC_TO_SEC 0.000001
#define MS_TO_NS 1000000
#define MS_TO_USEC 1000
#define MS_TO_SEC 0.001
#define SEC_TO_NS 1000000000
#define SEC_TO_USEC 1000000
#define SEC_TO_HOUR 0.000277777778

#define PTP_TIME_TO_USEC(seconds, nanos) (unsigned long long) ((unsigned long long)(SEC_TO_USEC * seconds) + (unsigned long long)(NS_TO_USEC * nanos))
#define PTP_TIME_TO_NS(seconds, nanos) (unsigned long long) ((unsigned long long)(SEC_TO_NS * (unsigned long long)seconds) + (unsigned long long)nanos)

//Thresholds
#define PTP_NS_OFFSET_THRESHOLD 500000*USEC_TO_NS
#define PTP_SEC_OFFSET_THRESHOLD 0

//Drift
#define WCET_COMPENSATION 3*USEC_TO_NS
#define DRIFT_RATE 10 //usec
#define PTP_DRIFT_AMOUNT(syncInterval) (int) (syncInterval*(DRIFT_RATE*USEC_TO_SEC))*USEC_TO_NS

//Constants & Options
#define PTP_DELAY_FOLLOWUP 31000 //cc
#define USE_HW_TIMESTAMP
#define PTP_RATE_CONTROL 1
#define PTP_CORRECTION_EN 1

static const unsigned SYNC_INTERVAL_OPTIONS[] = {1000000, 500000, 250000, 125000, 62500, 31250, 15625, 7812, 3906, 1935, 976};

enum ptp_role{PTP_MASTER, PTP_SLAVE};

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
} PTPv2Time;

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
  unsigned int eth_base;
  unsigned char ip[4];
  unsigned char mac[6];
  unsigned char clockId[8];
  unsigned short id;
  unsigned int portRole;
	char syncInterval;
} PTPPortInfo;

PTPv2Msg txPTPMsg;
PTPv2Msg rxPTPMsg;
PTPv2TimeRecord ptpTimeRecord;
PTPPortInfo thisPtpPortInfo;
PTPPortInfo lastMasterInfo;
PTPPortInfo lastSlaveInfo;

///////////////////////////////////////////////////////////////
//Functions for PTP 1588 protocol
///////////////////////////////////////////////////////////////

//Intialiaze PTP port
PTPPortInfo ptpv2_intialize_local_port(unsigned int eth_base, int portRole, unsigned char mac[6], unsigned char ip[4], unsigned short portId, int syncPeriod);

//Process a received PTPV2 message on a specific port
int ptpv2_process_received(PTPPortInfo ptpPortInfo, unsigned tx_addr, unsigned rx_addr);

//Issues a PTPv2 Message
int ptpv2_issue_msg(PTPPortInfo ptpPortInfo, unsigned tx_addr, unsigned rx_addr, unsigned char destination_mac[6], unsigned char destination_ip[4], unsigned seqId, unsigned msgType);

//Handles a PTPv2 Message
int ptpv2_handle_msg(PTPPortInfo ptpPortInfo, unsigned tx_addr, unsigned rx_addr, unsigned char source_mac[6]);

//Applies the correction mechanism based on the calculated offset and acceptable threshold value
void ptp_correct_offset(PTPPortInfo ptpPortInfo);

//Calculates the offset from the master clock based on timestamps T1, T2
int ptp_calc_offset(int t1, int t2, int delay);

//Calculates the delay from the master clock based on timestamps T1, T2, T3, T4
int ptp_calc_delay(int t1, int t2, int t3, int t4);

//Returns 1 if the source clock port identity matches the filter identity 
unsigned char ptp_filter_clockport(unsigned char sourceId[8], unsigned short sourcePortId, unsigned char matchId[8], unsigned short matchPortId);

///////////////////////////////////////////////////////////////
//Help Functions
///////////////////////////////////////////////////////////////

unsigned long long get_ptp_nanos(unsigned int eth_base);

unsigned long long get_ptp_usecs(unsigned int eth_base);

unsigned int get_ptp_secs(unsigned int eth_base);

void print_bytes(unsigned char byte_buffer[], unsigned int len);

#endif
