/*
	Example of a time-triggered network control of a PWM

	Author: Lefteris Kyriakakis 
	Copyright: DTU, BSD License
*/
#include "ttepwm.h"

int main(int argc, char **argv){
	//MAC controller settings
    *led_ptr = 0x1FF;
	eth_mac_initialize();
    srand((unsigned) get_cpu_usecs());
    unsigned char rand_addr = rand()%253;
    ipv4_set_my_ip((unsigned char[4]){192, 168, 2, rand_addr});
    arp_table_init();
    thisPtpPortInfo = ptpv2_intialize_local_port(PATMOS_IO_ETH, my_mac, (unsigned char[4]){192, 168, 2, rand_addr}, rand_addr);
    cpuPeriod = (1.0f/get_cpu_freq()) * SEC_TO_NS;
    //Initialize node
    dutyCycle = 0.1;
    *gpio_ptr = 0x0;
    print_general_info();
    puts("Testing ptp offset correction mechanism");
    test_ptp_offset_correction();
#ifdef CLIENT
    *led_ptr = 0x0;
    ssyncTurn = NODE1;
    actTimer = get_ptp_usecs(thisPtpPortInfo.eth_base);
    while(1){
        client_run();
    }
    *led_ptr = 0x10F;
#endif
#ifdef SERVER
    *led_ptr = 0x0;
    ssyncTurn = NODE0;
    actTimer = get_ptp_usecs(thisPtpPortInfo.eth_base);
    while(1){
        server_run();
    }
    *led_ptr = 0x1F0;
#endif
    *gpio_ptr = 0x0;
    *led_ptr = 0x1FF;
    puts("Exiting...");
    *led_ptr = 0x100;
}

void client_run(){
    register unsigned long long elapsedTime = get_ptp_usecs(thisPtpPortInfo.eth_base)-actTimer;
    if(elapsedTime >= PWM_PERIOD)
    {
        exec_act_task(dutyCycle);
        exec_report_task();
        if(ssyncTurn == NODE0) { 
            exec_slvsync_task(PTP_REQ_TIMEOUT);
            ssyncTurn = NODE1;
        } else {
            ssyncTurn = NODE0;
        }
        actTimer = get_ptp_usecs(thisPtpPortInfo.eth_base);
    }
}

void server_run(){
    register unsigned long long elapsedTime = get_ptp_usecs(thisPtpPortInfo.eth_base)-actTimer;
    if(elapsedTime >= PWM_PERIOD)
    {
        exec_act_task(dutyCycle);
        exec_report_task();
        if(ssyncTurn == NODE1) { 
            exec_slvsync_task(PTP_REQ_TIMEOUT);
            ssyncTurn = NODE0;
        } else {
            ssyncTurn = NODE1;
        }
        actTimer = get_ptp_usecs(thisPtpPortInfo.eth_base);
    }
}

/*
 * Tasks
 */
__attribute__((noinline))
void exec_report_task(){
    printSegmentInt(get_ptp_secs(thisPtpPortInfo.eth_base));
    #ifdef SERVER
    *led_ptr |= 0x80;
    #endif
    #ifdef CLIENT
    *led_ptr |= 0x10;
    #endif
}

__attribute__((noinline))
void exec_act_task(float dutyCycle){
    int val = 0;
    *gpio_ptr = 0x1;
    *dead_ptr = DEAD_CALC(dutyCycle, cpuPeriod);
    val = *dead_ptr;
    *gpio_ptr = 0x0;
}

__attribute__((noinline))
float exec_daq_task(unsigned long long timeout){
    #ifdef SERVER
        udp_send_mac(tx_addr, rx_addr, PTP_BROADCAST_MAC, PTP_MULTICAST_IP, 666, 666, (unsigned char*) &appMsg, 4, 0);
        // appMsg.dutyCycle=appMsg.dutyCycle + 1;
        if(appMsg.dutyCycle < 5){
            appMsg.dutyCycle = 10;
        } else if(appMsg.dutyCycle > 10){
            appMsg.dutyCycle = 5;
        }
    #endif
    #ifdef CLIENT
    if(check_packet(timeout) == UDP){
        udp_get_data(rx_addr, (unsigned char*) &appMsg, 4);
    }
    #endif           
    return (float) (appMsg.dutyCycle / 100.0f);
}

