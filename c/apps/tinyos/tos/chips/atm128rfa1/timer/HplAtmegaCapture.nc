/*
 * Copyright (c) 2010, University of Szeged
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
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: Miklos Maroti
 */

interface HplAtmegaCapture<size_type>
{
// ----- input capture register (ICR) 

	/* Returns the current captured value */
	async command size_type get();

	/* Sets the current captured value */
	async command void set(size_type value);

// ----- timer interrupt flag register (TIFR), input capture flag (ICF)

	/* Signalled when the captured event has occured */
	async event void fired();

	/* Tests if there is a pending captured event */
	async command bool test();

	/* Resets a pending interrupt */
	async command void reset();

// ----- timer interrupt mask register (TIMSK), input capture interrupt enable (ICIE)

	/* Enables the capture interrupt */
	async command void start();

	/* Disables the capture interrupt */
	async command void stop();

	/* Checks is the overflow interrupt is enabled */
	async command bool isOn();

// ----- timer control register (TCCR), input capture mode (ICNC and ICES)

	/* Sets the input capture mode bits */
	async command void setMode(uint8_t mode);

	/* Returns the input capture mode bits */
	async command uint8_t getMode();
}
