/*
 * Copyright (c) 2008, Technische Universitaet Berlin
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
 * $Revision: 1.10 $
 * $Date: 2009-05-18 12:54:10 $
 * @author Jan Hauer <hauer@tkn.tu-berlin.de>
 * ========================================================================
 */

#include "TKN154_PHY.h"
#include "TKN154_MAC.h"

/** 
 * This module is responsible for the transmission/reception of DATA and
 * COMMAND frames in a nonbeacon-enabled PAN. Its main tasks are initialization
 * of the parameters of the unslotted CSMA-CA algorithm (NB, BE, etc.),
 * initiating retransmissions and managing requests for enabling the receiver
 * for a finite period of time. It does not implement the actual CSMA-CA
 * algorithm, because due to its timing requirements the CSMA-CA algorithm is
 * not part of the MAC implementation but of the chip-specific radio driver.
 */

module DispatchUnslottedCsmaP
{
  provides
  {
    interface Init as Reset;
    interface FrameTx as FrameTx;
    interface FrameRx as FrameRx[uint8_t frameType];
    interface FrameExtracted as FrameExtracted[uint8_t frameType];
    interface Notify<bool> as WasRxEnabled;
  }
  uses
  {
    interface Timer<TSymbolIEEE802154> as IndirectTxWaitTimer;
    interface TransferableResource as RadioToken;
    interface ResourceRequested as RadioTokenRequested;
    interface GetNow<token_requested_t> as IsRadioTokenRequested;
    interface GetNow<bool> as IsRxEnableActive; 
    interface Set<ieee154_macSuperframeOrder_t> as SetMacSuperframeOrder;
    interface Get<ieee154_txframe_t*> as GetIndirectTxFrame; 
    interface Notify<bool> as RxEnableStateChange;
    interface Notify<const void*> as PIBUpdateMacRxOnWhenIdle;
    interface FrameUtility;
    interface UnslottedCsmaCa;
    interface RadioRx;
    interface RadioOff;
    interface MLME_GET;
    interface MLME_SET;
    interface TimeCalc;
    interface Leds;
  }
}
implementation
{
  typedef enum {
    SWITCH_OFF,       
    WAIT_FOR_RXDONE,  
    WAIT_FOR_TXDONE,  
    DO_NOTHING,       
  } next_state_t; 

  typedef enum {
    INDIRECT_TX_ALARM,
    NO_ALARM,
  } rx_alarm_t;

  /* state / frame management */
  norace bool m_lock;
  norace bool m_resume;
  norace ieee154_txframe_t *m_currentFrame;
  norace ieee154_txframe_t *m_lastFrame;
  norace ieee154_macRxOnWhenIdle_t m_macRxOnWhenIdle;

  /* variables for the unslotted CSMA-CA */
  norace ieee154_csma_t m_csma;
  norace ieee154_macMaxBE_t m_BE;
  norace ieee154_macMaxCSMABackoffs_t m_macMaxCSMABackoffs;
  norace ieee154_macMaxBE_t m_macMaxBE;
  norace ieee154_macMaxFrameRetries_t m_macMaxFrameRetries;
  norace ieee154_status_t m_txStatus;
  norace uint32_t m_transactionTime;
  norace bool m_indirectTxPending = FALSE;

  /* function / task prototypes */
  next_state_t tryReceive();
  next_state_t tryTransmit();
  next_state_t trySwitchOff();
  void backupCurrentFrame();
  void restoreFrameFromBackup();  
  void updateState();
  void setCurrentFrame(ieee154_txframe_t *frame);
  void signalTxBroadcastDone(ieee154_txframe_t *frame, ieee154_status_t error);
  task void signalTxDoneTask();
  task void wasRxEnabledTask();
  task void startIndirectTxTimerTask();

  command error_t Reset.init()
  {
    if (m_currentFrame)
      signal FrameTx.transmitDone(m_currentFrame, IEEE154_TRANSACTION_OVERFLOW);
    if (m_lastFrame)
      signal FrameTx.transmitDone(m_lastFrame, IEEE154_TRANSACTION_OVERFLOW);
    m_currentFrame = m_lastFrame = NULL;
    call IndirectTxWaitTimer.stop();
    return SUCCESS;
  }

  command ieee154_status_t FrameTx.transmit(ieee154_txframe_t *frame)
  {
    if (m_currentFrame != NULL) {
      // we've not finished transmitting the current frame yet
      dbg_serial("DispatchUnslottedCsmaP", "Overflow\n");
      return IEEE154_TRANSACTION_OVERFLOW;
    } else {
      setCurrentFrame(frame);
      if (call RadioToken.isOwner())
        updateState();
      else
        call RadioToken.request();      
      return IEEE154_SUCCESS;
    }
  }

  event void RadioToken.granted()
  {
    updateState();
  }

  void setCurrentFrame(ieee154_txframe_t *frame)
  {
    if (frame->header->mhr[MHR_INDEX_FC1] != FC1_FRAMETYPE_BEACON) { 
      // set the sequence number for command/data frame
      ieee154_macDSN_t dsn = call MLME_GET.macDSN();
      frame->header->mhr[MHR_INDEX_SEQNO] = dsn++;
      call MLME_SET.macDSN(dsn);
    } else {
      // set the sequence number for beacon frame
      ieee154_macBSN_t bsn = call MLME_GET.macBSN(); 
      frame->header->mhr[MHR_INDEX_SEQNO] = bsn++;
      call MLME_SET.macBSN(bsn);
    }    
    m_csma.NB = 0;
    m_csma.macMaxCsmaBackoffs = m_macMaxCSMABackoffs = call MLME_GET.macMaxCSMABackoffs();
    m_csma.macMaxBE = m_macMaxBE = call MLME_GET.macMaxBE();
    m_csma.BE = call MLME_GET.macMinBE();
    if (call MLME_GET.macBattLifeExt() && m_csma.BE > 2)
      m_csma.BE = 2;
    m_BE = m_csma.BE;
    if (call GetIndirectTxFrame.get() == frame)
      m_macMaxFrameRetries =  0; // this is an indirect transmissions (never retransmit)
    else
      m_macMaxFrameRetries =  call MLME_GET.macMaxFrameRetries();
    m_transactionTime = IEEE154_SHR_DURATION + 
      (frame->headerLen + frame->payloadLen + 2) * IEEE154_SYMBOLS_PER_OCTET; // extra 2 for CRC
    if (frame->header->mhr[MHR_INDEX_FC1] & FC1_ACK_REQUEST)
      m_transactionTime += (IEEE154_aTurnaroundTime + IEEE154_aUnitBackoffPeriod + 
          11 * IEEE154_SYMBOLS_PER_OCTET); // 11 byte for the ACK PPDU
    // if (frame->headerLen + frame->payloadLen > IEEE154_aMaxSIFSFrameSize) 
    //  m_transactionTime += call MLME_GET.macMinLIFSPeriod(); 
    // else 
    //  m_transactionTime += call MLME_GET.macMinSIFSPeriod(); 
    m_currentFrame = frame;
  }

  /** 
   * The updateState() function is called whenever some event happened that
   * might require a state transition; it implements a lock mechanism (m_lock)
   * to prevent race conditions. Whenever the lock is set a "done"-event (from
   * a RadioTx/RadioRx/RadioOff interface) is pending and will "soon" unset the
   * lock (and then updateState() will called again).  The updateState()
   * function decides about the next state by checking a list of possible
   * current states ordered by priority. Calling this function more than
   * necessary can do no harm.
   */ 

  void updateState()
  {
    next_state_t next;
    atomic {
      // long atomics are bad... but in this block, once the
      // current state has been determined only one branch will
      // be taken (there are no loops)
      if (m_lock || !call RadioToken.isOwner())
        return;
      m_lock = TRUE; // lock

      // Check 1: was an indirect transmission successfully started 
      // and are we now waiting for a frame from the coordinator?
      if (m_indirectTxPending) {
        next = tryReceive();
      }

      // Check 2: is some other operation (like MLME-SCAN or MLME-RESET) pending? 
      else if (call IsRadioTokenRequested.getNow()) {
        if (call RadioOff.isOff()) {
          // nothing more to do... just release the Token
          dbg_serial("DispatchUnslottedCsmaP", "Token requested: releasing it.\n");
          call RadioToken.request(); // we want it back afterwards ...
          m_lock = FALSE; // unlock
          call RadioToken.release();
          return;
        } else 
          next = SWITCH_OFF;
      }

      // Check 3: is there a frame ready to transmit?
      else if (m_currentFrame != NULL) {
        next = tryTransmit();
      }

      // Check 4: should we be in receive mode?
      else if (call IsRxEnableActive.getNow() || m_macRxOnWhenIdle) {
        next = tryReceive();
        if (next == DO_NOTHING) {
          // if there was an active MLME_RX_ENABLE.request then we'll
          // inform the next higher layer that radio is now in Rx mode
          post wasRxEnabledTask();
        }
      }

      // Check 5: just make sure the radio is switched off  
      else {
        next = trySwitchOff();
        if (next == DO_NOTHING) {
          // nothing more to do... just release the Token
          m_lock = FALSE; // unlock
          dbg_serial("DispatchUnslottedCsmaP", "Releasing token\n");
          call RadioToken.release();
          return;
        }
      }

      // if there is nothing to do, then we must clear the lock
      if (next == DO_NOTHING)
        m_lock = FALSE;
    } // atomic

    // put next state in operation (possibly keeping the lock)
    switch (next)
    {
      case SWITCH_OFF: ASSERT(call RadioOff.off() == SUCCESS); break;
      case WAIT_FOR_RXDONE: break;
      case WAIT_FOR_TXDONE: break;
      case DO_NOTHING: break;
    }
  }
  
  next_state_t tryTransmit()
  {
    // tries to transmit m_currentFrame
    next_state_t next;

    if (!call RadioOff.isOff())
      next = SWITCH_OFF;
    else {
      error_t res;
      res = call UnslottedCsmaCa.transmit(m_currentFrame, &m_csma);
      dbg_serial("DispatchUnslottedCsmaP", "UnslottedCsmaCa.transmit() -> %lu\n", (uint32_t) res);
      next = WAIT_FOR_TXDONE; // this will NOT clear the lock
    }
    return next;
  }

  next_state_t tryReceive()
  {
    next_state_t next;
    if (call RadioRx.isReceiving())
      next = DO_NOTHING;
    else if (!call RadioOff.isOff())
      next = SWITCH_OFF;
    else {
      call RadioRx.enableRx(0, 0);
      next = WAIT_FOR_RXDONE;
    }
    return next;
  }

  next_state_t trySwitchOff()
  {
    next_state_t next;
    if (call RadioOff.isOff())
      next = DO_NOTHING;
    else
      next = SWITCH_OFF;
    return next;
  }

  async event void RadioOff.offDone() 
  { 
    m_lock = FALSE; 
    updateState();
  }

  async event void RadioRx.enableRxDone() 
  { 
    if (m_indirectTxPending) // indirect transmission, now waiting for data
      post startIndirectTxTimerTask();
    m_lock = FALSE; 
    updateState();
  }

  event void RxEnableStateChange.notify(bool whatever) 
  { 
    if (!call RadioToken.isOwner())
      call RadioToken.request();
    else
      updateState();
  }

  event void PIBUpdateMacRxOnWhenIdle.notify( const void* val ) 
  {
    atomic m_macRxOnWhenIdle = *((ieee154_macRxOnWhenIdle_t*) val);
    signal RxEnableStateChange.notify(TRUE);
  }

  event void IndirectTxWaitTimer.fired() 
  { 
    atomic {
      if (m_indirectTxPending) {
        m_indirectTxPending = FALSE; 
        post signalTxDoneTask(); 
      }
    }
  }

  task void startIndirectTxTimerTask()
  {
    call IndirectTxWaitTimer.startOneShot(call MLME_GET.macMaxFrameTotalWaitTime()); 
  }

  async event void UnslottedCsmaCa.transmitDone(ieee154_txframe_t *frame, 
      ieee154_csma_t *csma, bool ackPendingFlag, error_t result)
  {
    bool done = TRUE;
    dbg_serial("DispatchUnslottedCsmaP", "UnslottedCsmaCa.transmitDone() -> %lu\n", (uint32_t) result);
    m_resume = FALSE;
    switch (result)
    {
      case SUCCESS:
        // frame was successfully transmitted, if ACK was requested
        // then a matching ACK was successfully received as well   
        m_txStatus = IEEE154_SUCCESS;
        if (frame->payload[0] == CMD_FRAME_DATA_REQUEST &&
            ((frame->header->mhr[MHR_INDEX_FC1]) & FC1_FRAMETYPE_MASK) == FC1_FRAMETYPE_CMD) {
          // this was a data request frame
          m_txStatus = IEEE154_NO_DATA; // pessimistic 
          if (ackPendingFlag) {
            // the coordinator has data for us; switch to Rx 
            // to complete the indirect transmission         
            m_indirectTxPending = TRUE;
            m_lastFrame = m_currentFrame;
            m_currentFrame = NULL;
            ASSERT(call RadioRx.enableRx(0, 0) == SUCCESS);
            return;
          }
        }
        break;
      case FAIL:
        // The CSMA-CA algorithm failed: the frame was not transmitted,
        // because channel was never idle                              
        m_txStatus = IEEE154_CHANNEL_ACCESS_FAILURE;
        break;
      case ENOACK:
        // frame was transmitted, but we didn't receive an ACK (although
        // we requested an one). note: coordinator never retransmits an 
        // indirect transmission (see above) 
        if (m_macMaxFrameRetries > 0) {
          // retransmit: reinitialize CSMA-CA parameters
          done = FALSE;
          m_csma.NB = 0;
          m_csma.macMaxCsmaBackoffs = m_macMaxCSMABackoffs;
          m_csma.macMaxBE = m_macMaxBE;
          m_csma.BE = m_BE;
          m_macMaxFrameRetries -= 1;
        } else
          m_txStatus = IEEE154_NO_ACK;
        break;
      default: 
        ASSERT(0);
        break;
    }

    if (done) {
      m_lastFrame = m_currentFrame;
      m_currentFrame = NULL;
      post signalTxDoneTask();
    }  

    m_lock = FALSE;
    updateState();
    dbg_serial_flush();
  }

  task void signalTxDoneTask()
  {
    ieee154_txframe_t *lastFrame = m_lastFrame;
    ieee154_status_t status = m_txStatus;
    m_indirectTxPending = FALSE;
    m_lastFrame = NULL; // only now the next transmission can begin 
    if (lastFrame) {
      dbg_serial("DispatchUnslottedCsmaP", "Transmit done, DSN: %lu, result: 0x%lx\n", 
          (uint32_t) MHR(lastFrame)[MHR_INDEX_SEQNO], (uint32_t) status);
      signal FrameTx.transmitDone(lastFrame, status);
    }
    updateState();
  }

  event message_t* RadioRx.received(message_t* frame)
  {
    // received a frame -> find out frame type and
    // signal it to responsible client component
    uint8_t *payload = (uint8_t *) frame->data;
    uint8_t *mhr = MHR(frame);
    uint8_t frameType = mhr[MHR_INDEX_FC1] & FC1_FRAMETYPE_MASK;

    if (frameType == FC1_FRAMETYPE_CMD)
      frameType += payload[0];
    atomic {
      if (m_indirectTxPending) {
        message_t* frameBuf;
        call IndirectTxWaitTimer.stop();
        // TODO: check!
        //if (frame->payloadLen)
          // is this frame from our coordinator? hmm... we cannot say
          // with certainty, because we might only know either the 
          // coordinator extended or short address (and the frame could
          // have been sent with the other addressing mode) ??
          m_txStatus = IEEE154_SUCCESS;
        frameBuf = signal FrameExtracted.received[frameType](frame, m_lastFrame);
        signal IndirectTxWaitTimer.fired();
        return frameBuf;
      } else
        return signal FrameRx.received[frameType](frame);
    }
  }

  task void wasRxEnabledTask()
  {
    signal WasRxEnabled.notify(TRUE);
  }


  default event void FrameTx.transmitDone(ieee154_txframe_t *data, ieee154_status_t status) {}
  default event message_t* FrameRx.received[uint8_t client](message_t* data) {return data;}
  default async command bool IsRxEnableActive.getNow() {return FALSE;}

  default event message_t* FrameExtracted.received[uint8_t client](message_t* msg, ieee154_txframe_t *txFrame) {return msg;}

  command error_t WasRxEnabled.enable() {return FAIL;}
  command error_t WasRxEnabled.disable() {return FAIL;}
  async event void RadioToken.transferredFrom(uint8_t fromClientID) {ASSERT(0);}
  async event void RadioTokenRequested.requested(){ updateState(); }
  async event void RadioTokenRequested.immediateRequested(){ updateState(); }
}
