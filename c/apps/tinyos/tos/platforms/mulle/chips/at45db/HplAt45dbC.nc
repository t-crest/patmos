/*
 * Copyright (c) 2011 Lulea University of Technology
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
 */

/**
 * Mulle specific wiring for the AT45DB161D Flash storage.
 *
 * @author Henrik Makitaavola <henrik.makitaavola@gmail.com>
 */

configuration HplAt45dbC
{
  provides interface HplAt45db;
}
implementation
{
  components  new HplAt45dbByteC(10),
              new SoftwareSpiC() as Spi,
              new NoArbiterC(),
              HplAt45dbP,
              HplM16c60GeneralIOC as IOs,
              RealMainP,
              BusyWaitMicroC;

  Spi.MISO -> IOs.PortP40;
  Spi.MOSI -> IOs.PortP41;
  Spi.SCLK -> IOs.PortP42;

  HplAt45db = HplAt45dbByteC;

  HplAt45dbByteC.Resource -> NoArbiterC;
  HplAt45dbByteC.FlashSpi -> Spi;
  HplAt45dbByteC.HplAt45dbByte -> HplAt45dbP;

  HplAt45dbP.VCC -> IOs.PortP32;
  HplAt45dbP.WP -> IOs.PortP44;
  HplAt45dbP.Select -> IOs.PortP45;
  HplAt45dbP.RESET -> IOs.PortP46;
  HplAt45dbP.MISO -> IOs.PortP40;
  HplAt45dbP.MOSI -> IOs.PortP41;
  HplAt45dbP.SCLK -> IOs.PortP42;

  HplAt45dbP.FlashSpi -> Spi;
  HplAt45dbP.BusyWait -> BusyWaitMicroC;
  RealMainP.SoftwareInit -> HplAt45dbP.Init;
}
