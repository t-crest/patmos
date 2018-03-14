#include <stdio.h>
#include <machine/exceptions.h>
#include <machine/rtc.h>
#include "eth_patmos_io.h"
#include "eth_mac_driver.h"

#define TTE_CT_MARKER    0xABADBABE
#define TTE_MAX_TRANS_DELAY 0x638 //0x638!! 0x13880 = 1ms 0x61A80 = 5ms  clock cycles at 80MHz (12.5 ns)
#define TTE_WIRE_DELAY 0 //0x4E20 = 1/4 ms clock cycles at 80MHz (12.5 ns)
#define TTE_STATIC_RECIEVE_DELAY 0 //clock cycles at 80MHz (12.5 ns)
#define TTE_COMP_DELAY 0x349 //clock cycles at 80MHz (12.5 ns)
#define TTE_PRECISION 0x33E//0x33E! // 0x61A80 = 5ms 0x13880 = 1ms

unsigned char is_pcf(unsigned int addr);

int handle_integration_frame(unsigned int addr,unsigned long long rec_start,unsigned long long r_pit[],unsigned long long p_pit[],unsigned long long s_pit[],unsigned int int_pd[],unsigned long long trans_clk[],int i);

unsigned char is_tte(unsigned int addr);

void tte_initialize(unsigned int int_period);

//void tte_clock_tick(void);

unsigned long long transClk_to_clk (unsigned long long transClk);

void tte_prepare_test_data(unsigned int tx_addr);

void tte_send_test_data(unsigned int tx_addr);
