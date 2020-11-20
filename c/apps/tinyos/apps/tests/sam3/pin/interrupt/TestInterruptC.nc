/**
 * Copyright (c) 2009 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:  
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 * - Neither the name of the copyright holder nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @author Thomas Schmid
 **/

module TestInterruptC
{
	uses interface Leds;
	uses interface Boot;
    uses interface GpioInterrupt as GpioInterruptLeft;
    uses interface HplSam3GeneralIOPin as ButtonLeft;
    uses interface GpioInterrupt as GpioInterruptRight;
    uses interface HplSam3GeneralIOPin as ButtonRight;
}
implementation
{

	event void Boot.booted()
	{

        //call ButtonLeft.enablePioControl();
        //call ButtonRight.enablePioControl();

        // schematic demands that we pull up the pins!
        call ButtonLeft.enablePullUpResistor();
        call ButtonRight.enablePullUpResistor();

        call GpioInterruptLeft.enableFallingEdge();
        call GpioInterruptRight.enableFallingEdge();

	}

    async event void GpioInterruptLeft.fired()
    {
        call Leds.led0Toggle(); 
    }

    async event void GpioInterruptRight.fired()
    {
        call Leds.led1Toggle(); 
    }
}
