/*
 * PTP1588 section of ethlib (ethernet library)
 * 
 * Authors: Eleftherios Kyriakakis (elky@dtu.dk)
 */

#include "ptp1588.h"

PTPPortInfo ptpv2_intialize_local_port(unsigned int eth_base, int portRole, unsigned char mac[6], unsigned char ip[4], unsigned short portId, int syncPeriod){
	PTPPortInfo newPort;
	newPort.eth_base = eth_base;
	newPort.ip[0] = ip[0];
	newPort.ip[1] = ip[1];
	newPort.ip[2] = ip[2];
	newPort.ip[3] = ip[3];
	newPort.mac[0] = mac[0];
	newPort.mac[1] = mac[1];
	newPort.mac[2] = mac[2];
	newPort.mac[3] = mac[3];
	newPort.mac[4] = mac[4];
	newPort.mac[5] = mac[5];
	newPort.clockId[0] = mac[0];
	newPort.clockId[1] = mac[1];
	newPort.clockId[2] = mac[2];
	newPort.clockId[3] = 0xff;
	newPort.clockId[4] = 0xfe;
	newPort.clockId[5] = mac[3];
	newPort.clockId[6] = mac[4];
	newPort.clockId[7] = mac[5];
	newPort.id = portId;
	newPort.portRole = portRole;
	newPort.syncInterval = syncPeriod;
	return newPort;
}

int ptpv2_process_received(PTPPortInfo ptpPortInfo, unsigned tx_addr, unsigned rx_addr){
	short msgType = -1;
	switch(ptpPortInfo.portRole){
		case PTP_MASTER:
			if((msgType = ptpv2_handle_msg(thisPtpPortInfo, tx_addr, rx_addr, PTP_BROADCAST_MAC)) == PTP_DLYREQ_MSGTYPE){
				ptpv2_issue_msg(thisPtpPortInfo, tx_addr, rx_addr, PTP_BROADCAST_MAC, lastSlaveInfo.ip, rxPTPMsg.head.sequenceId, PTP_DLYRPLY_MSGTYPE);
			}
		break;
		case PTP_SLAVE:
			switch(msgType = ptpv2_handle_msg(thisPtpPortInfo, tx_addr, rx_addr, PTP_BROADCAST_MAC)){
			case PTP_SYNC_MSGTYPE:
				if((rxPTPMsg.head.flagField & FLAG_PTP_TWO_STEP_MASK) != FLAG_PTP_TWO_STEP_MASK){
					ptpv2_issue_msg(thisPtpPortInfo, tx_addr, rx_addr, PTP_BROADCAST_MAC, lastMasterInfo.ip, rxPTPMsg.head.sequenceId, PTP_DLYREQ_MSGTYPE);
				}
				break;
			case PTP_FOLLOW_MSGTYPE:
				ptpv2_issue_msg(thisPtpPortInfo, tx_addr, rx_addr, PTP_BROADCAST_MAC, lastMasterInfo.ip, rxPTPMsg.head.sequenceId, PTP_DLYREQ_MSGTYPE);
				break;
			case PTP_DLYRPLY_MSGTYPE:
				break;
			case PTP_ANNOUNCE_MSGTYPE:
				break;
			default:
				break;
			}
		break;
	}
	return msgType;
}

