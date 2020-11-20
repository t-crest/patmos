/*
 * Copyright (c) 2008 Stanford University.
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
 * - Neither the name of the Stanford University nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL STANFORD
 * UNIVERSITY OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
/**
 * @author Kevin Klues <klueska@cs.stanford.edu>
 */

#include "thread.h"

generic configuration ThreadC(uint16_t stack_size) {
  provides {
    interface Thread;
    interface ThreadNotification;
    interface ThreadInfo;
  }
}
implementation {
  enum {
    THREAD_ID = unique(UQ_TOS_THREAD),
  };
  
  components MainC;
  components new ThreadInfoP(stack_size, THREAD_ID);
  components StaticThreadC;
  components ThreadMapC;
  
  MainC.SoftwareInit -> ThreadInfoP;
  Thread = StaticThreadC.Thread[THREAD_ID];
  ThreadNotification = StaticThreadC.ThreadNotification[THREAD_ID];
  ThreadInfo = ThreadInfoP;
  
  StaticThreadC.ThreadFunction[THREAD_ID] -> ThreadInfoP;
  StaticThreadC.ThreadCleanup[THREAD_ID]  -> ThreadMapC.StaticThreadCleanup[THREAD_ID];
  StaticThreadC.ThreadInfo[THREAD_ID] -> ThreadInfoP;
  
  components LedsC;
  ThreadInfoP.Leds -> LedsC;
}
