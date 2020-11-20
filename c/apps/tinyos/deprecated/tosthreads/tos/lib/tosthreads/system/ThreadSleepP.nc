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

module ThreadSleepP {
  provides {
    interface ThreadSleep;
  }
  uses {
    interface SystemCall;
    interface ThreadScheduler;
    interface Timer<TMilli> as TimerMilli[uint8_t id];
    interface Leds;
  }
}
implementation {
  typedef struct sleep_params {
    uint32_t* milli;
  } sleep_params_t;
  
  void sleepTask(syscall_t* s) {
    sleep_params_t* p = s->params;
    call TimerMilli.startOneShot[s->thread->id]( *(p->milli) );
  }
  
  command error_t ThreadSleep.sleep(uint32_t milli) {
    syscall_t s;
    sleep_params_t p;    
    p.milli = &milli;
    call SystemCall.start(&sleepTask, &s, INVALID_ID, &p);
    return SUCCESS;
  }
  
  //Need to add a cancel command so that components like
  //BlockingAMReceiverImplP can use the ThreadSleep interface
  //directly instead of reusing the underlying ThreadTimerC 
  //component.  The current implementation does things this way
  //and causes us to be defensive in here since the same 
  //TimerMilli.fired() event is sent to that component as well.
  //Basically its just broken....  cancel() would get rid of this.
  
  //Also need some sort of a sleepWithSyscall command
  //Need to think about this one a little more
  
  event void TimerMilli.fired[uint8_t id]() {
    thread_t* t = call ThreadScheduler.threadInfo(id);
    if(t->syscall->syscall_ptr == sleepTask)
	    call SystemCall.finish(t->syscall);
  }
}
