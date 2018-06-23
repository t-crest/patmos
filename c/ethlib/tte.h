#include <stdio.h>
#include <stdlib.h>
#include <machine/exceptions.h>
#include <machine/rtc.h>
#include "eth_patmos_io.h"
#include "eth_mac_driver.h"

#define TTE_MAX_TRANS_DELAY 0x2A60 //0x638!! 0x13880 = 1ms 0x61A80 = 5ms  clock cycles at 80MHz (12.5 ns)
#define TTE_COMP_DELAY 0x349 //clock cycles at 80MHz (12.5 ns)
//#define TTE_PRECISION 0x33E//0x33E! // 0x61A80 = 5ms 0x13880 = 1ms
#define TTE_PRECISION 0x67C

unsigned char is_pcf(unsigned int addr);// __attribute__((noinline));

void tte_clear_free_rx_buffer(unsigned int addr);// __attribute__((noinline));

void tte_wait_for_message(unsigned long long * receive_point);

unsigned char tte_receive(unsigned int addr,unsigned long long rec_start);// __attribute__((noinline));

unsigned char tte_receive_log(unsigned int addr,unsigned long long rec_start,signed long long error[],int i);// __attribute__((noinline));

int handle_integration_frame(unsigned int addr,unsigned long long rec_start);// __attribute__((noinline));

int handle_integration_frame_log(unsigned int addr,unsigned long long rec_start,
  signed long long error[],int i);// __attribute__((noinline));

unsigned char is_tte(unsigned int addr);// __attribute__((noinline));

void tte_initialize(unsigned int int_period, unsigned int cl_period, unsigned char CT[], unsigned char VLcount);// __attribute__((noinline));

void tte_init_VL(unsigned char i, unsigned int start, unsigned int period);// __attribute__((noinline));

unsigned long long transClk_to_clk (unsigned long long transClk);// __attribute__((noinline));

void tte_prepare_test_data(unsigned int tx_addr, unsigned char VL[], unsigned char data, int length);// __attribute__((noinline));

void tte_prepare_pcf(unsigned int addr,unsigned char VL[],unsigned char type);// __attribute__((noinline));

void tte_send_data(unsigned char i);// __attribute__((noinline));

void tte_start_ticking(char enable_int, void (int_handler)(void));// __attribute__((noinline));

void tte_stop_ticking();// __attribute__((noinline));

char tte_schedule_send(unsigned int addr,unsigned int size,unsigned char i);// __attribute__((noinline));

void tte_clock_tick(void);// __attribute__((noinline));
