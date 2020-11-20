/*
 * Copyright (c) 2005 Stanford University. All rights reserved.
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

/**
 * The TOSSIM implementation of the Atm128 Timer0. It is built from a
 * timer-specific counter component and a generic compare
 * component. The counter component has an additional simulation-only
 * interface to let the compare component know when its state has
 * changed (e.g., TCNTX was set).
 *
 * @date November 22 2005
 *
 * @author Philip Levis <pal@cs.stanford.edu>
 * @author Martin Turon <mturon@xbow.com>
 * @author David Gay <dgay@intel-research.net>
 */

// $Id: HplAtm128Timer0AsyncC.nc,v 1.2 2010-06-29 22:07:51 scipio Exp $/// $Id: HplAtm128Timer2C.nc,

#include <Atm128Timer.h>

configuration HplAtm128Timer0AsyncC
{
  provides {
    // 8-bit Timers
    interface HplAtm128Timer<uint8_t>   as Timer;
    interface HplAtm128TimerCtrl8       as TimerCtrl;
    interface HplAtm128Compare<uint8_t> as Compare;
    interface HplAtm128TimerAsync       as TimerAsync;
  }
}
implementation {
  components HplAtm128Timer0AsyncP;
  Timer = HplAtm128Timer0AsyncP;
  TimerCtrl = HplAtm128Timer0AsyncP;
  Compare = HplAtm128Timer0AsyncP;
  TimerAsync = HplAtm128Timer0AsyncP;
  
  components TinyThreadSchedulerC;
  HplAtm128Timer0AsyncP.ThreadScheduler -> TinyThreadSchedulerC;
}
