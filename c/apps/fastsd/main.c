#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "sdcio.h"

int main() {

    // write to argument
    *(SDCIO_BASE + 0x00) = 0xffffffff;
    // write to command 
    *(SDCIO_BASE + 0x04) = 0xffffffff;  // note bits 31 downto 14 are reserved and therefore always 0 on read.
    // ...
    // read a read only register e.g. voltage
    uint32_t voltage = *(SDCIO_BASE + 0x2C);
    // write to rx_tx_buffer (bit 11 is the toggle bit for ADDR_WIDTH=14 in HW)
    *(SDCIO_BASE+(1<<11)+0) = 0x0000000f;
    *(SDCIO_BASE+(1<<11)+1) = 0x000000ff;

    // read and print the crap we have written before
    printf("Content of argument is %08lx\n", *(SDCIO_BASE + 0x00));
    printf("Content of command is %08lx\n", *(SDCIO_BASE + 0x04));
    printf("Voltage is %lu\n", voltage);
    printf("Content of addr 0 of rx_tx_buffer is %08lx\n", *(SDCIO_BASE+(1<<11)+0));
    printf("Content of addr 0 of rx_tx_buffer is %08lx\n", *(SDCIO_BASE+(1<<11)+1));

   return 0;
}

