#ifndef TTEPWM_H
#define TTEPWM_H

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
// CPS constraints
#define PWM_PERIOD 20000
#define MIN_CYCLE 0.015
#define MAX_CYCLE 0.1
#define MOTOR_STEP 0.001
// Time-triggered constraints
#define NET_DELAY 23
#define DISP_SYM_MASK 0x80
#define PTP_PERIOD 62500
#define RPRT_PERIOD 500000
#define DAQ_PERIOD 500000
#define NODE0 0
#define NODE1 1
// Macros
#define HIGH_TIME(DUTY_CYCLE) PWM_PERIOD*DUTY_CYCLE
#define LOW_TIME(DUTY_CYCLE) PWM_PERIOD-HIGH_TIME(DUTY_CYCLE)
#define SYNC_INTERVAL (int) log2((int)PTP_PERIOD*USEC_TO_SEC)
#define DEAD_COMP 0
#define DEAD_CALC(DUTY_CYCLE, CPU_PERIOD) (HIGH_TIME(dutyCycle)*USEC_TO_NS/CPU_PERIOD)-DEAD_COMP

/*
 * Types
 */
typedef struct {
	unsigned dutyCycle;
} AppMsg;

/*
 * Global variables
 */
volatile _SPM int *led_ptr  = (volatile _SPM int *)  PATMOS_IO_LED;
volatile _SPM int *key_ptr = (volatile _SPM int *)	 PATMOS_IO_KEYS;
volatile _SPM int *gpio_ptr = (volatile _SPM int *) PATMOS_IO_GPIO;
volatile _IODEV unsigned *disp_ptr = (volatile _SPM unsigned *) PATMOS_IO_SEGDISP;
volatile _IODEV int *dead_ptr = (volatile _IODEV int *) PATMOS_IO_DEADLINE;

unsigned int rx_addr = 0x000;
unsigned int tx_addr = 0x800;

float cpuPeriod = 12.5;         // cpu period in nanos
unsigned long long actTimer = 0;            // used to for scheduling, current time in micros
unsigned char ssyncTurn = 0;    // which nodes turn is to reply to a sync message
float dutyCycle = 0.075;          // the duty cycle of the PWM
AppMsg appMsg = {               // the TT message exchanged between the two nodes
    .dutyCycle = 10
}; 

/*
 * Function Prototypes
 */
int check_packet(unsigned long long timeout);
void print_general_info();
void printSegmentInt(unsigned number);
void exec_report_task();
void exec_act_task(float dutyCycle);
float exec_daq_task(unsigned long long timeout);
void exec_slvsync_task(unsigned long long timeout);
void server_run();
void client_run();
unsigned char test_ptp_offset_correction();

#endif // !TTEPWM_H