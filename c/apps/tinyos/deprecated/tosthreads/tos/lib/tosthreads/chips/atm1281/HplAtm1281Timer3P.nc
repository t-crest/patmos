
/*
 * Copyright (c) 2004-2005 Crossbow Technology, Inc.  All rights reserved.
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
 * - Neither the name of Crossbow Technology nor the names of
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
 * Copyright (c) 2007, Vanderbilt University
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
 */

/**
 * Internal component of the HPL interface to Atmega1281 timer 3.
 *
 * @author Martin Turon <mturon@xbow.com>
 * @author Janos Sallai <janos.sallai@vanderbilt.edu>
 */

#include <Atm128Timer.h>

module HplAtm1281Timer3P
{
  provides {
    // 16-bit Timers
    interface HplAtm128Timer<uint16_t>   as Timer;
    interface HplAtm128TimerCtrl16       as TimerCtrl;
    interface HplAtm128Capture<uint16_t> as Capture;
    interface HplAtm128Compare<uint16_t> as CompareA;
    interface HplAtm128Compare<uint16_t> as CompareB;
    interface HplAtm128Compare<uint16_t> as CompareC;
  }
  uses interface PlatformInterrupt;
}
implementation
{
  //=== Read the current timer value. ===================================
  async command uint16_t Timer.get() { return TCNT3; }

  //=== Set/clear the current timer value. ==============================
  async command void Timer.set(uint16_t t) { TCNT3 = t; }

  //=== Read the current timer scale. ===================================
  async command uint8_t Timer.getScale() { return TCCR3B & 0x7; }

  //=== Turn off the timers. ============================================
  async command void Timer.off() { call Timer.setScale(AVR_CLOCK_OFF); }

  //=== Write a new timer scale. ========================================
  async command void Timer.setScale(uint8_t s)  { 
    Atm128_TCCRB_t x = (Atm128_TCCRB_t) call TimerCtrl.getControlB();
    x.bits.cs = s;
    call TimerCtrl.setControlB(x.flat);  
  }

  //=== Read the control registers. =====================================
  async command uint8_t TimerCtrl.getControlA() { 
    return TCCR3A; 
  }

  async command uint8_t TimerCtrl.getControlB() { 
    return TCCR3B; 
  }

  async command uint8_t TimerCtrl.getControlC() { 
    return TCCR3C; 
  }

  //=== Write the control registers. ====================================
  async command void TimerCtrl.setControlA( uint8_t x ) { 
    TCCR3A = x; 
  }

  async command void TimerCtrl.setControlB( uint8_t x ) { 
    TCCR3B = x; 
  }

  async command void TimerCtrl.setControlC( uint8_t x ) { 
    TCCR3C = x; 
  }

  //=== Read the interrupt mask. =====================================
  async command uint8_t TimerCtrl.getInterruptMask() { 
    return TIMSK3; 
  }

  //=== Write the interrupt mask. ====================================
  async command void TimerCtrl.setInterruptMask( uint8_t x ) { 
    TIMSK3 = x; 
  }

  //=== Read the interrupt flags. =====================================
  async command uint8_t TimerCtrl.getInterruptFlag() { 
    return TIFR3; 
  }

  //=== Write the interrupt flags. ====================================
  async command void TimerCtrl.setInterruptFlag( uint8_t x ) { 
    TIFR3 = x; 
  }

  //=== Capture 16-bit implementation. ===================================
  async command void Capture.setEdge(bool up) { WRITE_BIT(TCCR3B, ICES3, up); }

  //=== Timer 16-bit implementation. ===================================
  async command void Timer.reset()    { TIFR3 = 1 << TOV3; }
  async command void Capture.reset()  { TIFR3 = 1 << ICF3; }
  async command void CompareA.reset() { TIFR3 = 1 << OCF3A; }
  async command void CompareB.reset() { TIFR3 = 1 << OCF3B; }
  async command void CompareC.reset() { TIFR3 = 1 << OCF3C; }

  async command void Timer.start()    { SET_BIT(TIMSK3,TOIE3); }
  async command void Capture.start()  { SET_BIT(TIMSK3,ICIE3); }
  async command void CompareA.start() { SET_BIT(TIMSK3,OCIE3A); }
  async command void CompareB.start() { SET_BIT(TIMSK3,OCIE3B); }
  async command void CompareC.start() { SET_BIT(TIMSK3,OCIE3C); }

  async command void Timer.stop()    { CLR_BIT(TIMSK3,TOIE3); }
  async command void Capture.stop()  { CLR_BIT(TIMSK3,ICIE3); }
  async command void CompareA.stop() { CLR_BIT(TIMSK3,OCIE3A); }
  async command void CompareB.stop() { CLR_BIT(TIMSK3,OCIE3B); }
  async command void CompareC.stop() { CLR_BIT(TIMSK3,OCIE3C); }

  async command bool Timer.test() { 
    return ((Atm128_TIFR_t)call TimerCtrl.getInterruptFlag()).bits.tov; 
  }
  async command bool Capture.test()  { 
    return ((Atm128_TIFR_t)call TimerCtrl.getInterruptFlag()).bits.icf; 
  }
  async command bool CompareA.test() { 
    return ((Atm128_TIFR_t)call TimerCtrl.getInterruptFlag()).bits.ocfa; 
  }
  async command bool CompareB.test() { 
    return ((Atm128_TIFR_t)call TimerCtrl.getInterruptFlag()).bits.ocfb; 
  }
  async command bool CompareC.test() { 
    return ((Atm128_TIFR_t)call TimerCtrl.getInterruptFlag()).bits.ocfc; 
  }

  async command bool Timer.isOn() {
    return ((Atm128_TIMSK_t)call TimerCtrl.getInterruptMask()).bits.toie;
  }
  async command bool Capture.isOn()  {
    return ((Atm128_TIMSK_t)call TimerCtrl.getInterruptMask()).bits.icie;
  }
  async command bool CompareA.isOn() {
    return ((Atm128_TIMSK_t)call TimerCtrl.getInterruptMask()).bits.ociea;
  }
  async command bool CompareB.isOn() {
    return ((Atm128_TIMSK_t)call TimerCtrl.getInterruptMask()).bits.ocieb;
  }
  async command bool CompareC.isOn() {
    return ((Atm128_TIMSK_t)call TimerCtrl.getInterruptMask()).bits.ociec;
  }

  //=== Read the compare registers. =====================================
  async command uint16_t CompareA.get() { return OCR3A; }
  async command uint16_t CompareB.get() { return OCR3B; }
  async command uint16_t CompareC.get() { return OCR3C; }

  //=== Write the compare registers. ====================================
  async command void CompareA.set(uint16_t t) { OCR3A = t; }
  async command void CompareB.set(uint16_t t) { OCR3B = t; }
  async command void CompareC.set(uint16_t t) { OCR3C = t; }

  //=== Read the capture registers. =====================================
  async command uint16_t Capture.get() { return ICR3; }

  //=== Write the capture registers. ====================================
  async command void Capture.set(uint16_t t)  { ICR3 = t; }

  //=== Timer interrupts signals ========================================
  default async event void CompareA.fired() { }
  AVR_NONATOMIC_HANDLER(SIG_OUTPUT_COMPARE3A) {
    signal CompareA.fired();
    call PlatformInterrupt.postAmble();
  }
  default async event void CompareB.fired() { }
  AVR_NONATOMIC_HANDLER(SIG_OUTPUT_COMPARE3B) {
    signal CompareB.fired();
    call PlatformInterrupt.postAmble();
  }
  default async event void CompareC.fired() { }
  AVR_NONATOMIC_HANDLER(SIG_OUTPUT_COMPARE3C) {
    signal CompareC.fired();
    call PlatformInterrupt.postAmble();
  }
  default async event void Capture.captured(uint16_t time) { }
  AVR_NONATOMIC_HANDLER(SIG_INPUT_CAPTURE3) {
    signal Capture.captured(call Timer.get());
    call PlatformInterrupt.postAmble();
  }
  default async event void Timer.overflow() { }
  AVR_NONATOMIC_HANDLER(SIG_OVERFLOW3) {
    signal Timer.overflow();
    call PlatformInterrupt.postAmble();
  }
}
