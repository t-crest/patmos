/**
 * @file sdcio.h
 * @author Philipp Birkl
 * @date 2021-01-03
 * @brief Lowlevel interface for the fast SD controller
 */
#ifndef SDCIO_H
#define SDCIO_H

#include <stdint.h>

typedef enum {
    R_ARGUMENT      = 0x00,
    R_COMMAND       = 0x04,
    R_RESP1         = 0x08,
    R_RESP2         = 0x0C,
    R_RESP3         = 0x10,
    R_RESP4         = 0x14,
    R_DAT_TIMEOUT   = 0x18,
    R_CONTROL       = 0x1C,
    R_CMD_TIMEOUT   = 0x20,
    R_CLK_DEVIDER   = 0x24,
    R_RESET         = 0x28,
    R_VOLTAGE       = 0x2C,
    R_CAPABILITIES  = 0x30,
    R_CMD_EV_STATUS = 0x34,
    R_CMD_EV_ENABLE = 0x38,
    R_DAT_EV_STATUS = 0x3C,
    R_DAT_EV_ENABLE = 0x40,
    R_BLOCK_SIZE    = 0x44,
    R_BLOCK_COUNT   = 0x48,
    R_BUFF_ADDR     = 0x60
} sdc_reg_t;

void sdc_reg_write(const sdc_reg_t reg, const uint32_t value);
uint32_t sdc_reg_read(const sdc_reg_t reg);

void sdc_buffer_write(const sdc_reg_t reg, const uint32_t value);
uint32_t sdc_buffer_read(const sdc_reg_t reg);
#endif
