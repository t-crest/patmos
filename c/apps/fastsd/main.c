#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "sdcio.h"

int main() {

    // write to argument
    sdc_reg_write(R_ARGUMENT, 0xAA55AA55);
    // write to command
    // note bits 31 downto 14 are reserved and therefore always 0 on read.
    sdc_reg_write(R_COMMAND, 0xFFFFFFFF);
    // ...
    // read a read only register e.g. voltage
    uint32_t voltage = sdc_reg_read(R_VOLTAGE);

    // write to rx_tx_buffer (bit 11 is the toggle bit for ADDR_WIDTH=14 in HW)
    sdc_buffer_write(0, 0x0000000f);
    sdc_buffer_write(1, 0x000000ff);

    // read and print the crap we have written before
    printf("Content of argument is %08lx\n", sdc_reg_read(R_ARGUMENT));
    printf("Content of command is %08lx\n", sdc_reg_read(R_COMMAND));
    printf("Voltage is %lu\n", voltage);
    printf("Content of addr 0 of rx_tx_buffer is %08lx\n", sdc_buffer_read(0));
    printf("Content of addr 0 of rx_tx_buffer is %08lx\n", sdc_buffer_read(1));

   return 0;
}

