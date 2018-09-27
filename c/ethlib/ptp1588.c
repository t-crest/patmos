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

void ptpv2_intr_tx_handler(void) {
	exc_prologue();
	unsigned char msgType = (unsigned char) (PTP_TXCHAN_STATUS & PTP_CHAN_MSG_TYPE_MASK);
	printf("ptpv2_tx_intr_h(%x02X)\n", msgType);
	if(msgType==PTP_SYNC_MSGTYPE){
		//Master
		#ifdef USE_HW_TIMESTAMP
		ptpTimeRecord.t1Nanoseconds = (unsigned) PTP_TXCHAN_TIMESTAMP_NS;
		ptpTimeRecord.t1Seconds = (unsigned) PTP_TXCHAN_TIMESTAMP_SEC;
		#else
		ptpTimeRecord.t1Nanoseconds = (unsigned) RTC_TIME_NS;
		ptpTimeRecord.t1Seconds = (unsigned) RTC_TIME_SEC;
		#endif
	} else if(msgType==PTP_DLYREQ_MSGTYPE){
		//Slave
		#ifdef USE_HW_TIMESTAMP
		ptpTimeRecord.t4Nanoseconds = (unsigned) PTP_TXCHAN_TIMESTAMP_NS;
		ptpTimeRecord.t4Seconds = (unsigned) PTP_TXCHAN_TIMESTAMP_SEC;
		#else
		ptpTimeRecord.t4Nanoseconds = (unsigned) RTC_TIME_NS;
		ptpTimeRecord.t4Seconds = (unsigned) RTC_TIME_SEC;
		#endif
	}
	exc_epilogue();
}

__attribute__((noinline))
int ptpv2_serialize(PTPv2Msg msg, unsigned char buffer[]){
	//Head
	memcpy(buffer, &msg.head.transportSpec_msgType, 1*sizeof(char));
	memcpy(buffer+1, &msg.head.reserved_versionPTP, 1*sizeof(char));
	memcpy(buffer+2, &msg.head.messageLength, 2*sizeof(char));
	memcpy(buffer+4, &msg.head.domainNumber, 1*sizeof(char));
	memcpy(buffer+5, &msg.head.reserved1, 1*sizeof(char));
	memcpy(buffer+6, &msg.head.flagField, 2*sizeof(char));
	memcpy(buffer+8, &msg.head.correctionField, 8*sizeof(char));
	memcpy(buffer+16, &msg.head.reserved2, 4*sizeof(char));
	memcpy(buffer+20, &msg.head.sourcePortIdentity, 10*sizeof(char));
	memcpy(buffer+30, &msg.head.sequenceId, 2*sizeof(char));
	memcpy(buffer+32, &msg.head.controlField, 1*sizeof(char));
	memcpy(buffer+33, &msg.head.logMessageInterval, 1*sizeof(char));
	//Body
	memcpy(buffer+36, &msg.body.seconds, 4*sizeof(char));
	memcpy(buffer+40, &msg.body.nanoseconds, 4*sizeof(char));
    if((msg.head.transportSpec_msgType & 0x0F)==PTP_DLYRPLY_MSGTYPE){
	    memcpy(buffer+44, &msg.body.portIdentity, 8*sizeof(char));
	    memcpy(buffer+52, &msg.body.portId, 2*sizeof(char));
    }
	// print_bytes(buffer, 54);
	return 1;
}

