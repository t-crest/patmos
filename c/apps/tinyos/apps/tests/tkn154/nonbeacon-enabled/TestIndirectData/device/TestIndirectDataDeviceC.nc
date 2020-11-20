/*
 * Copyright (c) 2009, Technische Universitaet Berlin
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
 * $Revision: 1.2 $
 * $Date: 2009-09-10 12:00:50 $
 * @author: Jasper Buesch <buesch@tkn.tu-berlin.de>
 * ========================================================================
 */

#include "TKN154.h"
#include "app_profile.h"
module TestIndirectDataDeviceC
{
  uses {
    interface Boot;
    interface MCPS_DATA;
    interface MLME_RESET;
    interface MLME_SET;
    interface MLME_GET;
    interface MLME_POLL;
    interface Leds;
    interface Timer<T62500hz> as PollTimer;
    interface Timer<T62500hz> as Led2Timer;
  }
} implementation {

  void startApp();

  event void Boot.booted() {
    call MLME_RESET.request(TRUE);
  }

  event void MLME_RESET.confirm(ieee154_status_t status)
  {
    if (status == IEEE154_SUCCESS)
      startApp();
  }

  void startApp()
  {
    call MLME_SET.phyCurrentChannel(RADIO_CHANNEL);
    call MLME_SET.macAutoRequest(FALSE);
    call MLME_SET.macPANId(PAN_ID);
    call MLME_SET.macCoordShortAddress(COORDINATOR_ADDRESS);  
    call MLME_SET.macShortAddress(DEVICE_ADDRESS);
    call PollTimer.startPeriodic(62500U); 
  }

  event void PollTimer.fired(){
    // check the coordinator for outstanding transmissions
    ieee154_address_t coordAdr;
    coordAdr.shortAddress = COORDINATOR_ADDRESS;
    call MLME_POLL.request  (
                          ADDR_MODE_SHORT_ADDRESS,
                          PAN_ID,
                          coordAdr,
                          NULL
                        );
  }

  event void MLME_POLL.confirm    (
                          ieee154_status_t status
                        ){}

  event void MCPS_DATA.confirm(
                          message_t *msg,
                          uint8_t msduHandle,
                          ieee154_status_t status,
                          uint32_t Timestamp
                        ){}

  event message_t* MCPS_DATA.indication ( message_t* frame__ ){
    call Leds.led2On();
    call Led2Timer.startOneShot(12500U);
    return frame__;
  }

  event void Led2Timer.fired(){
    call Leds.led2Off();
  }

}