__attribute__((noinline))
int ptpv2_issue_msg(PTPPortInfo ptpPortInfo, unsigned tx_addr, unsigned rx_addr, unsigned char destination_mac[6], unsigned char destination_ip[4], unsigned seqId, unsigned msgType) {
	// printf("ptpv2_issue_msg #%d :", seqId);
	txPTPMsg.head.sequenceId = seqId;
	txPTPMsg.head.transportSpec_msgType = msgType;
	txPTPMsg.head.reserved_versionPTP = 0x02;
	txPTPMsg.head.flagField |= FLAG_PTP_TWO_STEP_MASK;
	txPTPMsg.head.flagField |= FLAG_PTP_UNICAST_MASK;
	txPTPMsg.head.correctionField = 0x0LL;
	txPTPMsg.head.portIdentity[0] = ptpPortInfo.clockId[0];
	txPTPMsg.head.portIdentity[1] = ptpPortInfo.clockId[1];
	txPTPMsg.head.portIdentity[2] = ptpPortInfo.clockId[2];
	txPTPMsg.head.portIdentity[3] = ptpPortInfo.clockId[3];
	txPTPMsg.head.portIdentity[4] = ptpPortInfo.clockId[4];
	txPTPMsg.head.portIdentity[5] = ptpPortInfo.clockId[5];
	txPTPMsg.head.portIdentity[6] = ptpPortInfo.clockId[6];
	txPTPMsg.head.portIdentity[7] = ptpPortInfo.clockId[7];
	txPTPMsg.head.portId = ptpPortInfo.id;
	txPTPMsg.head.logMessageInterval = (unsigned char) ptpPortInfo.syncInterval;
	switch(msgType){
		case PTP_SYNC_MSGTYPE:
			// puts("\tPTP_SYNC");
			//Master
			txPTPMsg.head.messageLength = 44;
			txPTPMsg.head.controlField = PTP_SYNC_CTRL;
			txPTPMsg.body.seconds = (unsigned)RTC_TIME_SEC(ptpPortInfo.eth_base) & 0xFFFFFFFF;
			txPTPMsg.body.nanoseconds = (unsigned)RTC_TIME_NS(ptpPortInfo.eth_base);
			udp_send_mac(tx_addr, rx_addr, destination_mac, destination_ip, PTP_EVENT_PORT, PTP_EVENT_PORT, (unsigned char*)&txPTPMsg, 44, 0);
			break;
		case PTP_FOLLOW_MSGTYPE:
			// puts("\tPTP_FOLLOW");
			//Master
			txPTPMsg.head.messageLength = 44;
			txPTPMsg.head.controlField = PTP_FOLLOW_CTRL;
			txPTPMsg.body.seconds = ptpTimeRecord.t1Seconds;
			txPTPMsg.body.nanoseconds = ptpTimeRecord.t1Nanoseconds;
			udp_send_mac(tx_addr, rx_addr, destination_mac, destination_ip, PTP_GENERAL_PORT, PTP_GENERAL_PORT, (unsigned char*)&txPTPMsg, 44, 0);
			break;
		case PTP_DLYREQ_MSGTYPE:
			// puts("\tPTP_DLYREQ");
			//Slave
			txPTPMsg.head.messageLength = 44;
			txPTPMsg.head.controlField = PTP_DLYREQ_CTRL;
			txPTPMsg.body.seconds = (unsigned)RTC_TIME_SEC(ptpPortInfo.eth_base) & 0xFFFFFFFF;
			txPTPMsg.body.nanoseconds = (unsigned)RTC_TIME_NS(ptpPortInfo.eth_base);
			udp_send_mac(tx_addr, rx_addr, destination_mac, destination_ip, PTP_EVENT_PORT, PTP_EVENT_PORT, (unsigned char*)&txPTPMsg, 44, 0);
			break;
		case PTP_DLYRPLY_MSGTYPE:
			// puts("\tPTP_DLYRPLY");
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
		ptpTimeRecord.t1Nanoseconds = (unsigned) PTP_TXCHAN_TIMESTAMP_NS(ptpPortInfo.eth_base);
		ptpTimeRecord.t1Seconds = (unsigned) PTP_TXCHAN_TIMESTAMP_SEC(ptpPortInfo.eth_base);
		PTP_TXCHAN_STATUS(ptpPortInfo.eth_base) = 0x1; //Clear PTP flag
		// puts("\t using hw timestamping");
		#else
		ptpTimeRecord.t1Nanoseconds = (unsigned) RTC_TIME_NS(ptpPortInfo.eth_base);
		ptpTimeRecord.t1Seconds = (unsigned) RTC_TIME_SEC(ptpPortInfo.eth_base);
		// puts("\t using sw timestamping");
		#endif
	} else if(msgType==PTP_DLYREQ_MSGTYPE){
		//Slave
		#ifdef USE_HW_TIMESTAMP
		ptpTimeRecord.t3Nanoseconds = (unsigned) PTP_TXCHAN_TIMESTAMP_NS(ptpPortInfo.eth_base);
		ptpTimeRecord.t3Seconds = (unsigned) PTP_TXCHAN_TIMESTAMP_SEC(ptpPortInfo.eth_base);
		PTP_TXCHAN_STATUS(ptpPortInfo.eth_base) = 0x1; //Clear PTP flag
		// puts("\t using hw timestamping");
		#else
		ptpTimeRecord.t3Nanoseconds = (unsigned) RTC_TIME_NS(ptpPortInfo.eth_base);
		ptpTimeRecord.t3Seconds = (unsigned) RTC_TIME_SEC(ptpPortInfo.eth_base);
		// puts("\t using sw timestamping");
		#endif
	}
	return msgType;
}

