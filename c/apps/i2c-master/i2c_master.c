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
 */


#include "i2c_master.h"

#include <machine/spm.h>

static volatile _SPM int *i2c_ctrl   = (volatile _SPM int *) 0xF00C0000;
static volatile _SPM int *i2c_status = (volatile _SPM int *) 0xF00C0004;
static volatile _SPM int *i2c_addr   = (volatile _SPM int *) 0xF00C0008;
static volatile _SPM int *i2c_data   = (volatile _SPM int *) 0xF00C000C;

static int i2c_write(uint8_t addr, int cnt, uint8_t *buf)
{
    if (cnt < 0)
        return -1;

    *i2c_addr = (addr << 1);
    while (((*i2c_status) & 1));

    if (!((*i2c_status) & 2))
        return -1;

    int i;
    for (i = 0; i < cnt; i++) {
        *i2c_data = buf[i];
        while (((*i2c_status) & 1));
    }

    return 0;
}

static int i2c_read(uint8_t addr, int cnt, uint8_t *buf)
{
    if (cnt < 1)
        return -1;

    *i2c_ctrl = (cnt == 1) ? 1 : 0;

    *i2c_addr = (addr << 1) | 1;
    while (((*i2c_status) & 1));

    if (((*i2c_status) & 0x10))
        return -1;

    int i;
    //for (i = cnt - 1; i >= 0; i--) {
    for (i = 0; cnt > 0; i++) {
        if (--cnt == 1)
            *i2c_ctrl = 1;

        buf[i] = *i2c_data;
        while (((*i2c_status) & 1));
    }

    return 0;
}

static void i2c_stop()
{
    *i2c_ctrl = 2;
    while (((*i2c_status) & 1));
}

int i2c_probe(uint8_t addr)
{
    if (i2c_write(addr, 0, NULL) < 0)
        return -1;

    i2c_stop();
    return 0;
}

int i2c_reg8_read_stream(uint8_t addr, uint8_t reg, uint8_t *buf, int buf_len)
{
    if (i2c_write(addr, 1, &reg) < 0)
        return -1;

    if (i2c_read(addr, buf_len, buf) < 0)
        return -1;

    return 0;
}

int i2c_reg8_read8(uint8_t addr, uint8_t reg)
{
    if (i2c_write(addr, 1, &reg) < 0)
        return -1;

    uint8_t data;
    if (i2c_read(addr, 1, &data) < 0)
        return -1;

    return data;
}

int i2c_reg8_read16l(uint8_t addr, uint8_t reg)
{
    if (i2c_write(addr, 1, &reg) < 0)
        return -1;

    uint8_t buf[2];
    if (i2c_read(addr, 2, buf) < 0)
        return -1;

    return buf[0] | (buf[1] << 8);
}

int i2c_reg8_read16b(uint8_t addr, uint8_t reg)
{
    if (i2c_write(addr, 1, &reg) < 0)
        return -1;

    uint8_t buf[2];
    if (i2c_read(addr, 2, buf) < 0)
        return -1;

    return (buf[0] << 8) | buf[1];
}

int i2c_reg8_read24l(uint8_t addr, uint8_t reg)
{
    if (i2c_write(addr, 1, &reg) < 0)
        return -1;

    uint8_t buf[3];
    if (i2c_read(addr, 3, buf) < 0)
        return -1;

    return buf[0] | (buf[1] << 8) | (buf[2] << 16);
}

int i2c_reg8_read24b(uint8_t addr, uint8_t reg)
{
    if (i2c_write(addr, 1, &reg) < 0)
        return -1;

    uint8_t buf[3];
    if (i2c_read(addr, 3, buf) < 0)
        return -1;

    return (buf[0] << 16) | (buf[1] << 8) | buf[2];
}

int i2c_reg16l_read16l(uint8_t addr, uint16_t reg)
{
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = reg >> 8;

    if (i2c_write(addr, 2, buf) < 0)
        return -1;

    if (i2c_read(addr, 2, buf) < 0)
        return -1;

    return buf[0] | (buf[1] << 8);
}

int i2c_reg16b_read16b(uint8_t addr, uint16_t reg)
{
    uint8_t buf[2];
    buf[0] = reg >> 8;
    buf[1] = reg;

    if (i2c_write(addr, 2, buf) < 0)
        return -1;

    if (i2c_read(addr, 2, buf) < 0)
        return -1;

    return (buf[0] << 8) | buf[1];
}

int i2c_reg8_write8(uint8_t addr, uint8_t reg, uint8_t data)
{
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = data;

    if (i2c_write(addr, 2, buf) < 0)
        return -1;

    i2c_stop();
    return 0;
}

int i2c_reg8_write16l(uint8_t addr, uint8_t reg, uint16_t data)
{
    uint8_t buf[3];
    buf[0] = reg;
    buf[1] = data;
    buf[2] = data >> 8;

    if (i2c_write(addr, 3, buf) < 0)
        return -1;

    i2c_stop();
    return 0;
}

int i2c_reg8_write16b(uint8_t addr, uint8_t reg, uint16_t data)
{
    uint8_t buf[3];
    buf[0] = reg;
    buf[1] = data >> 8;
    buf[2] = data;

    if (i2c_write(addr, 3, buf) < 0)
        return -1;

    i2c_stop();
    return 0;
}

int i2c_reg8_write24l(uint8_t addr, uint8_t reg, uint32_t data)
{
    uint8_t buf[4];
    buf[0] = reg;
    buf[1] = data;
    buf[2] = data >> 8;
    buf[3] = data >> 16;

    if (i2c_write(addr, 4, buf) < 0)
        return -1;

    i2c_stop();
    return 0;
}

int i2c_reg8_write24b(uint8_t addr, uint8_t reg, uint32_t data)
{
    uint8_t buf[4];
    buf[0] = reg;
    buf[1] = data >> 16;
    buf[2] = data >> 8;
    buf[3] = data;

    if (i2c_write(addr, 4, buf) < 0)
        return -1;

    i2c_stop();
    return 0;
}

int i2c_reg16l_write16l(uint8_t addr, uint16_t reg, uint16_t data)
{
    uint8_t buf[4];
    buf[0] = reg;
    buf[1] = reg >> 8;
    buf[2] = data;
    buf[3] = data >> 8;

    if (i2c_write(addr, 4, buf) < 0)
        return -1;

    i2c_stop();
    return 0;
}

int i2c_reg16b_write16b(uint8_t addr, uint16_t reg, uint16_t data)
{
    uint8_t buf[4];
    buf[0] = reg >> 8;
    buf[1] = reg;
    buf[2] = data >> 8;
    buf[3] = data;

    if (i2c_write(addr, 4, buf) < 0)
        return -1;

    i2c_stop();
    return 0;
}
