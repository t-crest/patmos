#ifndef FLISERV_H
#define FLISERV_H

#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include "ethlib/eth_mac_driver.h"
#include "ethlib/icmp.h"
#include "ethlib/arp.h"
#include "ethlib/mac.h"
#include "ethlib/udp.h"
#include "ethlib/ipv4.h"
#include "ethlib/tte.h"
#include "xplane.h"

#define NS_TO_SEC 0.000000001
#define NS_TO_USEC 0.001
#define USEC_TO_NS 1000
#define USEC_TO_SEC 0.000001
#define SEC_TO_NS 1000000000
#define SEC_TO_USEC 1000000
#define SEC_TO_HOUR 0.000277777778

#define MS_TO_MAJA 10 //1 ms to tenths of us

#define INT_PERIOD 100 //ms
#define CYC_PERIOD 200 //ms
#define TTE_MAX_TRANS_DELAY 10848 //clock cycles from net_config (eclipse project)
#define TTE_COMP_DELAY 4000
#define TTE_PRECISION 800 //clock cycles from network_description (eclipse project)
#define TTE_VL_PERIOD 1000 //us

volatile _IODEV int *led_ptr = (volatile _IODEV int *) PATMOS_IO_LED;
volatile _IODEV unsigned* disp_ptr = (volatile _IODEV unsigned*) PATMOS_IO_SEGDISP;

typedef struct {
    unsigned base_addr;
    unsigned rx_addr;
    unsigned tx_addr;
} ethif_t;

void print_hex_bytes(unsigned char byte_buffer[], unsigned int len);
void print_segment(unsigned number);
void print_comm_info();
int checkForFrame(ethif_t ethif, const unsigned int timeout);
void handle_fsim_msg(ethif_t ethif, fsim_msg_circbuf_t *fsim_msg_buf);
void send_sensor_data(ethif_t, fsim_msg_circbuf_t *fsim_msg_buf);
void execute();

#endif // !FLISERV_H