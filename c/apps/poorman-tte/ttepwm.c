/*
	Author: Lefteris Kyriakakis 
	Copyright: DTU, BSD License
*/
/*
	Example of a time-triggered network control of a PWM

	Author: Lefteris Kyriakakis 
	Copyright: DTU, BSD License
*/
#include "ttepwm.h"

int main(int argc, char **argv){
	//MAC controller settings
	eth_mac_initialize();
    srand((unsigned) get_cpu_usecs());
    unsigned char rand_addr = rand()%253;
    ipv4_set_my_ip((unsigned char[4]){192, 168, 2, rand_addr});
    cpuPeriod = (1.0f/get_cpu_freq()) * SEC_TO_NS;
    //Initialize node
    dutyCycle = 0.1;
    *led_ptr = 0x1FF;
    *gpio_ptr = 0x0;
    print_general_info();
#ifdef CLIENT
    *led_ptr = 0x0;
    ssyncTurn = NODE1;
    actTimer = get_rtc_usecs();
    while(1){
        client_run();
    }
    *led_ptr = 0x10F;
#endif
#ifdef SERVER
    *led_ptr = 0x0;
    ssyncTurn = NODE0;
    actTimer = get_rtc_usecs();
    while(1){
        server_run();
    }
    *led_ptr = 0x1F0;
#endif
    *gpio_ptr = 0x0;
    *led_ptr = 0x1FF;
    printf("Exiting...");
    *led_ptr = 0x100;
}

void server_run(){
    if(get_rtc_usecs()-actTimer >= PWM_PERIOD){
        actTimer = get_rtc_usecs();
        exec_act_task(dutyCycle);
        exec_report_task();
    } else if(get_rtc_usecs()-actTimer < PWM_PERIOD-PTP_REQ_TIMEOUT) {
        exec_slvsync_task(PTP_REQ_TIMEOUT);
    }
}

void client_run(){
    unsigned long long elapsedTime = get_rtc_usecs()-actTimer;
    if(elapsedTime >= PWM_PERIOD)
    {
        actTimer = get_rtc_usecs();
        exec_act_task(dutyCycle);
        exec_report_task();
        dutyCycle = (dutyCycle >= 0.1) ? 0.05 : dutyCycle + 0.0001;
    } 
    else if(elapsedTime + PTP_REQ_TIMEOUT < PWM_PERIOD)
    {
        exec_slvsync_task(PTP_REQ_TIMEOUT);
    } 
}

/*
 * Tasks
 */
__attribute__((noinline))
void exec_report_task(){
    printSegmentInt(get_rtc_secs());
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
        switch(ptpv2_handle_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC)){
        case PTP_SYNC_MSGTYPE:
            if((rxPTPMsg.head.flagField & FLAG_PTP_TWO_STEP_MASK) != FLAG_PTP_TWO_STEP_MASK){
                ptpv2_issue_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, lastMasterInfo.ip, rxPTPMsg.head.sequenceId, PTP_DLYREQ_MSGTYPE, ptpTimeRecord.syncInterval);
            }
            *led_ptr |= 0x1;
            break;
        case PTP_FOLLOW_MSGTYPE:
            *led_ptr |= 0x2;
            ptpv2_issue_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, lastMasterInfo.ip, rxPTPMsg.head.sequenceId, PTP_DLYREQ_MSGTYPE, ptpTimeRecord.syncInterval);
            *led_ptr |= 0x4;
            break;
        case PTP_DLYRPLY_MSGTYPE:
            *led_ptr |= 0x8;
            // ssyncTurn = 1 - ssyncTurn; //if synced then flip access
            break;
        default:
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

void print_general_info(){
	printf("\nGeneral info:\n");
    printf("\tNODE:");
    #ifdef SERVER
        printf(" as server\n");
    #endif
    #ifdef CLIENT
        printf(" as client\n");
    #endif
    printf("\tCPU Period (ns): %f\n", cpuPeriod);
	printf("\tMAC: %llx", get_mac_address());
	printf("\n\tIP: ");
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