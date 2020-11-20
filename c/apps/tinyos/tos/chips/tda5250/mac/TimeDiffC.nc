/* -*- mode:c++; indent-tabs-mode: nil -*-
 * Copyright (c) 2007, Technische Universitaet Berlin
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name of the Technische Universitaet Berlin nor the names
 *   of its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES {} LOSS OF USE, DATA,
 * OR PROFITS {} OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Implementation of timediff interface
 * @author: Andreas Koepke (koepke@tkn.tu-berlin.de)
 */

module TimeDiffC {
    provides {
        interface TimeDiff32;
        interface TimeDiff16;
    }
}
implementation {
    async command uint32_t TimeDiff32.computeDelta(uint32_t time_now, uint32_t time_past) {
        uint32_t rval;
        if(time_now >= time_past) {
            rval = time_now - time_past;
        }
        else {
            rval = (uint32_t)(-1) - time_past + time_now;
        }
        return rval;
    }

    async command uint16_t TimeDiff16.computeDelta(uint16_t time_now, uint16_t time_past) {
        uint16_t rval;
        if(time_now >= time_past) {
            rval = time_now - time_past;
        }
        else {
            rval = (uint16_t)(-1) - time_past + time_now;
        }
        return rval;
    }
}

