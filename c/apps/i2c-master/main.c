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
 * A short demo program to show how to use the I2C master module.
 *
 * Authors: Michael Platzer (TU Wien)
 */

#include <stdio.h>
#include <machine/patmos.h>
#include <machine/spm.h>

#include "i2c_master.h"

#define DEV_ADDR    0x48
#define DEV_REG     0x00

int main(void)
{
    // check whether the device exists (i.e. replies with an ACK)
    if (i2c_probe(DEV_ADDR) < 0) {

        // read a register from that device
        int data;
        data = i2c_reg8_read8(DEV_ADDR, DEV_REG);

        // do stuff with the data
        // ...
    }

    for(;;);
}
