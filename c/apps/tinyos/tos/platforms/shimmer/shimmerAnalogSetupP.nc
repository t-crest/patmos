/*
 * Copyright (c) 2010, Shimmer Research, Ltd.
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:

 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of Shimmer Research, Ltd. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @author Steve Ayer
 * @date March, 2010

 * this is going to get duplicated in a million apps, so we gather simple setup 
 * routines with reasonable defaults here.  specific 
 */


module shimmerAnalogSetupP {
  provides{
    interface shimmerAnalogSetup;
    interface Init;
  }
  uses {
    interface Msp430DmaControl;
    interface Msp430DmaChannel;
    interface HplAdc12;
    interface Leds;
  }
}

implementation { 
  void initADC12CTL0();
  void initADC12CTL1();
  void initADC12MEMCTLx();
  void setupDMA(uint16_t * destAddr);
  void addNewChannels(uint8_t * chans, uint8_t howmany_new);

  uint8_t NUM_ADC_CHANS = 0;
  uint8_t ADC_CHANS[8];  // msp430 only has eight!

  command void shimmerAnalogSetup.addAccelInputs() {
    uint8_t new_chans[] = { 5, 4, 3 };
    addNewChannels(new_chans, 3);
    
    initADC12MEMCTLx();

    TOSH_MAKE_ADC_5_INPUT();         
    TOSH_SEL_ADC_5_MODFUNC();

    TOSH_MAKE_ADC_4_INPUT();         
    TOSH_SEL_ADC_4_MODFUNC();

    TOSH_MAKE_ADC_3_INPUT();         
    TOSH_SEL_ADC_3_MODFUNC();
  }

  command void shimmerAnalogSetup.addGyroInputs() {
    uint8_t new_chans[] = { 1, 6, 2 };    // x, y, z
    addNewChannels(new_chans, 3);
    
    initADC12MEMCTLx();

    TOSH_MAKE_ADC_1_INPUT();         
    TOSH_SEL_ADC_1_MODFUNC();

    TOSH_MAKE_ADC_6_INPUT();         
    TOSH_SEL_ADC_6_MODFUNC();

    TOSH_MAKE_ADC_2_INPUT();         
    TOSH_SEL_ADC_2_MODFUNC();
  }

  // identical to ecg
  command void shimmerAnalogSetup.addStrainGaugeInputs(){
    call shimmerAnalogSetup.addECGInputs();
  }

  command void shimmerAnalogSetup.addECGInputs() {
    uint8_t new_chans[] = { 1, 2 };  // ecg_lall, ecg_rall
    addNewChannels(new_chans, 2);
    
    initADC12MEMCTLx();

    TOSH_MAKE_ADC_1_INPUT();         
    TOSH_SEL_ADC_1_MODFUNC();

    TOSH_MAKE_ADC_2_INPUT();         
    TOSH_SEL_ADC_2_MODFUNC();
  }

  command void shimmerAnalogSetup.addUVInputs() {
    uint8_t new_chans[] = { 1, 2, 6 };    // ambient, uvb, uva
    addNewChannels(new_chans, 3);
    
    initADC12MEMCTLx();

    TOSH_MAKE_ADC_1_INPUT();         
    TOSH_SEL_ADC_1_MODFUNC();

    TOSH_MAKE_ADC_2_INPUT();         
    TOSH_SEL_ADC_2_MODFUNC();

    TOSH_MAKE_ADC_6_INPUT();         
    TOSH_SEL_ADC_6_MODFUNC();
  }

  command void shimmerAnalogSetup.addGSRInput() { 
    uint8_t new_chans[] = { 6 };
    addNewChannels(new_chans, 1);
    
    initADC12MEMCTLx();

    TOSH_MAKE_ADC_6_INPUT();         
    TOSH_SEL_ADC_6_MODFUNC();
  }

  command void shimmerAnalogSetup.addEMGInput() { 
    uint8_t new_chans[] = { 1 };  // ecg_lall, ecg_rall
    addNewChannels(new_chans, 1);
    
    initADC12MEMCTLx();

    TOSH_MAKE_ADC_1_INPUT();         
    TOSH_SEL_ADC_1_MODFUNC();

    // this channel is identical to adc1, so we we don't read it, but make it an input to avoid pushing against the signal
    TOSH_MAKE_ADC_2_INPUT();         
  }

  command void shimmerAnalogSetup.addAnExInput(uint8_t channel) { 
    uint8_t new_chans[1];

    if(channel == 0){
      new_chans[0] = 0;
      addNewChannels(new_chans, 1);

      initADC12MEMCTLx();

      TOSH_MAKE_ADC_0_INPUT();         
      TOSH_SEL_ADC_0_MODFUNC();
    }
    else if(channel == 7) {
      new_chans[0] = 7;
      addNewChannels(new_chans, 1);

      initADC12MEMCTLx();

      TOSH_MAKE_ADC_7_INPUT();         
      TOSH_SEL_ADC_7_MODFUNC();
    }
  }

  command void shimmerAnalogSetup.finishADCSetup(uint16_t * buffer){
    setupDMA(buffer);
  }
  
  command void shimmerAnalogSetup.triggerConversion() {
    call Msp430DmaChannel.startTransfer();
    call HplAdc12.startConversion();
  }

  command void shimmerAnalogSetup.stopConversion() {
    call HplAdc12.stopConversion();
    call HplAdc12.setIEFlags(0);
    call HplAdc12.resetIFGs();
  }                                            
    
  command uint8_t shimmerAnalogSetup.getNumberOfChannels() {
    return NUM_ADC_CHANS;
  }

  command void shimmerAnalogSetup.reset() {
    NUM_ADC_CHANS = 0;
  }

  command error_t Init.init() {
    initADC12CTL0();
    initADC12CTL1();
    //    initADC12MEMCTLx();

    TOSH_uwait(50000UL);
    return SUCCESS;
  }

  void addNewChannels(uint8_t * chans, uint8_t howmany_new) {
    register uint8_t i, j;
    
    for(j = 0, i = NUM_ADC_CHANS; (j < howmany_new) && (i < 8) ; i++, j++)
      ADC_CHANS[i] = chans[j];

    NUM_ADC_CHANS += howmany_new;
  }

  void initADC12CTL0()
  {

    ADC12CTL0 = 0;
    SET_FLAG(ADC12CTL0, ADC12ON);
    SET_FLAG(ADC12CTL0, REF2_5V);
    SET_FLAG(ADC12CTL0, MSC);
    SET_FLAG(ADC12CTL0, SHT0_0);
    SET_FLAG(ADC12CTL0, SHT1_0); 
  }

  void initADC12CTL1()
  {
    ADC12CTL1 = 0;
    SET_FLAG(ADC12CTL1, CONSEQ_1);
    SET_FLAG(ADC12CTL1, ADC12SSEL_3);
    SET_FLAG(ADC12CTL1, ADC12DIV_0);
    SET_FLAG(ADC12CTL1, SHP);
  }

  void initADC12MEMCTLx()
  {
    uint8_t i;

    for (i = 0; i < NUM_ADC_CHANS; ++i) {
      ADC12MCTL[i] = 0;
      SET_FLAG(ADC12MCTL[i], ADC_CHANS[i]);  // inch is lower four bits
      // don't have to do anything to set sref to AVcc_AVss, which is 0
    }    
    SET_FLAG(ADC12MCTL[i - 1], EOS);
  }

  void setupDMA(uint16_t * destAddr) {
    call Msp430DmaControl.init();                                     // blanks registers

    call Msp430DmaControl.setFlags(FALSE, FALSE, FALSE);              // enable_nmi, round_robin, on_fetch

    call Msp430DmaChannel.setupTransfer(DMA_BLOCK_TRANSFER,           //dma_transfer_mode_t transfer_mode, 
					DMA_TRIGGER_ADC12IFGx,        //dma_trigger_t trigger, 
					DMA_EDGE_SENSITIVE,           //dma_level_t level,
					(void *)ADC12MEM0_,            //void *src_addr, 
					(void *)destAddr,              //void *dst_addr, 
					NUM_ADC_CHANS,                //uint16_t size,
					DMA_WORD,                     //dma_byte_t src_byte, 
					DMA_WORD,                     //dma_byte_t dst_byte,
					DMA_ADDRESS_INCREMENTED,      //dma_incr_t src_incr, 
					DMA_ADDRESS_INCREMENTED);     //dma_incr_t dst_incr

    call Msp430DmaChannel.startTransfer();
  }
  async event void Msp430DmaChannel.transferDone(error_t success) {
  }
  async event void HplAdc12.conversionDone(uint16_t iv) {
  }
}
  
  
