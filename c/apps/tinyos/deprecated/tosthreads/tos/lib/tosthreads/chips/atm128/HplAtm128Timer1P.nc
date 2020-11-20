/// $Id: HplAtm128Timer1P.nc,v 1.2 2010-06-29 22:07:51 scipio Exp $

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

/**
 * Internal component of the HPL interface to Atmega128 timer 1.
 *
 * @author Martin Turon <mturon@xbow.com>
 */

#include <Atm128Timer.h>

module HplAtm128Timer1P
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
  uses interface HplAtm128TimerCtrl8     as Timer0Ctrl;
  uses interface PlatformInterrupt;
}
implementation
{
  //=== Read the current timer value. ===================================
  async command uint16_t Timer.get() { return TCNT1; }

  //=== Set/clear the current timer value. ==============================
  async command void Timer.set(uint16_t t) { TCNT1 = t; }

  //=== Read the current timer scale. ===================================
  async command uint8_t Timer.getScale() { return TCCR1B & 0x7; }

  //=== Turn off the timers. ============================================
  async command void Timer.off() { call Timer.setScale(AVR_CLOCK_OFF); }

  //=== Write a new timer scale. ========================================
  async command void Timer.setScale(uint8_t s)  { 
    Atm128TimerCtrlCapture_t x = call TimerCtrl.getCtrlCapture();
    x.bits.cs = s;
    call TimerCtrl.setCtrlCapture(x);  
  }

  //=== Read the control registers. =====================================
  async command Atm128TimerCtrlCompare_t TimerCtrl.getCtrlCompare() { 
    return *(Atm128TimerCtrlCompare_t*)&TCCR1A; 
  }
  async command Atm128TimerCtrlCapture_t TimerCtrl.getCtrlCapture() { 
    return *(Atm128TimerCtrlCapture_t*)&TCCR1B; 
  }
  async command Atm128TimerCtrlClock_t TimerCtrl.getCtrlClock() { 
    return *(Atm128TimerCtrlClock_t*)&TCCR1C; 
  }


  //=== Control registers utilities. ==================================
  DEFINE_UNION_CAST(TimerCtrlCompare2int, Atm128TimerCtrlCompare_t, uint16_t);
  DEFINE_UNION_CAST(TimerCtrlCapture2int, Atm128TimerCtrlCapture_t, uint16_t);
  DEFINE_UNION_CAST(TimerCtrlClock2int, Atm128TimerCtrlClock_t, uint16_t);

  //=== Write the control registers. ====================================
  async command void TimerCtrl.setCtrlCompare( Atm128_TCCR1A_t x ) { 
    TCCR1A = TimerCtrlCompare2int(x); 
  }
  async command void TimerCtrl.setCtrlCapture( Atm128_TCCR1B_t x ) { 
    TCCR1B = TimerCtrlCapture2int(x); 
  }
  async command void TimerCtrl.setCtrlClock( Atm128_TCCR1C_t x ) { 
    TCCR1C = TimerCtrlClock2int(x); 
  }

  //=== Read the interrupt mask. =====================================
  async command Atm128_ETIMSK_t TimerCtrl.getInterruptMask() { 
    return *(Atm128_ETIMSK_t*)&ETIMSK; 
  }

  //=== Write the interrupt mask. ====================================
  DEFINE_UNION_CAST(TimerMask8_2int, Atm128_TIMSK_t, uint8_t);
  DEFINE_UNION_CAST(TimerMask16_2int, Atm128_ETIMSK_t, uint8_t);

  async command void TimerCtrl.setInterruptMask( Atm128_ETIMSK_t x ) { 
    ETIMSK = TimerMask16_2int(x); 
  }

  //=== Read the interrupt flags. =====================================
  async command Atm128_ETIFR_t TimerCtrl.getInterruptFlag() { 
    return *(Atm128_ETIFR_t*)&ETIFR; 
  }

  //=== Write the interrupt flags. ====================================
  DEFINE_UNION_CAST(TimerFlags8_2int, Atm128_TIFR_t, uint8_t);
  DEFINE_UNION_CAST(TimerFlags16_2int, Atm128_ETIFR_t, uint8_t);

  async command void TimerCtrl.setInterruptFlag( Atm128_ETIFR_t x ) { 
    ETIFR = TimerFlags16_2int(x); 
  }

  //=== Capture 16-bit implementation. ===================================
  async command void Capture.setEdge(bool up) { WRITE_BIT(TCCR1B,ICES1, up); }

  //=== Timer 16-bit implementation. ===================================
  async command void Timer.reset()    { TIFR = 1 << TOV1; }
  async command void Capture.reset()  { TIFR = 1 << ICF1; }
  async command void CompareA.reset() { TIFR = 1 << OCF1A; }
  async command void CompareB.reset() { TIFR = 1 << OCF1B; }
  async command void CompareC.reset() { ETIFR = 1 << OCF1C; }

  async command void Timer.start()    { SET_BIT(TIMSK,TOIE1); }
  async command void Capture.start()  { SET_BIT(TIMSK,TICIE1); }
  async command void CompareA.start() { SET_BIT(TIMSK,OCIE1A); }
  async command void CompareB.start() { SET_BIT(TIMSK,OCIE1B); }
  async command void CompareC.start() { SET_BIT(ETIMSK,OCIE1C); }

  async command void Timer.stop()    { CLR_BIT(TIMSK,TOIE1); }
  async command void Capture.stop()  { CLR_BIT(TIMSK,TICIE1); }
  async command void CompareA.stop() { CLR_BIT(TIMSK,OCIE1A); }
  async command void CompareB.stop() { CLR_BIT(TIMSK,OCIE1B); }
  async command void CompareC.stop() { CLR_BIT(ETIMSK,OCIE1C); }

  // Note: Many Timer interrupt flags are on Timer0 register
  async command bool Timer.test() { 
    return (call Timer0Ctrl.getInterruptFlag()).bits.tov1; 
  }
  async command bool Capture.test()  { 
    return (call Timer0Ctrl.getInterruptFlag()).bits.icf1; 
  }
  async command bool CompareA.test() { 
    return (call Timer0Ctrl.getInterruptFlag()).bits.ocf1a; 
  }
  async command bool CompareB.test() { 
    return (call Timer0Ctrl.getInterruptFlag()).bits.ocf1b; 
  }
  async command bool CompareC.test() { 
    return (call TimerCtrl.getInterruptFlag()).bits.ocf1c; 
  }

  // Note: Many Timer interrupt mask bits are on Timer0 register
  async command bool Timer.isOn() {
    return (call Timer0Ctrl.getInterruptMask()).bits.toie1;
  }
  async command bool Capture.isOn()  {
    return (call Timer0Ctrl.getInterruptMask()).bits.ticie1;
  }
  async command bool CompareA.isOn() {
    return (call Timer0Ctrl.getInterruptMask()).bits.ocie1a;
  }
  async command bool CompareB.isOn() {
    return (call Timer0Ctrl.getInterruptMask()).bits.ocie1b;
  }
  async command bool CompareC.isOn() {
    return (call TimerCtrl.getInterruptMask()).bits.ocie1c;
  }

  //=== Read the compare registers. =====================================
  async command uint16_t CompareA.get() { return OCR1A; }
  async command uint16_t CompareB.get() { return OCR1B; }
  async command uint16_t CompareC.get() { return OCR1C; }

  //=== Write the compare registers. ====================================
  async command void CompareA.set(uint16_t t) { OCR1A = t; }
  async command void CompareB.set(uint16_t t) { OCR1B = t; }
  async command void CompareC.set(uint16_t t) { OCR1C = t; }

  //=== Read the capture registers. =====================================
  async command uint16_t Capture.get() { return ICR1; }

  //=== Write the capture registers. ====================================
  async command void Capture.set(uint16_t t)  { ICR1 = t; }

  //=== Timer interrupts signals ========================================
  default async event void CompareA.fired() { }
  AVR_NONATOMIC_HANDLER(SIG_OUTPUT_COMPARE1A) {
    signal CompareA.fired();
    call PlatformInterrupt.postAmble();
  }
  default async event void CompareB.fired() { }
  AVR_NONATOMIC_HANDLER(SIG_OUTPUT_COMPARE1B) {
    signal CompareB.fired();
    call PlatformInterrupt.postAmble();
  }
  default async event void CompareC.fired() { }
  AVR_NONATOMIC_HANDLER(SIG_OUTPUT_COMPARE1C) {
    signal CompareC.fired();
    call PlatformInterrupt.postAmble();
  }
  default async event void Capture.captured(uint16_t time) { }
  AVR_NONATOMIC_HANDLER(SIG_INPUT_CAPTURE1) {
    signal Capture.captured(call Timer.get());
    call PlatformInterrupt.postAmble();
  }
  default async event void Timer.overflow() { }
  AVR_NONATOMIC_HANDLER(SIG_OVERFLOW1) {
    signal Timer.overflow();
    call PlatformInterrupt.postAmble();
  }
}
