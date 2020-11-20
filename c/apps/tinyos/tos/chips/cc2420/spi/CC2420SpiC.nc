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
 * Implementation of basic SPI primitives for the ChipCon CC2420 radio.
 *
 * @author Jonathan Hui <jhui@archrock.com>
 * @version $Revision: 1.4 $ $Date: 2009-08-14 20:33:43 $
 */

generic configuration CC2420SpiC() {

  provides interface Resource;
  provides interface ChipSpiResource;

  // commands
  provides interface CC2420Strobe as SNOP;
  provides interface CC2420Strobe as SXOSCON;
  provides interface CC2420Strobe as STXCAL;
  provides interface CC2420Strobe as SRXON;
  provides interface CC2420Strobe as STXON;
  provides interface CC2420Strobe as STXONCCA;
  provides interface CC2420Strobe as SRFOFF;
  provides interface CC2420Strobe as SXOSCOFF;
  provides interface CC2420Strobe as SFLUSHRX;
  provides interface CC2420Strobe as SFLUSHTX;
  provides interface CC2420Strobe as SACK;
  provides interface CC2420Strobe as SACKPEND;
  provides interface CC2420Strobe as SRXDEC;
  provides interface CC2420Strobe as STXENC;
  provides interface CC2420Strobe as SAES;

  // registers
  provides interface CC2420Register as MAIN;
  provides interface CC2420Register as MDMCTRL0;
  provides interface CC2420Register as MDMCTRL1;
  provides interface CC2420Register as RSSI;
  provides interface CC2420Register as SYNCWORD;
  provides interface CC2420Register as TXCTRL;
  provides interface CC2420Register as RXCTRL0;
  provides interface CC2420Register as RXCTRL1;
  provides interface CC2420Register as FSCTRL;
  provides interface CC2420Register as SECCTRL0;
  provides interface CC2420Register as SECCTRL1;
  provides interface CC2420Register as BATTMON;
  provides interface CC2420Register as IOCFG0;
  provides interface CC2420Register as IOCFG1;
  provides interface CC2420Register as MANFIDL;
  provides interface CC2420Register as MANFIDH;
  provides interface CC2420Register as FSMTC;
  provides interface CC2420Register as MANAND;
  provides interface CC2420Register as MANOR;
  provides interface CC2420Register as AGCCTRL;

  provides interface CC2420Register as AGCTST0;
  provides interface CC2420Register as AGCTST1;
  provides interface CC2420Register as AGCTST2;
  provides interface CC2420Register as FSTST0;
  provides interface CC2420Register as FSTST1;
  provides interface CC2420Register as FSTST2;
  provides interface CC2420Register as FSTST3;
  provides interface CC2420Register as RXBPFTST;
  provides interface CC2420Register as FSMSTATE;
  provides interface CC2420Register as ADCTST;
  provides interface CC2420Register as DACTST;
  provides interface CC2420Register as TOPTST;
  provides interface CC2420Register as TXFIFO_REGISTER;
  provides interface CC2420Register as RXFIFO_REGISTER;

  // ram
  provides interface CC2420Ram as IEEEADR;
  provides interface CC2420Ram as PANID;
  provides interface CC2420Ram as SHORTADR;
  provides interface CC2420Ram as TXFIFO_RAM;
  provides interface CC2420Ram as RXFIFO_RAM;
  provides interface CC2420Ram as KEY0;
  provides interface CC2420Ram as KEY1;
  provides interface CC2420Ram as SABUF;
  provides interface CC2420Ram as TXNONCE;
  provides interface CC2420Ram as RXNONCE;

  // fifos
  provides interface CC2420Fifo as RXFIFO;
  provides interface CC2420Fifo as TXFIFO;

}

