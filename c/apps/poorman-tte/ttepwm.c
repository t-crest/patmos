/*
	Author: Lefteris Kyriakakis 
	Copyright: DTU, BSD License
*/
/*
	Test GPIO device by generating a simple PWM of configurable DUTY_CYCLE and PERIOD.

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

/*
 * Defines
 */
// #define SERVER
// #define CLIENT
#define COMM_OVERHEAD 23000
#define DISP_SYM_MASK 0x80
#define PTP_PERIOD 15625
#define RPRT_PERIOD 500000
#define DAQ_PERIOD 1000
#define PWM_PERIOD 20000
//Macros
#define HIGH_TIME(DUTY_CYCLE) PWM_PERIOD*DUTY_CYCLE
#define LOW_TIME(DUTY_CYCLE) PWM_PERIOD-HIGH_TIME(DUTY_CYCLE)
#define SYNC_INTERVAL (int) log2((int)PTP_PERIOD*USEC_TO_SEC)

enum ttepwm_tasks{DAQ_TASK=1, ACT_TASK=2, SYNC_TASK=4, REPORT_TASK=8};
enum sync_status{SYNC_ST=1, FOLLOW_ST=2, REQ_ST=4, RPLY_ST=8};

volatile _SPM int *led_ptr  = (volatile _SPM int *)  0xF0090000;
volatile _SPM int *key_ptr = (volatile _SPM int *)	 0xF00A0000;
volatile _SPM int *gpio_ptr = (volatile _SPM int *) 0xF00C0000;
volatile _IODEV unsigned *disp_ptr = (volatile _SPM unsigned *) 0xF00B0000;

unsigned int rx_addr = 0x000;
unsigned int tx_addr = 0x800;

typedef struct {
	float dutyCycle;
} AppMsg;

enum ttepwm_tasks currentTask;
enum sync_status syncState;
unsigned short int seqId = 0;
unsigned char syncAttempts = 0;
float dutyCycle = 0.1;
unsigned long long actTimer = 0;
unsigned long long syncTimer = 0;
unsigned long long rprtTimer = 0;
unsigned long long daqTimer = 0;
unsigned long long pwmTimer = 0;
AppMsg appMsg = {.dutyCycle = 0.1};

void print_general_info(){
	printf("\nGeneral info:\n");
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
			return ARP;
		default:
			return UNSUPPORTED;
		}
	} else {
		return UNSUPPORTED;
	}
}

__attribute__((noinline))
void exec_daq_task(){
#ifdef SERVER
    udp_send_mac(tx_addr, rx_addr, PTP_BROADCAST_MAC, PTP_MULTICAST_IP, 666, 666, (unsigned char*) &appMsg, sizeof(AppMsg), 0);
    appMsg.dutyCycle=appMsg.dutyCycle + 0.01;
#endif
#ifdef CLIENT
    if(check_packet(PTP_REQ_TIMEOUT) == UDP){
        udp_get_data(rx_addr, (unsigned char*) &appMsg, sizeof(AppMsg));           
    }
#endif
}

__attribute__((noinline))
void exec_act_task(){
    pwmTimer = get_rtc_usecs();
    *gpio_ptr = 0x1;
    _Pragma("loopbound min 0 max 0") //2ms
    while(get_rtc_usecs()-pwmTimer < HIGH_TIME(dutyCycle)){;}
    *gpio_ptr = 0x0;
}

__attribute__((noinline))
void exec_sync_task(unsigned long long timeout){
#ifdef SERVER
    switch(syncState){
        case SYNC_ST:
            ptpv2_issue_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, lastSlaveInfo.ip, seqId, PTP_SYNC_MSGTYPE, SYNC_INTERVAL);
            syncState = FOLLOW_ST;
        break;
        case FOLLOW_ST:
            ptpv2_issue_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, lastSlaveInfo.ip, seqId, PTP_FOLLOW_MSGTYPE, SYNC_INTERVAL);
            if(check_packet(timeout) == PTP){
                if(ptpv2_handle_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC) == PTP_DLYREQ_MSGTYPE){
                    ptpv2_issue_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, lastSlaveInfo.ip, rxPTPMsg.head.sequenceId, PTP_DLYRPLY_MSGTYPE, SYNC_INTERVAL);
                    seqId++;
                }
            }
            syncState = SYNC_ST;
        break;
    }
