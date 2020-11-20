/*
 * Copyright (c) 2005 Arched Rock Corporation 
 * All rights reserved. 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *	Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *	Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *  
 *   Neither the name of the Arched Rock Corporation nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE ARCHED
 * ROCK OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */
/** 
 * This private component provides a 16-bit BusyWait interface
 * of a given precision over OS Timer channel 0
 *
 * @param precision_tag A type tag mapped to the set precision
 *
 * @param val4xScale A value to scale the underlying counter by. 
 *   The passed in parameter is given by the equation
 *   val4xScale = (3.25 MHz/<desired_precision_in_Hz>) * 4
 *   and rounded to the nearest integer.
 *   Example: Counter precision of 32.768 kHz would have 
 *   a val4xScale of 397
 * 
 * @author Phil Buonadonna
 *
 */

generic module HalPXA27xBusyWaitM(typedef precision_tag, uint16_t val4xScale)
{
  provides interface BusyWait<precision_tag,uint16_t>;
  uses interface HplPXA27xOSTimer as OST;
}

implementation
{

  async command void BusyWait.wait(uint16_t dt) {
    uint32_t dCounts;
    atomic {
      uint32_t t0 = call OST.getOSCR();
      dCounts = (dt * 4) * val4xScale;
      dCounts >>= 2;
      while (((call OST.getOSCR()) - t0) < dCounts);
    }
  }

  async event void OST.fired() {
    return;
  }

}
