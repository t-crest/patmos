/*
 * Copyright (c) 2005-2006 Arch Rock Corporation
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
 * - Neither the name of the Arch Rock Corporation nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * ARCHED ROCK OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE
 */

/**
 * An HAL abstraction of the ChipCon CC2420 radio. This abstraction
 * deals specifically with radio configurations. All get() and set()
 * commands are single-phase. After setting some values, a call to
 * sync() is required for the changes to propagate to the cc2420
 * hardware chip. This interface allows setting multiple parameters
 * before calling sync().
 *
 * @author Jonathan Hui <jhui@archrock.com>
 * @author Jan Hauer <hauer@tkn.tu-berlin.de> (added some commands)
 * @version $Revision: 1.1 $ $Date: 2008-06-16 18:02:40 $
 */

interface CC2420Config {

  /**
   * Sync configuration changes with the radio hardware. This only
   * applies to set commands below.
   *
   * @return SUCCESS if the request was accepted, FAIL otherwise.
   */
  async command error_t sync();

  /**
   * Whether changes have been made that should be sync-ed.
   *
   * @return TRUE if changes have been made, FALSE otherwise.
   */
  async command bool needsSync();

  /**
   * Change the channel of the radio, between 11 and 26
   */
  command uint8_t getChannel();
  command void setChannel( uint8_t channel );

  /**
   * Change the short address of the radio.
   */
  async command uint16_t getShortAddr();
  command void setShortAddr( uint16_t address );

  /**
   * Change the PAN address of the radio.
   */
  async command uint16_t getPanAddr();
  command void setPanAddr( uint16_t address );

  /**
   * Change the PAN address of the radio.
   */
  async command bool getPanCoordinator();
  command void setPanCoordinator( bool pcoord );  

  /**
   * Change to promiscuous mode.
   */
  command void setPromiscuousMode(bool on);
  async command bool isPromiscuousModeEnabled();

  /**
   * Change the CCA mode.
   */
  async command uint8_t getCCAMode();
  command void setCCAMode(uint8_t mode);

  /**
   * Change the transmission power.
   */
  async command uint8_t getTxPower();
  command void setTxPower(uint8_t txPower);
  
}
