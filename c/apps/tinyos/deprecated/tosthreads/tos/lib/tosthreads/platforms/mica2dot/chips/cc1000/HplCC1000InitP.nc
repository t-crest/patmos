// $Id: HplCC1000InitP.nc,v 1.2 2010-06-29 22:07:52 scipio Exp $
/*
 * Copyright (c) 2004-2005 The Regents of the University  of California.  
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
 * - Neither the name of the University of California nor the names of
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
 * Copyright (c) 2004-2005 Intel Corporation
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE     
 * file. If you do not find these files, copies can be found by writing to
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300, Berkeley, CA, 
 * 94704.  Attention:  Intel License Inquiry.
 */
/**
 * Hardware initialisation for the CC1000 radio. This component is always
 * included even if the radio is not used.
 *
 * @author David Gay
 */
configuration HplCC1000InitP {
  provides interface Init as PlatformInit;
}
implementation {
  components HplCC1000P, HplCC1000SpiP, HplAtm128GeneralIOC as IO;

  PlatformInit = HplCC1000P;
  PlatformInit = HplCC1000SpiP;

  HplCC1000P.CHP_OUT -> IO.PortE7;
  HplCC1000P.PALE -> IO.PortD5;
  HplCC1000P.PCLK -> IO.PortD6;
  HplCC1000P.PDATA -> IO.PortD7;

  HplCC1000SpiP.SpiSck -> IO.PortB1;
  HplCC1000SpiP.SpiMiso -> IO.PortB3;
  HplCC1000SpiP.SpiMosi -> IO.PortB2;
  HplCC1000SpiP.OC1C -> IO.PortB7;

  components PlatformInterruptC;
  HplCC1000SpiP.PlatformInterrupt -> PlatformInterruptC;
}
