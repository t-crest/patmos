// FIFO buffer for audio input from WM8731 Audio coded.


package io

import chisel3._
import chisel3.util._

class AudioADCBuffer(AUDIOBITLENGTH: Int, MAXADCBUFFERPOWER: Int) extends Module {

  // IOs
  val io = new Bundle {
    // to/from AudioADC
    val audioLAdcI = Input(UInt(AUDIOBITLENGTH.W))
    val audioRAdcI = Input(UInt(AUDIOBITLENGTH.W))
    val enAdcO = Output(UInt(1.W))
    val readEnAdcI = Input(UInt(1.W)) //used to sync reads
    // to/from PATMOS
    val enAdcI = Input(UInt(1.W))
    val audioLPatmosO = Output(UInt(AUDIOBITLENGTH.W))
    val audioRPatmosO = Output(UInt(AUDIOBITLENGTH.W))
    val readPulseI = Input(UInt(1.W))
    val emptyO = Output(UInt(1.W)) // empty buffer indicator
    val bufferSizeI = Input(UInt((MAXADCBUFFERPOWER + 1).W)) // maximum bufferSizeI: (2^MAXADCBUFFERPOWER) + 1
  }

  val BUFFERLENGTH: Int = (Math.pow(2, MAXADCBUFFERPOWER)).asInstanceOf[Int]

  //Registers for output audio data (to PATMOS)
  val audioLReg = RegInit(init = 0.U(AUDIOBITLENGTH.W))
  val audioRReg = RegInit(init = 0.U(AUDIOBITLENGTH.W))
  io.audioLPatmosO := audioLReg
  io.audioRPatmosO := audioRReg

  //FIFO buffer registers
  val audioBufferL = RegInit(VecInit(Seq.fill(BUFFERLENGTH)(0.U(AUDIOBITLENGTH.W))))
  val audioBufferR = RegInit(VecInit(Seq.fill(BUFFERLENGTH)(0.U(AUDIOBITLENGTH.W))))
  val w_pnt = RegInit(init = 0.U(MAXADCBUFFERPOWER.W))
  val r_pnt = RegInit(init = 0.U(MAXADCBUFFERPOWER.W))
  val fullReg = RegInit(init = 0.U(1.W))
  val emptyReg = RegInit(init = 1.U(1.W)) // starts empty
  io.emptyO := emptyReg
  val w_inc = RegInit(init = 0.U(1.W)) // write pointer increment
  val r_inc = RegInit(init = 0.U(1.W)) // read pointer increment

  // input handshake state machine (from AudioADC)
  val sInIdle :: sInRead :: Nil = Enum(2)
  val stateIn = RegInit(init = sInIdle)
  //counter for input handshake
  val readCntReg = RegInit(init = 0.U(3.W))
  val READCNTLIMIT = 3.U

  // output handshake state machine (to Patmos)
  val sOutIdle :: sOutReading :: Nil = Enum(2)
  val stateOut = RegInit(init = sOutIdle)

  // full and empty state machine
  val sFEIdle :: sFEAlmostFull :: sFEFull :: sFEAlmostEmpty :: sFEEmpty :: Nil = Enum(5)
  val stateFE = RegInit(init = sFEEmpty)

  // register to keep track of buffer size
  val bufferSizeReg = RegInit(init = 0.U((MAXADCBUFFERPOWER + 1).W)) //update buffer size register
  when(bufferSizeReg =/= io.bufferSizeI) {
    bufferSizeReg := io.bufferSizeI
    r_pnt := r_pnt & (io.bufferSizeI - 1.U)
    w_pnt := w_pnt & (io.bufferSizeI - 1.U)
  }

  // output enable: just wire from input enable
  io.enAdcO := io.enAdcI

  // audio input handshake: if enable
  when(io.enAdcI === 1.U) { //state machine
    switch(stateIn) {
      is(sInIdle) { //wait until posEdge readEnAdcI
        when(io.readEnAdcI === 1.U) { //wait READCNTLIMIT cycles until input data is written
          when(readCntReg === READCNTLIMIT) { //read input, increment write pointer
            audioBufferL(w_pnt) := io.audioLAdcI
            audioBufferR(w_pnt) := io.audioRAdcI
            w_pnt := (w_pnt + 1.U) & (io.bufferSizeI - 1.U)
            w_inc := 1.U //if it is full, write, but increment read pointer too
            //to store new samples and dump older ones
            when(fullReg === 1.U) {
              r_pnt := (r_pnt + 1.U) & (io.bufferSizeI - 1.U)
              r_inc := 1.U
            } //update state
            stateIn := sInRead
          }.otherwise {
            readCntReg := readCntReg + 1.U
          }
        }
      }
      is(sInRead) {
        readCntReg := 0.U //wait until negEdge readEnAdcI
        when(io.readEnAdcI === 0.U) { //update state
          stateIn := sInIdle
        }
      }
    }
  }.otherwise {
    readCntReg := 0.U
    stateIn := sInIdle
    w_inc := 0.U
  }



  // audio output state machine: if enable and not empty
  when((io.enAdcI === 1.U) && (emptyReg === 0.U)) { //state machine
    switch(stateOut) {
      is(sOutIdle) {
        when(io.readPulseI === 1.U) {
          audioLReg := audioBufferL(r_pnt)
          audioRReg := audioBufferR(r_pnt)
          stateOut := sOutReading
        }
      }
      is(sOutReading) {
        when(io.readPulseI === 0.U) {
          r_pnt := (r_pnt + 1.U) & (io.bufferSizeI - 1.U)
          r_inc := 1.U
          stateOut := sOutIdle
        }
      }
    }
  }.otherwise {
    stateOut := sOutIdle
  }



  //update full and empty states
  when((w_inc === 1.U) || (r_inc === 1.U)) { //default: set back variables
    w_inc := 0.U
    r_inc := 0.U //state machine
    switch(stateFE) {
      is(sFEIdle) {
        fullReg := 0.U
        emptyReg := 0.U
        when((w_inc === 1.U) && (w_pnt === ((r_pnt - 1.U) & (io.bufferSizeI - 1.U))) && (r_inc === 0.U)) {
          stateFE := sFEAlmostFull
        }.elsewhen((r_inc === 1.U) && (r_pnt === ((w_pnt - 1.U) & (io.bufferSizeI - 1.U))) && (w_inc === 0.U)) {
          stateFE := sFEAlmostEmpty
        }
      }
      is(sFEAlmostFull) {
        fullReg := 0.U
        emptyReg := 0.U
        when((r_inc === 1.U) && (w_inc === 0.U)) {
          stateFE := sFEIdle
        }.elsewhen((w_inc === 1.U) && (r_inc === 0.U)) {
          stateFE := sFEFull
          fullReg := 1.U
        }
      }
      is(sFEFull) {
        fullReg := 1.U
        emptyReg := 0.U
        when((r_inc === 1.U) && (w_inc === 0.U)) {
          stateFE := sFEAlmostFull
          fullReg := 0.U
        }
      }
      is(sFEAlmostEmpty) {
        fullReg := 0.U
        emptyReg := 0.U
        when((w_inc === 1.U) && (r_inc === 0.U)) {
          stateFE := sFEIdle
        }.elsewhen((r_inc === 1.U) && (w_inc === 0.U)) {
          stateFE := sFEEmpty
          emptyReg := 1.U
        }
      }
      is(sFEEmpty) {
        fullReg := 0.U
        emptyReg := 1.U
        when((w_inc === 1.U) && (r_inc === 0.U)) {
          stateFE := sFEAlmostEmpty
          emptyReg := 0.U
        }
      }
    }
  }
}