__attribute__((noinline))
int ptpv2_handle_msg(PTPPortInfo ptpPortInfo, unsigned tx_addr, unsigned rx_addr, unsigned char source_mac[6]){
	signed char ans = -2;
	unsigned char source_ip[4];
	#ifdef USE_HW_TIMESTAMP
	unsigned int timestampNanoseconds = (unsigned) PTP_RXCHAN_TIMESTAMP_NS(ptpPortInfo.eth_base);
	unsigned int timestampSeconds =  (unsigned) PTP_RXCHAN_TIMESTAMP_SEC(ptpPortInfo.eth_base);
	PTP_RXCHAN_STATUS(thisPtpPortInfo.eth_base) = 0x1; //Clear PTP flag
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
		// if(ptp_filter_clockport(rxPTPMsg.body.requestingSourcePort, rxPTPMsg.body.requestingSourceId, ptpPortInfo.clockId, ptpPortInfo.id)){
			ptpTimeRecord.t4Nanoseconds = rxPTPMsg.body.nanoseconds;
			ptpTimeRecord.t4Seconds = rxPTPMsg.body.seconds;
			ptpTimeRecord.delayNanoseconds = ptp_calc_delay(ptpTimeRecord.t1Nanoseconds, ptpTimeRecord.t2Nanoseconds, ptpTimeRecord.t3Nanoseconds, ptpTimeRecord.t4Nanoseconds);
			ptpTimeRecord.offsetNanoseconds = ptp_calc_offset(ptpTimeRecord.t1Nanoseconds, ptpTimeRecord.t2Nanoseconds, ptpTimeRecord.delayNanoseconds);
			ptpTimeRecord.delaySeconds = ptp_calc_delay(ptpTimeRecord.t1Seconds, ptpTimeRecord.t2Seconds, ptpTimeRecord.t3Seconds, ptpTimeRecord.t4Seconds);
			ptpTimeRecord.offsetSeconds = ptp_calc_offset(ptpTimeRecord.t1Seconds, ptpTimeRecord.t2Seconds, ptpTimeRecord.delaySeconds);
			if (PTP_CORRECTION_EN == 1) ptp_correct_offset(ptpPortInfo);
			ans = PTP_DLYRPLY_MSGTYPE;
		// }
		break;
	default:
		return -PTP;
		break;
	}
	return ans;
}

//Applies the correction mechanism based on the calculated offset and acceptable threshold value
__attribute__((noinline))
void ptp_correct_offset(PTPPortInfo ptpPortInfo){
	if(ptpTimeRecord.offsetSeconds != 0){
		RTC_TIME_SEC(ptpPortInfo.eth_base) = (unsigned) (-ptpTimeRecord.offsetSeconds + (int)RTC_TIME_SEC(ptpPortInfo.eth_base));	//reverse order to load time operand last
		RTC_TIME_NS(ptpPortInfo.eth_base) = (unsigned) (-(ptpTimeRecord.offsetNanoseconds) + WCET_COMPENSATION + (int)RTC_TIME_NS(ptpPortInfo.eth_base));	//reverse order to load time operand last
	} else {
		if(PTP_RATE_CONTROL==0 || abs(ptpTimeRecord.offsetNanoseconds) > PTP_NS_OFFSET_THRESHOLD){
			RTC_TIME_NS(ptpPortInfo.eth_base) = (unsigned) (-(ptpTimeRecord.offsetNanoseconds) + WCET_COMPENSATION + (int)RTC_TIME_NS(ptpPortInfo.eth_base));	//reverse order to load time operand last
		} else {
			// float driftCompens = (DRIFT_RATE * SYNC_INTERVAL_OPTIONS[-((signed char)ptpTimeRecord.syncInterval)]) / SEC_TO_USEC;
			RTC_ADJUST_OFFSET(ptpPortInfo.eth_base) = (int) (ptpTimeRecord.offsetNanoseconds);
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

unsigned long long get_ptp_usecs(unsigned int eth_base){
	return (unsigned long long) (SEC_TO_USEC * RTC_TIME_SEC(eth_base)) + (NS_TO_USEC * (RTC_TIME_NS(eth_base)));
}

unsigned int get_ptp_secs(unsigned int eth_base){
	return (unsigned int) (RTC_TIME_SEC(eth_base));
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
