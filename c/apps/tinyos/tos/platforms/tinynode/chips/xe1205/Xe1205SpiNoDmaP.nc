/**
 * Copyright (c) 2005-2006 Arched Rock Corporation
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
 * - Neither the name of the Arched Rock Corporation nor the names of
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
 * @author Jonathan Hui <jhui@archedrock.com>
 * @author Maxime Muller
 * @version $Revision: 1.1 $ $Date: 2007-07-13 15:54:09 $
 *
 * nss_data toggling for each rx byte
 */


generic module Xe1205SpiNoDmaP() {

  provides interface Resource[ uint8_t id ];
  provides interface ResourceConfigure[ uint8_t id ];
  provides interface SpiByte;
  provides interface SpiPacket[ uint8_t id ];

  uses interface Resource as UsartResource[ uint8_t id ];
  uses interface Msp430SpiConfigure[ uint8_t id ];
  uses interface HplMsp430Usart as Usart;
  uses interface HplMsp430UsartInterrupts as UsartInterrupts;
  uses interface Leds;

}

implementation {

  enum {
    SPI_ATOMIC_SIZE = 2,
  };

  norace uint8_t* m_tx_buf;
  norace uint8_t* m_rx_buf;
  norace uint16_t m_len;
  norace uint16_t m_pos;
  norace uint8_t m_client;

  void signalDone();
  task void signalDone_task();

  async command error_t Resource.immediateRequest[ uint8_t id ]() {
    return call UsartResource.immediateRequest[ id ]();
  }

  async command error_t Resource.request[ uint8_t id ]() {
    return call UsartResource.request[ id ]();
  }

  async command bool Resource.isOwner[ uint8_t id ]() {
    return call UsartResource.isOwner[ id ]();
  }

  async command error_t Resource.release[ uint8_t id ]() {
    return call UsartResource.release[ id ]();
  }

  async command void ResourceConfigure.configure[ uint8_t id ]() {
    call Usart.setModeSpi(call Msp430SpiConfigure.getConfig[id]());
  }

  async command void ResourceConfigure.unconfigure[ uint8_t id ]() {
    call Usart.resetUsart(TRUE);
    call Usart.disableSpi();
    call Usart.resetUsart(FALSE);
  }

  event void UsartResource.granted[ uint8_t id ]() {
    signal Resource.granted[ id ]();
  }

  async command uint8_t SpiByte.write( uint8_t tx ) {
    uint8_t byte;
    // we are in spi mode which is configured to have turned off interrupts
    // call Usart.disableRxIntr();
    call Usart.tx( tx );
    while( !call Usart.isRxIntrPending() );
    call Usart.clrRxIntr();
    byte = call Usart.rx();
    //    call Usart.enableRxIntr();
    return byte;
  }

  default async command bool UsartResource.isOwner[ uint8_t id ]() { return FALSE; }
  default async command error_t UsartResource.request[ uint8_t id ]() { return FAIL; }
  default async command error_t UsartResource.immediateRequest[ uint8_t id ]() { return FAIL; }
  default async command error_t UsartResource.release[ uint8_t id ]() { return FAIL; }
 default async command msp430_spi_union_config_t* Msp430SpiConfigure.getConfig[uint8_t id]() {
    return &msp430_spi_default_config;
  }

  default event void Resource.granted[ uint8_t id ]() {}

  void continueOp() {
      uint8_t end;
      uint8_t tmp;
      
      atomic {
	  
	  TOSH_CLR_NSS_DATA_PIN();
	  call Usart.tx( 0 );
	  while( !call Usart.isRxIntrPending() );
	  TOSH_SET_NSS_DATA_PIN();
	  
	  end = m_pos + SPI_ATOMIC_SIZE;
	  if ( end > m_len )
	      end = m_len;
	  
	  while ( ++m_pos < end ) {
	      TOSH_CLR_NSS_DATA_PIN();
	      call Usart.tx( 0 );
	      while( !call Usart.isRxIntrPending() );	    
	      tmp = call Usart.rx();
	      m_rx_buf[ m_pos - 1 ] = tmp;
	  }
      }
  }
  


  inline  void sendBuffer(uint8_t const* buffer_, int size_) {

      TOSH_CLR_NSS_DATA_PIN();

      call Usart.isTxIntrPending();
      call Usart.rx();
      while(size_ > 0) {
	  call Usart.tx(*buffer_);
	  ++buffer_;
	  --size_;	      
	  while( !call Usart.isRxIntrPending() );
	  call Usart.rx();
      }
      call Usart.disableRxIntr();
      TOSH_SET_NSS_DATA_PIN();
  }



     inline void readData() {
      uint8_t end;
      uint8_t tmp;
      
      atomic {
	TOSH_TOGGLE_YELLOW_LED_PIN();
	  call Usart.tx( 0 );
	  call Usart.isTxIntrPending();
	  call Usart.rx();
	  end = m_pos + SPI_ATOMIC_SIZE;
	  if ( end > m_len )
	      end = m_len;
	
	  while ( ++m_pos < end ) {
	      TOSH_CLR_NSS_DATA_PIN();
	      call Usart.tx( 0 );
	      while( !call Usart.isRxIntrPending() );
	      tmp = call Usart.rx();
	      TOSH_SET_NSS_DATA_PIN();
	      m_rx_buf[ m_pos - 1 ] = tmp;
	  }
      }
      
  }

  async command error_t SpiPacket.send[ uint8_t id ]( uint8_t* tx_buf,
                                                      uint8_t* rx_buf,
                                                      uint16_t len ) {

      m_client = id;
      m_tx_buf = tx_buf;
      m_rx_buf = rx_buf;
      m_len = len;
      m_pos = 0;

      if (m_tx_buf)
	  sendBuffer(m_tx_buf,m_len);
      else {
	  call Usart.enableRxIntr();

	  continueOp();
      }

      return SUCCESS;

  }

  task void signalDone_task() {
      atomic signalDone();
  }

  async event void UsartInterrupts.rxDone( uint8_t data ) {
   
      if ( m_rx_buf ) {
	  m_rx_buf[ m_pos-1 ] = data;
      } else 
	  return;
     TOSH_SET_NSS_DATA_PIN();
    if ( m_pos < m_len ) {
	continueOp();

    } else {
	call Usart.disableRxIntr();
	signalDone();
    }
  }

  void signalDone() {
    signal SpiPacket.sendDone[ m_client ]( m_tx_buf, m_rx_buf, m_len,
					   SUCCESS );
  }

  async event void UsartInterrupts.txDone() {}

  default async event void SpiPacket.sendDone[ uint8_t id ]( uint8_t* tx_buf, uint8_t* rx_buf, uint16_t len, error_t error ) {}

}
