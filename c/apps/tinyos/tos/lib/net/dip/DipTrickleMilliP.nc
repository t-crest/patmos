// $Id: DipTrickleMilliP.nc,v 1.2 2010-06-29 22:07:49 scipio Exp $
/*
 * Copyright (c) 2006 Stanford University. All rights reserved.
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
 */

/*
 * Module that provides a service instance of trickle timers. For
 * details on the working of the parameters, please refer to Levis et
 * al., "A Self-Regulating Algorithm for Code Maintenance and
 * Propagation in Wireless Sensor Networks," NSDI 2004.
 *
 * @param l Lower bound of the time period in seconds.
 * @param h Upper bound of the time period in seconds.
 * @param k Redundancy constant.
 * @param count How many timers to provide.
 *
 * @author Philip Levis
 * @author Gilman Tolle
 * @date   Jan 7 2006
 */ 

#include <Timer.h>
#include <Dip.h>

module DipTrickleMilliP {
  provides {
    interface Init;
    interface DipTrickleTimer as TrickleTimer;
  }
  uses {
    interface Timer<TMilli> as PeriodicIntervalTimer;
    interface Timer<TMilli> as SingleEventTimer;
    interface Random;
    interface Leds;
  }
}
implementation {

  uint32_t period;

  command error_t Init.init() {
    period = DIP_TAU_HIGH;
    return SUCCESS;
  }

  /**
   * Start a trickle timer. Reset the counter to 0.
   */
  command error_t TrickleTimer.start() {
    call PeriodicIntervalTimer.startOneShot(period);
    dbg("DipTrickleMilliP",
	"Starting trickle timer @ %s\n", sim_time_string());
    return SUCCESS;
  }

  /**
   * Stop the trickle timer. This call sets the timer period to H.
   */
  command void TrickleTimer.stop() {
    call PeriodicIntervalTimer.stop();
    dbg("DipTrickleMilliP",
	"Stopping trickle timer @ %s\n", sim_time_string());
  }

  /**
   * Reset the timer period to L. If called while the timer is running,
   * then a new interval (of length L) begins immediately.
   */
  command void TrickleTimer.reset() {
    period = DIP_TAU_LOW;
    call PeriodicIntervalTimer.stop();
    call PeriodicIntervalTimer.startOneShot(period);
    dbg("DipTrickleMilliP",
	"Resetting trickle timer @ %s\n", sim_time_string());
  }

  command void TrickleTimer.maxInterval() {
    period = DIP_TAU_HIGH;
  }

  /**
   * The trickle timer has fired. Signaled if C &gt; K.
   */
  event void PeriodicIntervalTimer.fired() {
    uint32_t dtfire;

    dtfire = (call Random.rand16() % (period / 2)) + (period / 2);
    dbg("DipTrickleMilliP", "Scheduling Trickle event with %u\n", dtfire);
    call SingleEventTimer.startOneShot(dtfire);
    period = signal TrickleTimer.requestWindowSize();
    call PeriodicIntervalTimer.startOneShot(period);
    //call Leds.led0Toggle();
  }

  event void SingleEventTimer.fired() {
    dbg("Trickle", "Firing Trickle Event Timer\n");
    signal TrickleTimer.fired();
  }
}

  
