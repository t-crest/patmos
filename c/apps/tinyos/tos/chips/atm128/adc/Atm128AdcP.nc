/* $Id: Atm128AdcP.nc,v 1.8 2010-06-29 22:07:43 scipio Exp $
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
 * Copyright (c) 2002-2005 Intel Corporation
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE     
 * file. If you do not find these files, copies can be found by writing to
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300, Berkeley, CA, 
 * 94704.  Attention:  Intel License Inquiry.
 *
 * Copyright (c) 2004-2005 Crossbow Technology, Inc.  All rights reserved.
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
 * - Neither the name of Crossbow Technology nor the names of
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

#include "Atm128Adc.h"

/**
 * Internal component of the Atmega128 A/D HAL.
 *
 * @author Jason Hill
 * @author David Gay
 * @author Philip Levis
 * @author Phil Buonadonna
 * @author Hu Siquan <husq@xbow.com>
 */

module Atm128AdcP @safe()
{
  provides {
    interface Init;
    interface AsyncStdControl;
    interface Atm128AdcSingle;
    interface Atm128AdcMultiple;
  }
  uses {
    interface HplAtm128Adc;
    interface Atm128Calibrate;
  }
}
implementation
{  
  /* State for the current and next (multiple-sampling only) conversion */
  struct {
    bool multiple : 1;		/* single and multiple-sampling mode */
    bool precise : 1;		/* is this result going to be precise? */
    uint8_t channel : 5;	/* what channel did this sample come from? */
  } f, nextF;
  
  command error_t Init.init() {
    atomic
      {
	Atm128Adcsra_t adcsr;

	adcsr.aden = ATM128_ADC_ENABLE_OFF;
	adcsr.adsc = ATM128_ADC_START_CONVERSION_OFF;  
	adcsr.adfr = ATM128_ADC_FREE_RUNNING_OFF; 
	adcsr.adif = ATM128_ADC_INT_FLAG_OFF;               
	adcsr.adie = ATM128_ADC_INT_ENABLE_OFF;       
	adcsr.adps = ATM128_ADC_PRESCALE_2;
	call HplAtm128Adc.setAdcsra(adcsr);
      }
    return SUCCESS;
  }

  /* We enable the A/D when start is called, and disable it when stop is
     called. This drops A/D conversion latency by a factor of two (but
     increases idle mode power consumption a little). 
  */
  async command error_t AsyncStdControl.start() {
    atomic call HplAtm128Adc.enableAdc();
    return SUCCESS;
  }

  async command error_t AsyncStdControl.stop() {
    atomic call HplAtm128Adc.disableAdc();

    return SUCCESS;
  }

  /* Return TRUE if switching to 'channel' with reference voltage 'refVoltage'
     will give a precise result (the first sample after changing reference
     voltage or switching to/between a differential channel is imprecise)
  */
  inline bool isPrecise(Atm128Admux_t admux, uint8_t channel, uint8_t refVoltage) {
    return refVoltage == admux.refs &&
      (channel <= ATM128_ADC_SNGL_ADC7 || channel >= ATM128_ADC_SNGL_1_23 || channel == admux.mux);
  }

  async event void HplAtm128Adc.dataReady(uint16_t data) {
    bool precise, multiple;
    uint8_t channel;

    atomic 
      {
	channel = f.channel;
	precise = f.precise;
	multiple = f.multiple;
      }

    if (!multiple)
      {
	/* A single sample. Disable the ADC interrupt to avoid starting
	   a new sample at the next "sleep" instruction. */
	call HplAtm128Adc.disableInterruption();
	signal Atm128AdcSingle.dataReady(data, precise);
      }
    else
      {
	/* Multiple sampling. The user can:
	   - tell us to stop sampling
	   - or, to continue sampling on a new channel, possibly with a
	     new reference voltage; however this change applies not to
	     the next sample (the hardware has already started working on
	     that), but on the one after.
	*/
	bool cont;
	uint8_t nextChannel, nextVoltage;
	Atm128Admux_t admux;

	atomic 
	  {
	    admux = call HplAtm128Adc.getAdmux();
	    nextVoltage = admux.refs;
	    nextChannel = admux.mux;
	  }

	cont = signal Atm128AdcMultiple.dataReady(data, precise, channel,
						  &nextChannel, &nextVoltage);
	atomic
	  if (cont)
	    {
	      /* Switch channels and update our internal channel+precision
		 tracking state (f and nextF). Note that this tracking will
		 be incorrect if we take too long to get to this point. */
	      admux.refs = nextVoltage;
	      admux.mux = nextChannel;
	      call HplAtm128Adc.setAdmux(admux);

	      f = nextF;
	      nextF.channel = nextChannel;
	      nextF.precise = isPrecise(admux, nextChannel, nextVoltage);
	    }
	  else
	    call HplAtm128Adc.cancel();
      }
  }

  /* Start sampling based on request parameters */
  void getData(uint8_t channel, uint8_t refVoltage, bool leftJustify, uint8_t prescaler) {
    Atm128Admux_t admux;
    Atm128Adcsra_t adcsr;

    admux = call HplAtm128Adc.getAdmux();
    f.precise = isPrecise(admux, channel, refVoltage);
    f.channel = channel;

    admux.refs = refVoltage;
    admux.adlar = leftJustify;
    admux.mux = channel;
    call HplAtm128Adc.setAdmux(admux);

    adcsr.aden = ATM128_ADC_ENABLE_ON;
    adcsr.adsc = ATM128_ADC_START_CONVERSION_ON;
    adcsr.adfr = f.multiple;
    adcsr.adif = ATM128_ADC_INT_FLAG_ON; // clear any stale flag
    adcsr.adie = ATM128_ADC_INT_ENABLE_ON;
    if (prescaler == ATM128_ADC_PRESCALE)
      prescaler = call Atm128Calibrate.adcPrescaler();
    adcsr.adps = prescaler;
    call HplAtm128Adc.setAdcsra(adcsr);
  }

  async command bool Atm128AdcSingle.getData(uint8_t channel, uint8_t refVoltage,
					     bool leftJustify, uint8_t prescaler) {
    atomic
      {
	f.multiple = FALSE;
	getData(channel, refVoltage, leftJustify, prescaler);

	return f.precise;
      }
  }

  async command bool Atm128AdcSingle.cancel() {
    /* There is no Atm128AdcMultiple.cancel, for reasons discussed in that
       interface */
    return call HplAtm128Adc.cancel();
  }

  async command bool Atm128AdcMultiple.getData(uint8_t channel, uint8_t refVoltage,
					       bool leftJustify, uint8_t prescaler) {
    atomic
      {
	f.multiple = TRUE;
	getData(channel, refVoltage, leftJustify, prescaler);
	nextF = f;
	/* We assume the 2nd sample is precise */
	nextF.precise = TRUE;

	return f.precise;
      }
  }

  default async event void Atm128AdcSingle.dataReady(uint16_t data, bool precise) {
  }

  default async event bool Atm128AdcMultiple.dataReady(uint16_t data, bool precise, uint8_t channel,
						       uint8_t *newChannel, uint8_t *newRefVoltage) {
    return FALSE; // stop conversion if we somehow end up here.
  }
}