implementation {

  enum {
    CLIENT_ID = unique( "CC2420Spi.Resource" ),
  };
  
  components HplCC2420PinsC as Pins;
  components CC2420SpiWireC as Spi;
  
  ChipSpiResource = Spi.ChipSpiResource;
  Resource = Spi.Resource[ CLIENT_ID ];
  
  // commands
  SNOP = Spi.Strobe[ CC2420_SNOP ];
  SXOSCON = Spi.Strobe[ CC2420_SXOSCON ];
  STXCAL = Spi.Strobe[ CC2420_STXCAL ];
  SRXON = Spi.Strobe[ CC2420_SRXON ];
  STXON = Spi.Strobe[ CC2420_STXON ];
  STXONCCA = Spi.Strobe[ CC2420_STXONCCA ];
  SRFOFF = Spi.Strobe[ CC2420_SRFOFF ];
  SXOSCOFF = Spi.Strobe[ CC2420_SXOSCOFF ];
  SFLUSHRX = Spi.Strobe[ CC2420_SFLUSHRX ];
  SFLUSHTX = Spi.Strobe[ CC2420_SFLUSHTX ];
  SACK = Spi.Strobe[ CC2420_SACK ];
  SACKPEND = Spi.Strobe[ CC2420_SACKPEND ];
  SRXDEC = Spi.Strobe[ CC2420_SRXDEC ];
  STXENC = Spi.Strobe[ CC2420_STXENC ];
  SAES = Spi.Strobe[ CC2420_SAES ];
  
  // registers
  MAIN = Spi.Reg[ CC2420_MAIN ];
  MDMCTRL0 = Spi.Reg[ CC2420_MDMCTRL0 ];
  MDMCTRL1 = Spi.Reg[ CC2420_MDMCTRL1 ];
  RSSI = Spi.Reg[ CC2420_RSSI ];
  SYNCWORD = Spi.Reg[ CC2420_SYNCWORD ];
  TXCTRL = Spi.Reg[ CC2420_TXCTRL ];
  RXCTRL0 = Spi.Reg[ CC2420_RXCTRL0 ];
  RXCTRL1 = Spi.Reg[ CC2420_RXCTRL1 ];
  FSCTRL = Spi.Reg[ CC2420_FSCTRL ];
  SECCTRL0 = Spi.Reg[ CC2420_SECCTRL0 ];
  SECCTRL1 = Spi.Reg[ CC2420_SECCTRL1 ];
  BATTMON = Spi.Reg[ CC2420_BATTMON ];
  IOCFG0 = Spi.Reg[ CC2420_IOCFG0 ];
  IOCFG1 = Spi.Reg[ CC2420_IOCFG1 ];
  MANFIDL = Spi.Reg[ CC2420_MANFIDL ];
  MANFIDH = Spi.Reg[ CC2420_MANFIDH ];
  FSMTC = Spi.Reg[ CC2420_FSMTC ];
  MANAND = Spi.Reg[ CC2420_MANAND ];
  MANOR = Spi.Reg[ CC2420_MANOR ];
  AGCCTRL = Spi.Reg[ CC2420_AGCCTRL ];

  AGCTST0	  = Spi.Reg[ CC2420_AGCTST0 ];
  AGCTST1	  = Spi.Reg[ CC2420_AGCTST1 ];
  AGCTST2	  = Spi.Reg[ CC2420_AGCTST2 ];
  FSTST0	  = Spi.Reg[ CC2420_FSTST0 ];
  FSTST1	  = Spi.Reg[ CC2420_FSTST1 ];
  FSTST2	  = Spi.Reg[ CC2420_FSTST2 ];
  FSTST3	  = Spi.Reg[ CC2420_FSTST3 ];
  RXBPFTST	  = Spi.Reg[ CC2420_RXBPFTST ];
  FSMSTATE	  = Spi.Reg[ CC2420_FSMSTATE ];
  ADCTST	  = Spi.Reg[ CC2420_ADCTST ];
  DACTST	  = Spi.Reg[ CC2420_DACTST ];
  TOPTST	  = Spi.Reg[ CC2420_TOPTST ];
  TXFIFO_REGISTER = Spi.Reg[ CC2420_TXFIFO ];
  RXFIFO_REGISTER = Spi.Reg[ CC2420_RXFIFO ];
  
  // ram
  IEEEADR = Spi.Ram[ CC2420_RAM_IEEEADR ];
  PANID = Spi.Ram[ CC2420_RAM_PANID ];
  SHORTADR = Spi.Ram[ CC2420_RAM_SHORTADR ];
  TXFIFO_RAM = Spi.Ram[ CC2420_RAM_TXFIFO ];
  RXFIFO_RAM = Spi.Ram[ CC2420_RAM_RXFIFO ];
  KEY0 = Spi.Ram[ CC2420_RAM_KEY0 ];
  KEY1 = Spi.Ram[ CC2420_RAM_KEY1 ];
  SABUF = Spi.Ram[ CC2420_RAM_SABUF ];
  TXNONCE = Spi.Ram[ CC2420_RAM_TXNONCE ];
  RXNONCE = Spi.Ram[ CC2420_RAM_RXNONCE ];

  // fifos
  RXFIFO = Spi.Fifo[ CC2420_RXFIFO ];
  TXFIFO = Spi.Fifo[ CC2420_TXFIFO ];

}

