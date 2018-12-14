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

#include "ptp1588.h"

PTPPortInfo ptpv2_intialize_local_port(unsigned char mac[6], unsigned char ip[4], unsigned short portId){
	PTPPortInfo newPort;
	*(newPort.ip) = *(ip);
	*(newPort.mac) = *(mac);
	newPort.clockId[0] = mac[0];
	newPort.clockId[1] = mac[1];
	newPort.clockId[2] = mac[2];
	newPort.clockId[3] = 0xff;
	newPort.clockId[4] = 0xfe;
	newPort.clockId[5] = mac[3];
	newPort.clockId[6] = mac[4];
	newPort.clockId[7] = mac[5];
	newPort.id = portId;
	return newPort;
}

__attribute__((noinline))
int ptpv2_issue_msg(unsigned tx_addr, unsigned rx_addr, unsigned char destination_mac[6], unsigned char destination_ip[4], unsigned seqId, unsigned msgType, unsigned char syncInterval) {
	txPTPMsg.head.sequenceId = seqId;
	txPTPMsg.head.transportSpec_msgType = msgType;
	txPTPMsg.head.reserved_versionPTP = 0x02;
	txPTPMsg.head.flagField |= FLAG_PTP_TWO_STEP_MASK;
	txPTPMsg.head.flagField |= FLAG_PTP_UNICAST_MASK;
	txPTPMsg.head.correctionField = 0x0LL;
	txPTPMsg.head.portIdentity[0] = thisPortInfo.clockId[0];
	txPTPMsg.head.portIdentity[1] = thisPortInfo.clockId[1];
	txPTPMsg.head.portIdentity[2] = thisPortInfo.clockId[2];
	txPTPMsg.head.portIdentity[3] = thisPortInfo.clockId[3];
	txPTPMsg.head.portIdentity[4] = thisPortInfo.clockId[4];
	txPTPMsg.head.portIdentity[5] = thisPortInfo.clockId[5];
	txPTPMsg.head.portIdentity[6] = thisPortInfo.clockId[6];
	txPTPMsg.head.portIdentity[7] = thisPortInfo.clockId[7];
	txPTPMsg.head.portId = thisPortInfo.id;
	txPTPMsg.head.logMessageInterval = syncInterval;
	switch(msgType){
		case PTP_SYNC_MSGTYPE:
			//Master
			txPTPMsg.head.messageLength = 44;
			txPTPMsg.head.controlField = PTP_SYNC_CTRL;
			txPTPMsg.body.seconds = (unsigned)RTC_TIME_SEC & 0xFFFFFFFF;
			txPTPMsg.body.nanoseconds = (unsigned)RTC_TIME_NS;
			udp_send_mac(tx_addr, rx_addr, destination_mac, destination_ip, PTP_EVENT_PORT, PTP_EVENT_PORT, (unsigned char*)&txPTPMsg, 44, 0);
			break;
		case PTP_FOLLOW_MSGTYPE:
			//Master
			txPTPMsg.head.messageLength = 44;
			txPTPMsg.head.controlField = PTP_FOLLOW_CTRL;
			txPTPMsg.body.seconds = ptpTimeRecord.t1Seconds;
			txPTPMsg.body.nanoseconds = ptpTimeRecord.t1Nanoseconds;
			udp_send_mac(tx_addr, rx_addr, destination_mac, destination_ip, PTP_GENERAL_PORT, PTP_GENERAL_PORT, (unsigned char*)&txPTPMsg, 44, 0);
			break;
		case PTP_DLYREQ_MSGTYPE:
			//Slave
			txPTPMsg.head.messageLength = 44;
			txPTPMsg.head.controlField = PTP_DLYREQ_CTRL;
			txPTPMsg.body.seconds = (unsigned)RTC_TIME_SEC & 0xFFFFFFFF;
			txPTPMsg.body.nanoseconds = (unsigned)RTC_TIME_NS;
			udp_send_mac(tx_addr, rx_addr, destination_mac, destination_ip, PTP_EVENT_PORT, PTP_EVENT_PORT, (unsigned char*)&txPTPMsg, 44, 0);
			break;
		case PTP_DLYRPLY_MSGTYPE:
			//Master
			txPTPMsg.head.messageLength = 54;
			txPTPMsg.head.controlField = PTP_DLYRPLY_CTRL;
			*((unsigned long long*)txPTPMsg.body.requestingSourcePort) = *((unsigned long long*)rxPTPMsg.head.portIdentity);
			txPTPMsg.body.requestingSourceId = rxPTPMsg.head.portId;
			txPTPMsg.body.seconds = ptpTimeRecord.t4Seconds;
			txPTPMsg.body.nanoseconds = ptpTimeRecord.t4Nanoseconds;
			udp_send_mac(tx_addr, rx_addr, destination_mac, destination_ip, PTP_GENERAL_PORT, PTP_GENERAL_PORT, (unsigned char*)&txPTPMsg, 54, 0);
			break; 
	}
	if(msgType==PTP_SYNC_MSGTYPE){
		//Master
		#ifdef USE_HW_TIMESTAMP
		_Pragma("loopbound min 0 max 1")	
		while((PTP_TXCHAN_STATUS & PTP_CHAN_VALID_TS_MASK) != PTP_CHAN_VALID_TS_MASK){continue;}
		ptpTimeRecord.t1Nanoseconds = (unsigned) PTP_TXCHAN_TIMESTAMP_NS;
		ptpTimeRecord.t1Seconds = (unsigned) PTP_TXCHAN_TIMESTAMP_SEC;
		#else
		ptpTimeRecord.t1Nanoseconds = (unsigned) RTC_TIME_NS;
		ptpTimeRecord.t1Seconds = (unsigned) RTC_TIME_SEC;
		#endif
	} else if(msgType==PTP_DLYREQ_MSGTYPE){
		//Slave
		#ifdef USE_HW_TIMESTAMP
		_Pragma("loopbound min 0 max 1")	
		while((PTP_TXCHAN_STATUS & PTP_CHAN_VALID_TS_MASK) != PTP_CHAN_VALID_TS_MASK){continue;}
		ptpTimeRecord.t3Nanoseconds = (unsigned) PTP_TXCHAN_TIMESTAMP_NS;
		ptpTimeRecord.t3Seconds = (unsigned) PTP_TXCHAN_TIMESTAMP_SEC;
		#else
		ptpTimeRecord.t3Nanoseconds = (unsigned) RTC_TIME_NS;
		ptpTimeRecord.t3Seconds = (unsigned) RTC_TIME_SEC;
		#endif
	}
	return 1;
}

