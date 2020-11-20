/* -*- mode:c++; indent-tabs-mode: nil -*-
 * Copyright (c) 2007, Technische Universitaet Berlin
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
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES {} LOSS OF USE, DATA,
 * OR PROFITS {} OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Configuration for the Coulomb Counter board interface. This one is for
 * eyesIFXv2.1
 *
 * @author: Andreas Koepke (koepke@tkn.tu-berlin.de
 */

configuration CoulombCounterC {
    provides interface CoulombCounter;
}
implementation {
    components MainC, CoulombCounterP;
    components new TimerMilliC() as Timer;

    MainC.SoftwareInit -> CoulombCounterP;
    CoulombCounterP.Timer -> Timer;

    CoulombCounter = CoulombCounterP;
    
    components HplMsp430InterruptC, HplMsp430GeneralIOC;
    
    /** configure coulomb counter notification pin */
    components new Msp430InterruptC() as EnergyInterruptC,
        new Msp430GpioC() as EnergyPinC;

    EnergyInterruptC.HplInterrupt -> HplMsp430InterruptC.Port12;
    CoulombCounterP.EnergyInterrupt -> EnergyInterruptC.Interrupt;
    
    EnergyPinC -> HplMsp430GeneralIOC.Port12;
    CoulombCounterP.EnergyPin -> EnergyPinC;

    /** board control interface */
    components new Msp430GpioC() as UsbBatSwitch;
    components new Msp430GpioC() as ResetBoard;

    UsbBatSwitch -> HplMsp430GeneralIOC.Port64;
    ResetBoard -> HplMsp430GeneralIOC.Port65;

    CoulombCounterP.UsbBatSwitch -> UsbBatSwitch;
    CoulombCounterP.ResetBoard -> ResetBoard;

    /** Are we powered via USB? */
    components new Msp430GpioC() as UsbMonitor;
    UsbMonitor -> HplMsp430GeneralIOC.Port13;
    CoulombCounterP.UsbMonitor -> UsbMonitor;
}
