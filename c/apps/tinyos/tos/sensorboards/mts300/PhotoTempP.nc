/* $Id: PhotoTempP.nc,v 1.4 2007-03-14 04:14:13 pipeng Exp $
 * Copyright (c) 2007 Intel Corporation
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE     
 * file. If you do not find these files, copies can be found by writing to
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300, Berkeley, CA, 
 * 94704.  Attention:  Intel License Inquiry.
 */
/**
 * MTS300 photo and temp sensor ADC configuration.
 * @author David Gay <david.e.gay@intel.com>
 */
module PhotoTempP
{
  provides interface Atm128AdcConfig;
  uses interface MicaBusAdc as PhotoTempAdc;
}
implementation
{
  async command uint8_t Atm128AdcConfig.getChannel() {
    return call PhotoTempAdc.getChannel();
  }

  async command uint8_t Atm128AdcConfig.getRefVoltage() {
    return ATM128_ADC_VREF_OFF;
  }

  async command uint8_t Atm128AdcConfig.getPrescaler() {
    return ATM128_ADC_PRESCALE;
  }
}