__attribute__((noinline))
int ptpv2_handle_msg(unsigned tx_addr, unsigned rx_addr, unsigned char source_mac[6]){
	signed char ans = -2;
	unsigned char source_ip[4];
	#ifdef USE_HW_TIMESTAMP
	_Pragma("loopbound min 0 max 1")	
	while((PTP_RXCHAN_STATUS & PTP_CHAN_VALID_TS_MASK) != PTP_CHAN_VALID_TS_MASK){continue;}
	unsigned int timestampNanoseconds = (unsigned) PTP_RXCHAN_TIMESTAMP_NS;
	unsigned int timestampSeconds =  (unsigned) PTP_RXCHAN_TIMESTAMP_SEC;
	#else
	unsigned int timestampNanoseconds = (unsigned) RTC_TIME_NS;
	unsigned int timestampSeconds =  (unsigned) RTC_TIME_SEC;
	#endif
	ipv4_get_source_ip(rx_addr, source_ip);
	udp_get_data(rx_addr, (unsigned char*) &rxPTPMsg, udp_get_data_length(rx_addr));
	switch(rxPTPMsg.head.transportSpec_msgType){
	case PTP_SYNC_MSGTYPE:
		//Exec by slave port
		// ptpTimeRecord.syncInterval = (unsigned long long) (SEC_TO_NS * (timestampSeconds-ptpTimeRecord.t2Seconds)) + (timestampNanoseconds - ptpTimeRecord.t2Nanoseconds);
		ptpTimeRecord.t2Nanoseconds = timestampNanoseconds;
		ptpTimeRecord.t2Seconds = timestampSeconds;
		ptpTimeRecord.syncInterval = rxPTPMsg.head.logMessageInterval;
		if((rxPTPMsg.head.flagField & FLAG_PTP_TWO_STEP_MASK) != FLAG_PTP_TWO_STEP_MASK){
			// If single-step then store values as precise timestamp and issue DLYREQ
			lastMasterInfo.ip[0] = source_ip[0];
			lastMasterInfo.ip[1] = source_ip[1];
			lastMasterInfo.ip[2] = source_ip[2];
			lastMasterInfo.ip[3] = source_ip[3];
			lastMasterInfo.mac[0] = source_mac[0];
			lastMasterInfo.mac[1] = source_mac[1];
			lastMasterInfo.mac[2] = source_mac[2];
			lastMasterInfo.mac[3] = source_mac[3];
			lastMasterInfo.mac[4] = source_mac[4];
			lastMasterInfo.mac[5] = source_mac[5];
			ptpTimeRecord.t1Nanoseconds = rxPTPMsg.body.nanoseconds;
			ptpTimeRecord.t1Seconds = rxPTPMsg.body.seconds;
		}
		// printf("%04X %04X\n", rxPTPMsg.body.seconds, rxPTPMsg.body.nanoseconds);
		ans = PTP_SYNC_MSGTYPE;
		break;
	case PTP_FOLLOW_MSGTYPE:
		//Exec by slave port
		ptpTimeRecord.t1Nanoseconds = rxPTPMsg.body.nanoseconds;
		ptpTimeRecord.t1Seconds = rxPTPMsg.body.seconds;
		lastMasterInfo.ip[0] = source_ip[0];
		lastMasterInfo.ip[1] = source_ip[1];
		lastMasterInfo.ip[2] = source_ip[2];
		lastMasterInfo.ip[3] = source_ip[3];
		lastMasterInfo.mac[0] = source_mac[0];
		lastMasterInfo.mac[1] = source_mac[1];
		lastMasterInfo.mac[2] = source_mac[2];
		lastMasterInfo.mac[3] = source_mac[3];
		lastMasterInfo.mac[4] = source_mac[4];
		lastMasterInfo.mac[5] = source_mac[5];
		ans = PTP_FOLLOW_MSGTYPE;
		break;
	case PTP_DLYREQ_MSGTYPE:
		//Exec by master port
		ptpTimeRecord.t4Nanoseconds = timestampNanoseconds;
		ptpTimeRecord.t4Seconds = timestampSeconds;
		lastSlaveInfo.ip[0] = source_ip[0];
		lastSlaveInfo.ip[1] = source_ip[1];
		lastSlaveInfo.ip[2] = source_ip[2];
		lastSlaveInfo.ip[3] = source_ip[3];
		lastSlaveInfo.mac[0] = source_mac[0];
		lastSlaveInfo.mac[1] = source_mac[1];
		lastSlaveInfo.mac[2] = source_mac[2];
		lastSlaveInfo.mac[3] = source_mac[3];
		lastSlaveInfo.mac[4] = source_mac[4];
		lastSlaveInfo.mac[5] = source_mac[5];
		ans = PTP_DLYREQ_MSGTYPE;
		break;
	case PTP_DLYRPLY_MSGTYPE:
		// Exec by slave port
		if(ptp_filter_clockport(rxPTPMsg.body.requestingSourcePort, rxPTPMsg.body.requestingSourceId, thisPortInfo.clockId, thisPortInfo.id)){
			ptpTimeRecord.t4Nanoseconds = rxPTPMsg.body.nanoseconds;
			ptpTimeRecord.t4Seconds = rxPTPMsg.body.seconds;
			ptpTimeRecord.delayNanoseconds = ptp_calc_delay(ptpTimeRecord.t1Nanoseconds, ptpTimeRecord.t2Nanoseconds, ptpTimeRecord.t3Nanoseconds, ptpTimeRecord.t4Nanoseconds);
			ptpTimeRecord.offsetNanoseconds = ptp_calc_offset(ptpTimeRecord.t1Nanoseconds, ptpTimeRecord.t2Nanoseconds, ptpTimeRecord.delayNanoseconds);
			ptpTimeRecord.delaySeconds = ptp_calc_delay(ptpTimeRecord.t1Seconds, ptpTimeRecord.t2Seconds, ptpTimeRecord.t3Seconds, ptpTimeRecord.t4Seconds);
			ptpTimeRecord.offsetSeconds = ptp_calc_offset(ptpTimeRecord.t1Seconds, ptpTimeRecord.t2Seconds, ptpTimeRecord.delaySeconds);
			if (PTP_CORRECTION_EN == 1) ptp_correct_offset();
			ans = PTP_DLYRPLY_MSGTYPE;
		}
		break;
	default:
		return -PTP;
		break;
	}
	return ans;
}