__attribute__((noinline))
PTPv2Msg ptpv2_deserialize(unsigned char buffer[]){
    PTPv2Msg msg;
    //Head
	memcpy(&msg.head.transportSpec_msgType, buffer, 1*sizeof(char));
	memcpy(&msg.head.reserved_versionPTP, buffer+1, 1*sizeof(char));
	memcpy(&msg.head.messageLength, buffer+2,  2*sizeof(char));
	memcpy(&msg.head.domainNumber, buffer+4, 1*sizeof(char));
	memcpy(&msg.head.reserved1, buffer+5, 1*sizeof(char));
	memcpy(&msg.head.flagField, buffer+6, 2*sizeof(char));
	memcpy(&msg.head.correctionField, buffer+8, 8*sizeof(char));
	memcpy(&msg.head.reserved2, buffer+16, 4*sizeof(char));
	memcpy(&msg.head.sourcePortIdentity, buffer+20, 10*sizeof(char));
	memcpy(&msg.head.sequenceId, buffer+30, 2*sizeof(char));
	memcpy(&msg.head.controlField, buffer+32, 1*sizeof(char));
	memcpy(&msg.head.logMessageInterval, buffer+33, 1*sizeof(char));
	//Body
	memcpy(&msg.body.seconds, buffer+36, 4*sizeof(char));
	memcpy(&msg.body.nanoseconds, buffer+40, 4*sizeof(char));
    if((msg.head.transportSpec_msgType & 0x0F)==PTP_DLYRPLY_MSGTYPE){
	    memcpy(&msg.body.portIdentity, buffer+44, 8*sizeof(char));
	    memcpy(&msg.body.portId, buffer+52, 2*sizeof(char));
    }
	// print_bytes(buffer, 54);
    return msg;
}