#endif
#ifdef CLIENT
    if(check_packet(timeout) == PTP){
        if(ptpv2_handle_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC) == PTP_FOLLOW_MSGTYPE){
            ptpv2_issue_msg(tx_addr, rx_addr, PTP_BROADCAST_MAC, lastMasterInfo.ip, rxPTPMsg.head.sequenceId, PTP_DLYREQ_MSGTYPE, ptpTimeRecord.syncInterval);
        }
    }
#endif
}

__attribute__((noinline))
void exec_report_task(){
    printSegmentInt(get_rtc_secs());
}

void server_run(){
    switch(currentTask){
        case SYNC_TASK:
            if(get_rtc_usecs()-syncTimer >= PTP_PERIOD/2){
                syncTimer = get_rtc_usecs();
                exec_sync_task(PTP_REQ_TIMEOUT);
            }
            currentTask = REPORT_TASK;
        break;
        case ACT_TASK:
            if(get_rtc_usecs()-actTimer >= PWM_PERIOD){
                actTimer = get_rtc_usecs();
                exec_act_task();
            }
            currentTask = SYNC_TASK;
        break;
        case DAQ_TASK:
            if(get_rtc_usecs()-daqTimer >= DAQ_PERIOD){
                daqTimer = get_rtc_usecs();
                exec_daq_task();
            }
            currentTask = ACT_TASK;
        break;
        case REPORT_TASK:
            if(get_rtc_usecs()-rprtTimer >= RPRT_PERIOD){
                rprtTimer = get_rtc_usecs();
                exec_report_task();
            }
            currentTask = DAQ_TASK;
        break;
    }
    *led_ptr = currentTask;
}

void client_run(){
    switch(currentTask){
        case SYNC_TASK:
            if(get_rtc_usecs()-syncTimer >= PTP_PERIOD/2 - COMM_OVERHEAD){
                syncTimer = get_rtc_usecs(); 
                exec_sync_task(PTP_REQ_TIMEOUT);
            }
            currentTask = REPORT_TASK;
        break;
        case ACT_TASK:
            if(get_rtc_usecs()-actTimer >= PWM_PERIOD){
                actTimer = get_rtc_usecs();
                exec_act_task();
            }
            currentTask = SYNC_TASK;
        break;
        case DAQ_TASK:
            if(get_rtc_usecs()-daqTimer >= DAQ_PERIOD - COMM_OVERHEAD){
                daqTimer = get_rtc_usecs();
                exec_daq_task();
            }
            currentTask = ACT_TASK;
        break;
        case REPORT_TASK:
            if(get_rtc_usecs()-rprtTimer >= RPRT_PERIOD){
                rprtTimer = get_rtc_usecs() + COMM_OVERHEAD;
                exec_report_task();
            }
            currentTask = DAQ_TASK;
        break;
    }
    *led_ptr = currentTask;
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

int main(int argc, char **argv){
	//MAC controller settings
	eth_mac_initialize();
    ipv4_set_my_ip((unsigned char[4]){192, 168, 2, 254});
    arp_table_init();
    print_general_info();
    //Initial Duty Cycle
    *led_ptr = 0x1FF;
    *gpio_ptr = 0x0;
#ifdef CLIENT
    while(0xD != *key_ptr){
        seqId = 0x0;
        puts("Client started");
        init_ptp_slave_loop(500);
        *led_ptr = 0x00F;
        currentTask = SYNC_TASK;
        syncState = SYNC_ST;
        rprtTimer = get_rtc_usecs();
        syncTimer = get_rtc_usecs();
        while(0xE != *key_ptr){
            client_run();
        }
    }
    *led_ptr = 0x10F;
#endif
#ifdef SERVER
    while(0xD != *key_ptr){
        puts("Server started");
        init_ptp_master_loop(PTP_PERIOD, 500);
        *led_ptr = 0xF0;
        currentTask = SYNC_TASK;
        syncState = SYNC_ST;
        actTimer = get_rtc_usecs();
        syncTimer = get_rtc_usecs();
        rprtTimer = get_rtc_usecs();
        while(0xE != *key_ptr){
            server_run();
        }
    }
    *led_ptr = 0x1F0;
#endif
    *gpio_ptr = 0x0;
    printf("Exiting...");
}