__attribute__((noinline))
void exec_slvsync_task(unsigned long long timeout){
    if((*led_ptr = check_packet(timeout)) == PTP){
        switch(ptpv2_handle_msg(thisPtpPortInfo, tx_addr, rx_addr, PTP_BROADCAST_MAC)){
			case PTP_SYNC_MSGTYPE:
				if((rxPTPMsg.head.flagField & FLAG_PTP_TWO_STEP_MASK) != FLAG_PTP_TWO_STEP_MASK){
					*led_ptr = 0x1;
					ptpv2_issue_msg(thisPtpPortInfo, tx_addr, rx_addr, PTP_BROADCAST_MAC, lastMasterInfo.ip, rxPTPMsg.head.sequenceId, PTP_DLYREQ_MSGTYPE, ptpTimeRecord.syncInterval);
					*led_ptr = 0x4;
				}
				break;
			case PTP_FOLLOW_MSGTYPE:
				*led_ptr = 0x2;
				ptpv2_issue_msg(thisPtpPortInfo, tx_addr, rx_addr, PTP_BROADCAST_MAC, lastMasterInfo.ip, rxPTPMsg.head.sequenceId, PTP_DLYREQ_MSGTYPE, ptpTimeRecord.syncInterval);
				*led_ptr = 0x4;
				break;
			case PTP_DLYRPLY_MSGTYPE:
				*led_ptr = 0x8;
				printf("#%u\t%d\t%d\n", rxPTPMsg.head.sequenceId, ptpTimeRecord.offsetSeconds, ptpTimeRecord.offsetNanoseconds);
				break;
			case PTP_ANNOUNCE_MSGTYPE:
				*led_ptr = 0xF;
				break;
			default:
				*led_ptr = 0x100 | *led_ptr;
				break;
        }
    }
}

/*
 * Utilities
 */
int check_packet(unsigned long long timeout){
	// if(eth_mac_receive_nb(rx_addr)){
    if(eth_mac_receive(rx_addr, timeout)){
        unsigned short dst_port;
		switch (mac_packet_type(rx_addr)) {
		case ICMP:
			return ICMP;
		case UDP:
            dst_port = udp_get_destination_port(rx_addr);
			if((dst_port==PTP_EVENT_PORT) || (dst_port==PTP_GENERAL_PORT)){
				return PTP;
			} else {
				return UDP;
			}
		case ARP:
            arp_process_received(rx_addr, tx_addr);
			return ARP;
		default:
			return UNSUPPORTED;
		}
	} else {
		return UNSUPPORTED;
	}
}

unsigned char test_ptp_offset_correction(){
    //Keep initial time
	unsigned int initNanoseconds = RTC_TIME_NS(PATMOS_IO_ETH);
	unsigned int initSeconds = RTC_TIME_SEC(PATMOS_IO_ETH);
	//Test offset
	RTC_ADJUST_OFFSET(thisPtpPortInfo.eth_base) = 1024;
	while((*led_ptr=RTC_ADJUST_OFFSET(thisPtpPortInfo.eth_base)) != 0){
        printSegmentInt(*led_ptr);
    }
	RTC_TIME_NS(thisPtpPortInfo.eth_base) = initNanoseconds;
	RTC_TIME_SEC(thisPtpPortInfo.eth_base) = initSeconds;
	return 1;
}


void print_general_info(){
	printf("\nNode General info:\n");
    printf("\tRole:");
    #ifdef SERVER
        printf(" as server\n");
    #endif
    #ifdef CLIENT
        printf(" as client\n");
    #endif
    printf("\tCPU Period (ns): %f\n", cpuPeriod);
    printf("\tETH Base: 0x%x\n", thisPtpPortInfo.eth_base);
	printf("\tMAC: %llx\n", get_mac_address());
	printf("\tIP: ");
	ipv4_print_my_ip();
	printf("\n");
	arp_table_print();
	printf("\n");
	return;
}

void printSegmentInt(unsigned number) {
    *(disp_ptr+0) = number & 0xF;
    *(disp_ptr+1) = (number >> 4) & 0xF;
    *(disp_ptr+2) = (number >> 8) & 0xF;
    *(disp_ptr+3) = (number >> 12) & 0xF;
    *(disp_ptr+4) = (number >> 16) & 0xF;
    *(disp_ptr+5) = (number >> 20) & 0xF;
    *(disp_ptr+6) = (number >> 24) & 0xF;
    *(disp_ptr+7) = (number >> 28) & 0xF;
}