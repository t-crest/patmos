//$Id: Alarm.nc,v 1.5 2010-06-29 22:07:50 scipio Exp $

/* Copyright (c) 2000-2003 The Regents of the University of California.  
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
 */

#include "Timer.h"

/**
 * An Alarm is a low-level interface intended for precise timing.
 *
 * <p>An Alarm is parameterised by its "precision" (milliseconds,
 * microseconds, etc), identified by a type. This prevents, e.g.,
 * unintentionally mixing components expecting milliseconds with those
 * expecting microseconds as those interfaces have a different type.
 *
 * <p>An Alarm's second parameter is its "width", i.e., the number of
 * bits used to represent time values. Width is indicated by including
 * the appropriate size integer type as an Alarm parameter.
 *
 * <p>See TEP102 for more details.
 *
 * @param precision_tag A type indicating the precision of this Alarm.
 * @param size_type An integer type representing time values for this Alarm.
 *
 * @author Cory Sharp <cssharp@eecs.berkeley.edu>
 */

interface Alarm<precision_tag, size_type>
{
  // basic interface
  /**
   * Set a single-short alarm to some time units in the future. Replaces
   * any current alarm time. Equivalent to start(getNow(), dt). The
   * <code>fired</code> will be signaled when the alarm expires.
   *
   * @param dt Time until the alarm fires.
   */
  async command void start(size_type dt);

  /**
   * Cancel an alarm. Note that the <code>fired</code> event may have
   * already been signaled (even if your code has not yet started
   * executing).
   */
  async command void stop();

  /**
   * Signaled when the alarm expires.
   */
  async event void fired();

  // extended interface
  /**
   * Check if alarm is running. Note that a FALSE return does not indicate
   * that the <code>fired</code> event will not be signaled (it may have
   * already started executing, but not reached your code yet).
   *
   * @return TRUE if the alarm is still running.
   */
  async command bool isRunning();

  /**
   * Set a single-short alarm to time t0+dt. Replaces any current alarm
   * time. The <code>fired</code> will be signaled when the alarm expires.
   * Alarms set in the past will fire "soon".
   * 
   * <p>Because the current time may wrap around, it is possible to use
   * values of t0 greater than the <code>getNow</code>'s result. These
   * values represent times in the past, i.e., the time at which getNow()
   * would last of returned that value.
   *
   * @param t0 Base time for alarm.
   * @param dt Alarm time as offset from t0.
   */
  async command void startAt(size_type t0, size_type dt);

  /**
   * Return the current time.
   * @return Current time.
   */
  async command size_type getNow();

  /**
   * Return the time the currently running alarm will fire or the time that
   * the previously running alarm was set to fire.
   * @return Alarm time.
   */
  async command size_type getAlarm();
}

