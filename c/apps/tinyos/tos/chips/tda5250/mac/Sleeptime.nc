/* -*- mode:c++; indent-tabs-mode: nil -*-
 * Copyright (c) 2006, Technische Universitaet Berlin
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
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
 
/**
 * Interface to control the duty cycle of the MAC
 * @author Andreas Koepke (koepke at tkn.tu-berlin.de)
 */ 
interface Sleeptime {
  /**
   * set the sleep time of the MAC in units of a 32kHz clock, the setting
   * takes effect on the next wakeup.
   * 
   * Caution 1: To avoid synchroninization of the wake up times, some
   *             additional randomization can be necessary, esp. when
   *             switching from shorter to longer sleep times.
   * Caution 2: The local sleep time must be equal or shorter than the
   *            network sleep time
   */
  async command void setLocalSleeptime(uint16_t sT);
  
  /**
   * which sleep time is in effect?
   */
  async command uint16_t getLocalSleeptime();

  /**
   * set the expected sleep time of the network -- this defines how long this
   * node will attempt to wake up a remote node.
   * Caution: The local sleep time must be equal or shorter than the
   *          network sleep time
   */
  async command void setNetworkSleeptime(uint16_t sT);
  
  /**
   * how long do we expect our neighbors to sleep?
   */
  async command uint16_t getNetworkSleeptime();
  
}
