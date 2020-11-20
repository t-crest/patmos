/*
 * Copyright (c) 2016 Eric B. Decker
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of the copyright holders nor the names of
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
 * @author Eric B. Decker <cire831@gmail.com>
 *
 * Msp432Alarm is a generic component that wraps the msp432 HPL timers and
 * compares into a TinyOS Alarm.
 */

generic module Msp432AlarmC(typedef frequency_tag) {
  provides interface Init;
  provides interface Alarm<frequency_tag, uint16_t> as Alarm;
  uses interface Msp432Timer;
  uses interface Msp432TimerCCTL;
  uses interface Msp432TimerCompare;
}
implementation {
  command error_t Init.init() {
    call Msp432TimerCCTL.disableEvents();
    call Msp432TimerCCTL.setCCRforCompare();
    return SUCCESS;
  }

  async command void Alarm.start( uint16_t dt ) {
    call Alarm.startAt(call Alarm.getNow(), dt);
  }

  async command void Alarm.stop() {
    call Msp432TimerCCTL.disableEvents();
  }

  async event void Msp432TimerCompare.fired() {
    call Msp432TimerCCTL.disableEvents();
    signal Alarm.fired();
  }

  async command bool Alarm.isRunning() {
    return call Msp432TimerCCTL.areEventsEnabled();
  }

  async command void Alarm.startAt(uint16_t t0, uint16_t dt) {
    atomic {
      /*
       * probably should stop the underlying h/w first
       */
      uint16_t now = call Msp432Timer.get();
      uint16_t elapsed = now - t0;

      if (elapsed >= dt) {
        call Msp432TimerCompare.setEventFromNow(2);
      } else {
        uint16_t remaining = dt - elapsed;
        if( remaining <= 2 )
          call Msp432TimerCompare.setEventFromNow(2);
        else
          call Msp432TimerCompare.setEvent(now + remaining );
      }
      /*
       * Normally, TAx->CCTL[n].CCIFG autoclears (reading IV).  However
       * we may have been dormant for awhile and if the CCR matched
       * R IFG will be up.  Starting a new Alarm interval so need to
       * clear the IFG out.
       */
      call Msp432TimerCCTL.clearPendingInterrupt();
      call Msp432TimerCCTL.enableEvents();
    }
  }

  async command uint16_t Alarm.getNow() {
    return call Msp432Timer.get();
  }

  async command uint16_t Alarm.getAlarm() {
    return call Msp432TimerCompare.getEvent();
  }

  async event void Msp432Timer.overflow() { }
}
