/*
	Author: Lefteris Kyriakakis 
	Copyright: DTU, BSD License
*/
/*
	Measure network roundtrip using hardware timestamping and PTP

	Author: Lefteris Kyriakakis 
	Copyright: DTU, BSD License
*/
#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/spm.h>
#include <machine/rtc.h>
#include "ethlib/icmp.h"
#include "ethlib/arp.h"
#include "ethlib/mac.h"
#include "ethlib/udp.h"
#include "ethlib/ipv4.h"
#include "ethlib/ptp1588.h"

// #define SERVER
// #define CLIENT

void server_run(){

}

void client_run(){

}

void init_ptp_master_loop(unsigned long long period, unsigned short iterations){
	unsigned short int seqId = 0;
    signed char ans = 0;
	unsigned long long start_time = get_rtc_usecs();
	do{
		if(0xE == *key_ptr){
            return;
        } else if (get_rtc_usecs()-start_time >= period){
			printf("Seq# %u\n", seqId);
			//Send SYNQ
			*led_ptr = PTP_SYNC_MSGTYPE;
			//printf("%.3fus\n", elapsed_time);
			puts("tx_SYNC");
			ptpv2_issue_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, lastSlaveInfo.ip, seqId, PTP_SYNC_MSGTYPE, SYNC_INTERVAL);
			//Send FOLLOW_UP
			*led_ptr = PTP_FOLLOW_MSGTYPE;
			puts("tx_FOLLOW");
			ptpv2_issue_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, lastSlaveInfo.ip, seqId, PTP_FOLLOW_MSGTYPE, SYNC_INTERVAL);
			if(check_packet(PTP_REQ_TIMEOUT) == PTP){
				if((ans = ptpv2_handle_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC)) == PTP_DLYREQ_MSGTYPE){
					*led_ptr = PTP_DLYREQ_MSGTYPE;
					ptpv2_issue_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, lastSlaveInfo.ip, rxPTPMsg.head.sequenceId, PTP_DLYRPLY_MSGTYPE, SYNC_INTERVAL);
					*led_ptr = PTP_DLYRPLY_MSGTYPE;
					seqId++;
				}
			}
			start_time = get_rtc_usecs();
		}
		printSegmentInt(get_rtc_secs());
	}while(rxPTPMsg.head.sequenceId < iterations);
}

void init_ptp_slave_loop(unsigned short iterations){
	unsigned short int seqId = 0;
	do{
        if(0xE == *key_ptr){
            return;
        } else if(check_packet(PTP_SYNC_TIMEOUT) == PTP){
			switch(ptpv2_handle_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC)){
			case PTP_SYNC_MSGTYPE:
				if((rxPTPMsg.head.flagField & FLAG_PTP_TWO_STEP_MASK) != FLAG_PTP_TWO_STEP_MASK){
					*led_ptr = PTP_SYNC_MSGTYPE;
					ptpv2_issue_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, lastMasterInfo.ip, rxPTPMsg.head.sequenceId, PTP_DLYREQ_MSGTYPE, ptpTimeRecord.syncInterval);
					*led_ptr = PTP_DLYREQ_MSGTYPE;
				}
				break;
			case PTP_FOLLOW_MSGTYPE:
				*led_ptr = PTP_FOLLOW_MSGTYPE;
				ptpv2_issue_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, lastMasterInfo.ip, rxPTPMsg.head.sequenceId, PTP_DLYREQ_MSGTYPE, ptpTimeRecord.syncInterval);
				*led_ptr = PTP_DLYREQ_MSGTYPE;
				break;
			case PTP_DLYRPLY_MSGTYPE:
				*led_ptr = PTP_DLYRPLY_MSGTYPE;
				printf("#%u\t%d\t%d\n", rxPTPMsg.head.sequenceId, ptpTimeRecord.offsetSeconds, ptpTimeRecord.offsetNanoseconds);
				break;
			case PTP_ANNOUNCE_MSGTYPE:
				*led_ptr = PTP_ANNOUNCE_MSGTYPE;
				break;
			default:
				*led_ptr = 0x100 | *led_ptr;
				break;
			}
		}
		printSegmentInt(get_rtc_secs());
	}while(rxPTPMsg.head.sequenceId < iterations);
}

