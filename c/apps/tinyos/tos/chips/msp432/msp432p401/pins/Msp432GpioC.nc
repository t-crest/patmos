/*
 * Copyright (c) 2016 Eric B. Decker
 * Copyright (c) 2000-2003 The Regents of the University of California.  
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

/**
 * Implementation of the general-purpose I/O abstraction
 * for the TI MSP432 microcontroller.
 *
 * @author Eric B. Decker
 */

generic module Msp432GpioC() @safe() {
  provides interface GeneralIO;
  uses interface HplMsp432Gpio as HplGpio;
}
implementation {
  async command void GeneralIO.set()        { call HplGpio.set(); }
  async command void GeneralIO.clr()        { call HplGpio.clr(); }
  async command void GeneralIO.toggle()     { call HplGpio.toggle(); }
  async command bool GeneralIO.get()        { return call HplGpio.get(); }
  async command void GeneralIO.makeInput()  { call HplGpio.makeInput(); }
  async command bool GeneralIO.isInput()    { return call HplGpio.isInput(); }
  async command void GeneralIO.makeOutput() { call HplGpio.makeOutput(); }
  async command bool GeneralIO.isOutput()   { return call HplGpio.isOutput(); }
}
