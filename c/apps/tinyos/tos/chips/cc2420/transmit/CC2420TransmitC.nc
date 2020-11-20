/*
 * Copyright (c) 2005-2006 Arch Rock Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 * - Neither the name of the Arch Rock Corporation nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * ARCHED ROCK OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE
 */

/**
 * Implementation of the transmit path for the ChipCon CC2420 radio.
 *
 * @author Jonathan Hui <jhui@archrock.com>
 * @version $Revision: 1.3 $ $Date: 2009-08-14 20:33:43 $
 */

#include "IEEE802154.h"

configuration CC2420TransmitC {

  provides {
    interface StdControl;
    interface CC2420Transmit;
    interface RadioBackoff;
    interface ReceiveIndicator as EnergyIndicator;
    interface ReceiveIndicator as ByteIndicator;
  }
}

implementation {

  components CC2420TransmitP;
  StdControl = CC2420TransmitP;
  CC2420Transmit = CC2420TransmitP;
  RadioBackoff = CC2420TransmitP;
  EnergyIndicator = CC2420TransmitP.EnergyIndicator;
  ByteIndicator = CC2420TransmitP.ByteIndicator;

  components MainC;
  MainC.SoftwareInit -> CC2420TransmitP;
  MainC.SoftwareInit -> Alarm;
  
  components AlarmMultiplexC as Alarm;
  CC2420TransmitP.BackoffTimer -> Alarm;

  components HplCC2420PinsC as Pins;
  CC2420TransmitP.CCA -> Pins.CCA;
  CC2420TransmitP.CSN -> Pins.CSN;
  CC2420TransmitP.SFD -> Pins.SFD;

  components HplCC2420InterruptsC as Interrupts;
  CC2420TransmitP.CaptureSFD -> Interrupts.CaptureSFD;

  components new CC2420SpiC() as Spi;
  CC2420TransmitP.SpiResource -> Spi;
  CC2420TransmitP.ChipSpiResource -> Spi;
  CC2420TransmitP.SNOP        -> Spi.SNOP;
  CC2420TransmitP.STXON       -> Spi.STXON;
  CC2420TransmitP.STXONCCA    -> Spi.STXONCCA;
  CC2420TransmitP.SFLUSHTX    -> Spi.SFLUSHTX;
  CC2420TransmitP.TXCTRL      -> Spi.TXCTRL;
  CC2420TransmitP.TXFIFO      -> Spi.TXFIFO;
  CC2420TransmitP.TXFIFO_RAM  -> Spi.TXFIFO_RAM;
  CC2420TransmitP.MDMCTRL1    -> Spi.MDMCTRL1;
  CC2420TransmitP.SECCTRL0 -> Spi.SECCTRL0;
  CC2420TransmitP.SECCTRL1 -> Spi.SECCTRL1;
  CC2420TransmitP.STXENC -> Spi.STXENC;
  CC2420TransmitP.TXNONCE -> Spi.TXNONCE;
  CC2420TransmitP.KEY0 -> Spi.KEY0;
  CC2420TransmitP.KEY1 -> Spi.KEY1;
  
  components CC2420ReceiveC;
  CC2420TransmitP.CC2420Receive -> CC2420ReceiveC;
  
  components CC2420PacketC;
  CC2420TransmitP.CC2420Packet -> CC2420PacketC;
  CC2420TransmitP.CC2420PacketBody -> CC2420PacketC;
  CC2420TransmitP.PacketTimeStamp -> CC2420PacketC;
  CC2420TransmitP.PacketTimeSyncOffset -> CC2420PacketC;

  components LedsC;
  CC2420TransmitP.Leds -> LedsC;

}
