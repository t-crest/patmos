/*
 * Copyright (c) 2004, Technische Universitaet Berlin
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright 
 *   notice, this list of conditions and the following disclaimer in the 
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name of the Technische Universitaet Berlin nor the names 
 *   of its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * - Revision -------------------------------------------------------------
 * $Revision: 1.6 $
 * $Date: 2007-11-28 13:19:49 $
 * @author: Jan Hauer <hauer@tkn.tu-berlin.de>
 * ========================================================================
 */

/** 
 *
 * Please refer to TEP 109 for more information about this component and its
 * intended use. This component provides platform-independent access to the
 * onboard temperature sensor on the eyesIFXv{1,2} platform.
 *
 * @author Jan Hauer
 */

#include <sensors.h>
generic configuration TempExtSensorC()
{
  provides {
    interface Read<uint16_t> as Read;
    interface ReadNow<uint16_t> as ReadNow;
    interface Resource as ReadNowResource;
  }
}
implementation
{
  components SensorSettingsC as Settings;  
  
  components new AdcReadClientC() as AdcReadClient;
  //Read = AdcReadClient;
  AdcReadClient.AdcConfigure -> Settings.AdcConfigure[TEMP_SENSOR_DEFAULT];
  
  
  components new AdcReadNowClientC() as AdcReadNowClient;
  ReadNow = AdcReadNowClient;
  //ReadNowResource = AdcReadNowClient;
  AdcReadNowClient.AdcConfigure -> Settings.AdcConfigure[TEMP_SENSOR_DEFAULT];
  
  components HplMsp430GeneralIOC as MspGeneralIO;
  components new Msp430GpioC() as TEMP;
  TEMP -> MspGeneralIO.Port54;
  
  components TempExtSensorP;
  Read = TempExtSensorP.Read;
  TempExtSensorP.AdcRead -> AdcReadClient;
  ReadNowResource = TempExtSensorP.ReadNowResource;
  TempExtSensorP.AdcResource -> AdcReadNowClient;
  TempExtSensorP.TEMP -> TEMP;
}
