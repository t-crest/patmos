#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <machine/rtc.h>
#include <machine/spm.h>
#include "ethlib/ptp1588.h"

#define TEST_FRAMES 100

// #define TEST_TS
#define TEST_RATE

volatile _SPM unsigned *disp_ptr = (volatile _SPM unsigned *) PATMOS_IO_SEGDISP;

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

void measure_timestamp_rate(){
    register unsigned int count_frames = 0;
    register unsigned long long last_ts = 0;
    register unsigned long long delta_sum = 0;
    unsigned long long start = get_cpu_usecs();
    while(count_frames < TEST_FRAMES){
        PTP_RXCHAN_STATUS(PATMOS_IO_ETH) = 0x1;
        while(!PTP_RXCHAN_STATUS(PATMOS_IO_ETH)){continue;} //spin while and clear flag while no frame
        delta_sum += (last_ts > 0 ? get_rx_timestamp_nanos(PATMOS_IO_ETH) - last_ts : 0);
        last_ts = get_rx_timestamp_nanos(PATMOS_IO_ETH);
        count_frames++;
    }
    unsigned long long finish = get_cpu_usecs();
    printf("Avg. rx_delta_time = %.3f usec\n", (delta_sum / count_frames) * NS_TO_USEC);
    printf("Test duration %.3f sec\n", (finish - start) * USEC_TO_SEC);
}

void test_timestamp_rx_channel(){
    register unsigned int count_frames = 0;
    unsigned long long start = get_cpu_usecs();
    while(count_frames < TEST_FRAMES){
        unsigned char stat = PTP_RXCHAN_STATUS(PATMOS_IO_ETH);
        count_frames += stat;
        printf("%x", stat);
        PTP_RXCHAN_STATUS(PATMOS_IO_ETH) = stat;
        stat = PTP_RXCHAN_STATUS(PATMOS_IO_ETH);
        printf("%x", stat);
        count_frames += stat;
    }
    unsigned long long finish = get_cpu_usecs();
    printf("\nReceived frames count = %u\n", count_frames);
    printf("Test duration %.3f sec\n", (finish - start) * USEC_TO_SEC);
}

int main(){
    puts("Starting AION sanity check\n");
#ifdef TEST_TS
    test_timestamp_rx_channel();
#endif
#ifdef TEST_RATE
    measure_timestamp_rate();
#endif
    return 0;
}

