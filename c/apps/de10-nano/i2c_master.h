/*
 *  Copyright 2020 TU Wien, Austria.
 *  All rights reserved.
 *
 *  This file is part of the time-predictable VLIW processor Patmos.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *     1. Redistributions of source code must retain the above copyright notice,
 *        this list of conditions and the following disclaimer.
 *
 *     2. Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
 *  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
 *  NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  The views and conclusions contained in the software and documentation are
 *  those of the authors and should not be interpreted as representing official
 *  policies, either expressed or implied, of the copyright holder.
 *
 *  SPDX-License-Identifier: BSD-2-Clause
 */

/**
 * Convenience functions for using the i2c master module of Patmos.
 *
 * Authors: Michael Platzer (TU Wien)
 *
 * Please note: The addresses for the register of the I2C master IO module are
 * currently hard-coded in i2c_master.c in the SPM address range 0xF00C0000
 * through 0xF00CFFF (i.e. IO device offset 12).
 */


#ifndef I2C_MASTER_H
#define I2C_MASTER_H

#include <stdint.h>

/**
 * Probe the presence of an i2c device with the given address. The presence of
 * an i2c device is detected by issuing the address (with the R/W bit cleared,
 * i.e. in write mode) and sensing whether an ACK is asserted on the bus. Then
 * a stop condition is generated before any data is transmitted.
 *
 * @param addr  i2c address of the device to probe
 * @return      0 if a device responded with an ACK on the bus and -1 if no
 *              device responded or an error occurred.
 */
int i2c_probe(uint8_t addr);

/**
 * Read several bytes from an 8-bit starting address.
 */
int i2c_reg8_read_stream(uint8_t addr, uint8_t reg, uint8_t *buf, int buf_len);

/**
 * Read an i2c register. The width of the register address and the width of the
 * data in the register is part of the function name: i2c_reg<n>_read<m> reads
 * a *m* bit wide register with an *n* bit wide address.
 *
 * @param addr  i2c address of the device from which to read.
 * @param reg   Address of the register from which to read.
 * @return      The content of the register or -1 if an error occurred.
 */
int i2c_reg8_read8(uint8_t addr, uint8_t reg);
int i2c_reg8_read16l(uint8_t addr, uint8_t reg);
int i2c_reg8_read16b(uint8_t addr, uint8_t reg);
int i2c_reg8_read24l(uint8_t addr, uint8_t reg);
int i2c_reg8_read24b(uint8_t addr, uint8_t reg);
int i2c_reg16l_read16l(uint8_t addr, uint16_t reg);
int i2c_reg16b_read16b(uint8_t addr, uint16_t reg);

/**
 * Write an i2c register. The width of the register address and the width of
 * the data in the register is part of the function name:
 * 'i2c_reg<n>_write<m><b>' writes a *m* bit wide register with a *n* bit wide
 * address. For registers with a width *m* > 8, *b* identifies the byte order;
 * 'l' means little endian, 'b' means big endian.
 *
 * @param addr  i2c address of the device which to write to
 * @param reg   Address of the register which to write to.
 * @param data  The data that shall be placed in the register.
 * @return      0 if the operation was successful or -1 if an error occurred.
 */
int i2c_reg8_write8_empty(uint8_t addr, uint8_t reg);
int i2c_reg8_write8(uint8_t addr, uint8_t reg, uint8_t data);
int i2c_reg8_write16l(uint8_t addr, uint8_t reg, uint16_t data);
int i2c_reg8_write16b(uint8_t addr, uint8_t reg, uint16_t data);
int i2c_reg8_write24l(uint8_t addr, uint8_t reg, uint32_t data);
int i2c_reg8_write24b(uint8_t addr, uint8_t reg, uint32_t data);
int i2c_reg16l_write16l(uint8_t addr, uint16_t reg, uint16_t data);
int i2c_reg16b_write16b(uint8_t addr, uint16_t reg, uint16_t data);

#endif // I2C_MASTER_H
