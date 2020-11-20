/*                                     
 * Copyright (c) 2000-2003 The Regents of the University  of California.  
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
 * Copyright (c) 2002-2003 Intel Corporation
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE     
 * file. If you do not find these files, copies can be found by writing to
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300, Berkeley, CA, 
 * 94704.  Attention:  Intel License Inquiry.
 *
 * Date last modified:  $Id: MainC.nc,v 1.4 2010-06-29 22:07:52 scipio Exp $
 */

/**
 * MainC is the system interface the TinyOS boot sequence. It wires the
 * boot sequence implementation to the scheduler and hardware resources.
 *
 * @author Philip Levis
 * @author Kevin Klues <klueska@cs.stanford.edu>
 */

#include "hardware.h"

configuration MainC {
  provides interface Boot;
  uses {
    interface Init as SoftwareInit;
  }
}
implementation {
  components PlatformC;
  components TinyOSMainP;
  components RealMainP;
  
  components TinyTaskSchedulerC;
  components TinyThreadSchedulerC;
  components StaticThreadC;
    
#ifdef SAFE_TINYOS
  components SafeFailureHandlerC;
#endif

  // Export the SoftwareInit and Boot for applications
  SoftwareInit = TinyOSMainP.SoftwareInit;
  Boot = TinyOSMainP;
  
  //Wire up the platform specific code
  TinyOSMainP.PlatformInit -> PlatformC;
  TinyOSMainP.TaskScheduler -> TinyTaskSchedulerC;
  
  //Wire up the interdependent task and thread schedulers
  TinyTaskSchedulerC.ThreadScheduler -> TinyThreadSchedulerC;
  
  //Wire up the TinyOS code to its thread
  StaticThreadC.ThreadInfo[TOSTHREAD_TOS_THREAD_ID] -> TinyOSMainP;
  TinyOSMainP.TinyOSBoot -> TinyThreadSchedulerC;
  
  //Wire up the thread scheduler to start running
  TinyThreadSchedulerC.ThreadSchedulerBoot -> RealMainP;  
}

