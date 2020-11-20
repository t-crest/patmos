//$Id: VirtualizeAlarmC.nc,v 1.8 2010-06-29 22:07:50 scipio Exp $

/* Copyright (c) 2000-2003 The Regents of the University of California.  
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
 * VirtualizeAlarmC uses a single Alarm to create up to 255 virtual alarms.
 * Note that a virtualized Alarm will have significantly more overhead than
 * an Alarm built on a hardware compare register.
 *
 * @param precision_tag A type indicating the precision of the Alarm being 
 *   virtualized.
 * @param num_alarms Number of virtual alarms to create.
 *
 * @author Cory Sharp <cssharp@eecs.berkeley.edu>
 */

generic module VirtualizeAlarmC(typedef precision_tag, typedef size_type @integer(), int num_alarms)
{
  provides interface Init;
  provides interface Alarm<precision_tag,size_type> as Alarm[uint8_t id];
  uses interface Alarm<precision_tag,size_type> as AlarmFrom;
}
implementation
{
  enum {
    NUM_ALARMS = num_alarms,
  };

  typedef struct {
    size_type t0;
    size_type dt;
  } alarm_t;

  // css 26 jul 2006: All computations with respect to the current time ("now")
  // require that "now" is (non-strictly) monotonically increasing.  Calling
  // setNextAlarm within Alarm.start within Alarm.fired within signalAlarms
  // breaks this monotonicity requirements when "now" is cached at the start of
  // the function.  Two ways around this: 1) refresh "now" each time it is
  // used, or 2) use the is_signaling flag to prevent setNextAlarm from being
  // called inside signalAlarms.  The latter is generally more efficient by
  // preventing redundant calls to setNextAlarm at the expense of an extra byte
  // of RAM, so that's what the code does now.  Update: option 2 is
  // unacceptable because an Alarm.start could be called within some other
  // Alarm.fired, which can break monotonicity in now.

  // A struct of member variables so only one memset is called for init.
  struct {
    alarm_t alarm[NUM_ALARMS];
    bool isset[NUM_ALARMS];
    bool is_signaling;
  } m;

  command error_t Init.init() {
    memset( &m, 0, sizeof(m) );
    return SUCCESS;
  }

  void setNextAlarm() {
    if( !m.is_signaling ) {
      // css 25 jul 2006: To help prevent various problems with overflow, the
      // elapsed time from t0 for a particular alarm is calculated as
      // elapsed=now-t0 then dt-=elapsed and t0=now.  However, this means that
      // now must be a monotonically increasing value with each call to
      // setNextAlarm -- overflow in now is okay, but passing in older values of
      // now=t0 for some arbitrary t0 is not okay, which is what the previous
      // version of setAlarm did.

      const size_type now = call AlarmFrom.getNow();
      const alarm_t* pEnd = m.alarm+NUM_ALARMS;
      bool isset = FALSE;
      alarm_t* p = m.alarm;
      bool* pset = m.isset;
      size_type dt = ((size_type)0)-((size_type)1);

      for( ; p!=pEnd; p++,pset++ ) {
        if( *pset ) {
          size_type elapsed = now - p->t0;
          if( p->dt <= elapsed ) {
            p->t0 += p->dt;
            p->dt = 0;
          }
          else {
            p->t0 = now;
            p->dt -= elapsed;
          }

          if( p->dt <= dt ) {
            dt = p->dt;
            isset = TRUE;
          }
        }
      }

      if( isset ) {
        // css 25 jul 2006: If dt is big, then wait half of dt.  This helps
        // significantly reduce the chance of overflow in the elapsed calculation
        // for the alarm.  "big" is if the most signficant bit in dt is set.

        if( dt & (((size_type)1) << (8*sizeof(size_type)-1)) )
          dt >>= 1;

        call AlarmFrom.startAt( now, dt );
      }
      else {
        call AlarmFrom.stop();
      }
    }
  }
  
  void signalAlarms() {
    uint8_t id;

    m.is_signaling = TRUE;

    for( id=0; id<NUM_ALARMS; id++ ) {
      if( m.isset[id] ) {
        //size_type elapsed = call AlarmFrom.getNow() - m.alarm[id].t0;
        //if( m.alarm[id].dt <= elapsed ) {
        size_type t0 = m.alarm[id].t0;
        size_type elapsed = call AlarmFrom.getNow() - t0;
        if( m.alarm[id].dt <= elapsed ) {
          m.isset[id] = FALSE;
          signal Alarm.fired[id]();
        }
      }
    }

    m.is_signaling = FALSE;
  }


  // basic interface
  async command void Alarm.start[uint8_t id]( size_type dt ) {
    call Alarm.startAt[id]( call AlarmFrom.getNow(), dt );
  }

  async command void Alarm.stop[uint8_t id]() {
    atomic m.isset[id] = FALSE;
  }

  async event void AlarmFrom.fired() {
    atomic {
      signalAlarms();
      setNextAlarm();
    }
  }


  // extended interface
  async command bool Alarm.isRunning[uint8_t id]() {
    atomic return m.isset[id];
  }

  async command void Alarm.startAt[uint8_t id]( size_type t0, size_type dt ) {
    atomic {
      m.alarm[id].t0 = t0;
      m.alarm[id].dt = dt;
      m.isset[id] = TRUE;
      setNextAlarm();
    }
  }

  async command size_type Alarm.getNow[uint8_t id]() {
    return call AlarmFrom.getNow();
  }

  async command size_type Alarm.getAlarm[uint8_t id]() {
    atomic return m.alarm[id].t0 + m.alarm[id].dt;
  }

  default async event void Alarm.fired[uint8_t id]() {
  }
}

