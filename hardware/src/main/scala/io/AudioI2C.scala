//I2C interface to write into the configuration registers of WM8731


package io

import Chisel._

class AudioI2C extends Module
{
  //constant: I2C slave address
  val SLAVEADDR = "b0011010"
  //IOs
  val io = new Bundle
  {
    //inputs from PATMOS
    val dataI = Input(UInt(9.W))
    val addrI = Input(UInt(7.W))
    //wires to  to WM8731
    val sdinI = Input(UInt(1.W)) //inout
    val sdinO = Output(UInt(1.W)) //inout
    val sclkO = Output(UInt(1.W)) //output
    val weO = Output(UInt(1.W)) // write enable: 1->sdinO, 0->sdinI
		               //handshake with patmos
    val reqI = Input(UInt(1.W))
    val ackO = Output(UInt(1.W))
  }

  //registers for audio data
  val dataReg = Reg(init = UInt(0, 9))
  val addrReg = Reg(init = UInt(0, 7))

  //connect inputs to registers
  dataReg := io.dataI
  addrReg := io.addrI

  //clock divider for SCLK
  val sclkCntReg = Reg(init = UInt(0, 10)) //10 bit counter
  val sclkCntMax  = UInt(400/2) // 80 MHz -> 400 cycles -> 200 kHz
  val sclkCntMax2 = UInt(400) 	// Only used for finishing conditions

  val wordCntReg = Reg(init = UInt(0, 3)) //counter register for each word: 8 bit counter (000 to 111)

  //register for SCLK
  val sclkReg = Reg(init = UInt(1, 1)) //default high
  io.sclkO := sclkReg
  //register for SDINO
  val sdinReg = Reg(init = UInt(1, 1)) //default high
  io.sdinO := sdinReg
  //register for WEO
  val weReg = Reg(init = UInt(1, 1)) //default high: master controls SDIN
  io.weO := weReg

  //register for SLAVEADDR: constant
  val rAddrReg = Reg(init = UInt(SLAVEADDR, 7))

  //states
  val sIdle :: sStart :: sAck1 :: sDataMsb :: sAck2 :: sDataLsb :: sAck3 :: sFinish1 :: sFinish2 :: sFinishPatmos :: Nil = Enum(UInt(), 10)
  //state register
  val state = Reg(init = sIdle)

  //Default values:
  io.ackO 	:= UInt(0)

  when (io.reqI === UInt(1))
  { //PATMOS starts handshake

    //initial transition: idle to start
    when(state === sIdle) {
      weReg := UInt(1)
      sdinReg := UInt(0)
      state := sStart
    }
      .elsewhen(state === sFinish2) {
      //counter for SCLK
      sclkCntReg := sclkCntReg + UInt(1)
      when(sclkCntReg === sclkCntMax) {
	sclkReg := UInt(1)
      }
	.elsewhen(sclkCntReg === sclkCntMax2) {
	sclkCntReg := UInt(0)
	sdinReg := UInt(1)
	io.ackO := UInt(1) //to PATMOS
	state := sFinishPatmos
      }
    }
      .elsewhen(state === sFinishPatmos) {
      io.ackO := UInt(1) //to PATMOS
    }

    //in any other state, enable counter and everything
    .otherwise{
      //counter for SCLK
      sclkCntReg := sclkCntReg + UInt(1)
      //when limit - 1 -> switch
      when( sclkCntReg === (sclkCntMax - UInt(1)) ) {
	sclkReg := ~sclkReg
      }
      //when limit -> restart counter
	.elsewhen(sclkCntReg === sclkCntMax)
      {
	sclkCntReg := UInt(0)
	//update state machine only at falling edge
	when (sclkReg === UInt(0)) {
	  switch (state) {
	    is (sStart) {
	      when(wordCntReg < UInt(7)){
		sdinReg := rAddrReg(UInt(6)-wordCntReg)
		wordCntReg := wordCntReg + UInt(1)
	      }
		.otherwise {//wordCntReg === UInt(7))
		sdinReg := UInt(0) //for write
		wordCntReg := UInt(0)
		state := sAck1
	      }
	    }
	    is (sAck1) {
	      weReg := UInt(0)
	      sdinReg := UInt(1) //doesn't really matter
	      state := sDataMsb

	    }
	    is (sDataMsb) {
	      weReg := UInt(1)
	      //check if WM8731 responded with ACK correctly on the first CC of this state
	      when(  (sclkCntReg === UInt(0)) && (io.sdinI === UInt(1)) ) {
		//in this case, reset
		sclkCntReg := UInt(0)
		state := sIdle
	      }
	      //if it ACK is correct (sdin === 0), then proceed
	      when(wordCntReg < UInt(7)){
		sdinReg := addrReg(UInt(6)-wordCntReg)
		wordCntReg := wordCntReg + UInt(1)
	      }
		.otherwise {//wordCntReg === UInt(7))
		sdinReg := dataReg(UInt(8))
		wordCntReg := UInt(0)
		state := sAck2
	      }
	    }
	    is (sAck2) {
	      weReg := UInt(0)
	      sdinReg := UInt(1) //doesn't really matter
	      state := sDataLsb
	    }
	    is (sDataLsb) {
	      weReg := UInt(1)
	      //check if WM8731 responded with ACK correctly on the first CC of this state
	      when(  (sclkCntReg === UInt(0)) && (io.sdinI === UInt(1)) ) {
		//in this case, reset
		sclkCntReg := UInt(0)
		state := sIdle
	      }
	      //if it ACK is correct (sdin === 0), then proceed
	      when(wordCntReg < UInt(7)){
		sdinReg := dataReg(UInt(7)-wordCntReg)
		wordCntReg := wordCntReg + UInt(1)
	      }
		.otherwise {//wordCntReg === UInt(7))
		sdinReg := dataReg(UInt(0))
		wordCntReg := UInt(0)
		state := sAck3
	      }
	    }
	    is (sAck3) {
	      weReg := UInt(0)
	      sdinReg := UInt(0) //I think it matters on this case
	      state := sFinish1
	    }
	    //added
	    is (sFinish1) {
	      weReg := UInt(1)
	      sdinReg := UInt(0)
	      //check if WM8731 responded with ACK correctly
	      when(io.sdinI === UInt(0)) {
		state := sFinish2
	      }
		.otherwise { // if no ack
		state := sIdle
	      }
	    }
	  }
	}
      }
    }
  }
    .otherwise { //io.reqI === UInt(0)
    when (state === sFinishPatmos) {
      state := sIdle
    }
  }
}
