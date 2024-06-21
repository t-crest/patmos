//I2C interface to write into the configuration registers of WM8731


package io

import chisel3._
import chisel3.util._

class AudioI2C extends Module {
  //constant: I2C slave address
  val SLAVEADDR = "b0011010"
  //IOs
  val io = new Bundle {
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
  val dataReg = RegInit(init = 0.U(9.W))
  val addrReg = RegInit(init = 0.U(7.W))

  //connect inputs to registers
  dataReg := io.dataI
  addrReg := io.addrI

  //clock divider for SCLK
  val sclkCntReg = RegInit(init = 0.U(10.W)) //10 bit counter
  val sclkCntMax = (400 / 2).U // 80 MHz -> 400 cycles -> 200 kHz
  val sclkCntMax2 = 400.U // Only used for finishing conditions

  val wordCntReg = RegInit(init = 0.U(3.W)) //counter register for each word: 8 bit counter (000 to 111)

  //register for SCLK
  val sclkReg = RegInit(init = 1.U(1.W)) //default high
  io.sclkO := sclkReg
  //register for SDINO
  val sdinReg = RegInit(init = 1.U(1.W)) //default high
  io.sdinO := sdinReg
  //register for WEO
  val weReg = RegInit(init = 1.U(1.W)) //default high: master controls SDIN
  io.weO := weReg

  //register for SLAVEADDR: constant
  val rAddrReg = RegInit(init = SLAVEADDR.U(7.W))

  //states
  val sIdle :: sStart :: sAck1 :: sDataMsb :: sAck2 :: sDataLsb :: sAck3 :: sFinish1 :: sFinish2 :: sFinishPatmos :: Nil = Enum(10)
  //state register
  val state = RegInit(init = sIdle)

  //Default values:
  io.ackO := 0.U

  when(io.reqI === 1.U) { //PATMOS starts handshake

    //initial transition: idle to start
    when(state === sIdle) {
      weReg := 1.U
      sdinReg := 0.U
      state := sStart
    }.elsewhen(state === sFinish2) { //counter for SCLK
        sclkCntReg := sclkCntReg + 1.U
        when(sclkCntReg === sclkCntMax) {
          sclkReg := 1.U
        }.elsewhen(sclkCntReg === sclkCntMax2) {
          sclkCntReg := 0.U
          sdinReg := 1.U
          io.ackO := 1.U //to PATMOS
          state := sFinishPatmos
        }
      }.elsewhen(state === sFinishPatmos) {
        io.ackO := 1.U //to PATMOS
      }

      //in any other state, enable counter and everything
      .otherwise { //counter for SCLK
        sclkCntReg := sclkCntReg + 1.U //when limit - 1 -> switch
        when(sclkCntReg === (sclkCntMax - 1.U)) {
          sclkReg := ~sclkReg
        } //when limit -> restart counter
        .elsewhen(sclkCntReg === sclkCntMax) {
          sclkCntReg := 0.U //update state machine only at falling edge
          when(sclkReg === 0.U) {
            switch(state) {
              is(sStart) {
                when(wordCntReg < 7.U) {
                  sdinReg := rAddrReg(6.U - wordCntReg)
                  wordCntReg := wordCntReg + 1.U
                }.otherwise { //wordCntReg === 7.U)
                  sdinReg := 0.U //for write
                  wordCntReg := 0.U
                  state := sAck1
                }
              }
              is(sAck1) {
                weReg := 0.U
                sdinReg := 1.U //doesn't really matter
                state := sDataMsb

              }
              is(sDataMsb) {
                weReg := 1.U //check if WM8731 responded with ACK correctly on the first CC of this state
                when((sclkCntReg === 0.U) && (io.sdinI === 1.U)) { //in this case, reset
                  sclkCntReg := 0.U
                  state := sIdle
                } //if it ACK is correct (sdin === 0), then proceed
                when(wordCntReg < 7.U) {
                  sdinReg := addrReg(6.U - wordCntReg)
                  wordCntReg := wordCntReg + 1.U
                }.otherwise { //wordCntReg === 7.U)
                  sdinReg := dataReg(8.U)
                  wordCntReg := 0.U
                  state := sAck2
                }
              }
              is(sAck2) {
                weReg := 0.U
                sdinReg := 1.U //doesn't really matter
                state := sDataLsb
              }
              is(sDataLsb) {
                weReg := 1.U //check if WM8731 responded with ACK correctly on the first CC of this state
                when((sclkCntReg === 0.U) && (io.sdinI === 1.U)) { //in this case, reset
                  sclkCntReg := 0.U
                  state := sIdle
                } //if it ACK is correct (sdin === 0), then proceed
                when(wordCntReg < 7.U) {
                  sdinReg := dataReg(7.U - wordCntReg)
                  wordCntReg := wordCntReg + 1.U
                }.otherwise { //wordCntReg === 7.U)
                  sdinReg := dataReg(0.U)
                  wordCntReg := 0.U
                  state := sAck3
                }
              }
              is(sAck3) {
                weReg := 0.U
                sdinReg := 0.U //I think it matters on this case
                state := sFinish1
              } //added
              is(sFinish1) {
                weReg := 1.U
                sdinReg := 0.U //check if WM8731 responded with ACK correctly
                when(io.sdinI === 0.U) {
                  state := sFinish2
                }.otherwise { // if no ack
                  state := sIdle
                }
              }
            }
          }
        }
      }
  }.otherwise { //io.reqI === 0.U
    when(state === sFinishPatmos) {
      state := sIdle
    }
  }
}