//Applies the correction mechanism based on the calculated offset and acceptable threshold value
__attribute__((noinline))
void ptp_correct_offset(){
	if(ptpTimeRecord.offsetSeconds != 0){
		RTC_TIME_SEC = (unsigned) (-ptpTimeRecord.offsetSeconds + (int)RTC_TIME_SEC);	//reverse order to load time operand last
		RTC_TIME_NS = (unsigned) (-(ptpTimeRecord.offsetNanoseconds) + WCET_COMPENSATION + (int)RTC_TIME_NS);	//reverse order to load time operand last
	} else {
		if(PTP_RATE_CONTROL==0 || abs(ptpTimeRecord.offsetNanoseconds) > PTP_NS_OFFSET_THRESHOLD){
			RTC_TIME_NS = (unsigned) (-(ptpTimeRecord.offsetNanoseconds) + WCET_COMPENSATION + (int)RTC_TIME_NS);	//reverse order to load time operand last
		} else {
			float driftCompens = 0.0002455f * SYNC_INTERVAL_OPTIONS[-((signed char)ptpTimeRecord.syncInterval)] * USEC_TO_NS / 25;
			RTC_CORRECTION_OFFSET = (int) (ptpTimeRecord.offsetNanoseconds - driftCompens);
		}
	}
}