__attribute__((noinline))
int ptpv2_issue_msg(unsigned tx_addr, unsigned rx_addr, unsigned char destination_mac[6], unsigned char destination_ip[4], unsigned seqId, unsigned msgType, unsigned ctrlField, unsigned short eventPort) {
	unsigned short msgLen = msgType==PTP_DLYRPLY_MSGTYPE ? 54 : 44;
	unsigned int timestampNanoseconds = 0;
	unsigned int timestampSeconds = 0;
	PTPv2Msg msg;
	msg.head.transportSpec_msgType = msgType;
	msg.head.reserved_versionPTP = 0x02;
	msg.head.messageLength = msgLen;
	msg.head.sequenceId = seqId;
	msg.head.controlField = ctrlField;
	msg.head.flagField |= FLAG_PTP_TWO_STEP_MASK;
	switch(msgType){
		case PTP_SYNC_MSGTYPE:
			//Master
			msg.body.nanoseconds = (unsigned)RTC_TIME_NS;
			msg.body.seconds = (unsigned)RTC_TIME_SEC;
			break;
		case PTP_FOLLOW_MSGTYPE:
			//Master
			msg.body.nanoseconds = ptpTimeRecord.t1Nanoseconds;
			msg.body.seconds = ptpTimeRecord.t1Seconds;
			break;
		case PTP_DLYREQ_MSGTYPE:
			//Slave
			msg.body.nanoseconds = (unsigned)RTC_TIME_NS;
			msg.body.seconds = (unsigned)RTC_TIME_SEC;
			break;
		case PTP_DLYRPLY_MSGTYPE:
			//Master
			msg.body.nanoseconds = ptpTimeRecord.t4Nanoseconds;
			msg.body.seconds = ptpTimeRecord.t4Seconds;
			break; 
	}
	udp_send_mac(tx_addr, rx_addr, destination_mac, destination_ip, eventPort, eventPort, (unsigned char*)&msg, msgLen, 2000000);
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
int ptpv2_handle_msg(unsigned tx_addr, unsigned rx_addr, unsigned char source_mac[6], unsigned char source_ip[4]){
	unsigned char udp_data[54] = {0};
	signed char ans = 0;
	#ifdef USE_HW_TIMESTAMP
	_Pragma("loopbound min 0 max 1")	
	while((PTP_RXCHAN_STATUS & PTP_CHAN_VALID_TS_MASK) != PTP_CHAN_VALID_TS_MASK){continue;}
	unsigned int timestampNanoseconds = (unsigned) PTP_RXCHAN_TIMESTAMP_NS;
	unsigned int timestampSeconds =  (unsigned) PTP_RXCHAN_TIMESTAMP_SEC;
	#else
	unsigned int timestampNanoseconds = (unsigned) RTC_TIME_NS;
	unsigned int timestampSeconds =  (unsigned) RTC_TIME_SEC;
	#endif
	udp_get_data(rx_addr, (unsigned char*) &ptpMsg, udp_get_data_length(rx_addr));
	switch(ptpMsg.head.transportSpec_msgType){
	case PTP_SYNC_MSGTYPE:
		//Exec by slave port
		ptpTimeRecord.t2Nanoseconds = timestampNanoseconds;
		ptpTimeRecord.t2Seconds = timestampSeconds;
		if((ptpMsg.head.flagField & FLAG_PTP_TWO_STEP_MASK) != FLAG_PTP_TWO_STEP_MASK){
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
			ptpTimeRecord.t1Nanoseconds = ptpMsg.body.nanoseconds;
			ptpTimeRecord.t1Seconds = ptpMsg.body.seconds;
		}
		ans = PTP_SYNC_MSGTYPE;
		break;
	case PTP_FOLLOW_MSGTYPE:
		//Exec by slave port
		ptpTimeRecord.t1Nanoseconds = ptpMsg.body.nanoseconds;
		ptpTimeRecord.t1Seconds = ptpMsg.body.seconds;
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
		ptpTimeRecord.t4Nanoseconds = ptpMsg.body.nanoseconds;
		ptpTimeRecord.t4Seconds = ptpMsg.body.seconds;
		ptpTimeRecord.delayNanoseconds = ptp_calc_one_way_delay(ptpTimeRecord.t1Nanoseconds, ptpTimeRecord.t2Nanoseconds, ptpTimeRecord.t3Nanoseconds, ptpTimeRecord.t4Nanoseconds);
		ptpTimeRecord.offsetNanoseconds = ptp_calc_offset(ptpTimeRecord.t1Nanoseconds, ptpTimeRecord.t2Nanoseconds, ptpTimeRecord.delayNanoseconds);
		ptpTimeRecord.delaySeconds = ptp_calc_one_way_delay(ptpTimeRecord.t1Seconds, ptpTimeRecord.t2Seconds, ptpTimeRecord.t3Seconds, ptpTimeRecord.t4Seconds);
		ptpTimeRecord.offsetSeconds = ptp_calc_offset(ptpTimeRecord.t1Seconds, ptpTimeRecord.t2Seconds, ptpTimeRecord.delaySeconds);
		ptp_correct_offset();
		ans = PTP_DLYRPLY_MSGTYPE;
		break;
	default:
		return -2;
		break;
	}
	return ans;
}

//Applies the correction mechanism based on the calculated offset and acceptable threshold value
__attribute__((noinline))
void ptp_correct_offset(){
	//Calculation
	if(PTP_RATE_CONTROL==0){
		RTC_TIME_NS = (unsigned) ((int)RTC_TIME_NS - ptpTimeRecord.offsetNanoseconds);
	} else {
		RTC_CORRECTION_OFFSET = ptpTimeRecord.offsetNanoseconds;
	}
	if(ptpTimeRecord.offsetSeconds != 0){
		RTC_TIME_SEC = (unsigned) ((int)RTC_TIME_SEC - ptpTimeRecord.offsetSeconds);
	}
}

//Calculates the offset from the master clock based on timestamps T1, T2
__attribute__((noinline))
int ptp_calc_offset(int t1, int t2, int delay){
	return t2-t1-delay;
}

//Calculates the delay from the master clock based on timestamps T1, T2, T3, T4
__attribute__((noinline))
int ptp_calc_one_way_delay(int t1, int t2, int t3, int t4){
	return (t2-t1+t4-t3)/2;
}

///////////////////////////////////////////////////////////////
//Help Functions
///////////////////////////////////////////////////////////////

unsigned int get_rtc_usecs(){
	return (unsigned int) (NS_TO_USEC * RTC_TIME_NS);
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