//Calculates the offset from the master clock based on timestamps T1, T2
int ptp_calc_offset(int t1, int t2, int delay){
	return t2-t1-delay;
}

//Calculates the delay from the master clock based on timestamps T1, T2, T3, T4
int ptp_calc_delay(int t1, int t2, int t3, int t4){
	return (t2-t1+t4-t3)/2;
}

///////////////////////////////////////////////////////////////
//Help Functions
///////////////////////////////////////////////////////////////

unsigned char ptp_filter_clockport(unsigned char sourceId[8], unsigned short sourcePortId, unsigned char matchId[8], unsigned short matchPortId){
	unsigned char ans = sourceId[0] == matchId[0];
	ans &= sourceId[1] == matchId[1];
	ans &= sourceId[2] == matchId[2];
	ans &= sourceId[3] == matchId[3];
	ans &= sourceId[4] == matchId[4];
	ans &= sourceId[5] == matchId[5];
	ans &= sourceId[6] == matchId[6];
	ans &= sourceId[7] == matchId[7];
	ans &= sourcePortId == matchPortId;
	return ans;
}

unsigned long long get_rtc_usecs(){
	return (unsigned long long) (SEC_TO_USEC * RTC_TIME_SEC) + (NS_TO_USEC * (RTC_TIME_NS));
}

unsigned int get_rtc_secs(){
	return (unsigned int) (RTC_TIME_SEC);
}

void print_bytes(unsigned char byte_buffer[], unsigned int len){
	int i=0;
	while (i < len)
	{
		if(i > 0 && i % 16 == 0){
			printf("%02X\n",(unsigned)byte_buffer[i]);
		}else{
			printf("%02X ",(unsigned)byte_buffer[i]);
		}
	 	i++;
	}
	puts("\n");
}
