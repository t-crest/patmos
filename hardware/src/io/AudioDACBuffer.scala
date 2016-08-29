// FIFO buffer for audio output to WM8731 Audio coded.


package io

import Chisel._

class AudioDACBuffer(AUDIOBITLENGTH: Int, BUFFERPOWER: Int) extends Module {

  // IOs
  val io = new Bundle {
    // to/from PATMOS
    val audioLIPatmos = UInt(INPUT, AUDIOBITLENGTH)
    val audioRIPatmos = UInt(INPUT, AUDIOBITLENGTH)
    val enDacI = UInt(INPUT, 1) // enable signal
    val reqI = UInt(INPUT, 1)  // handshake REQ
    val ackO = UInt(OUTPUT, 1) // handshake ACK
    // to/from AudioDAC
    val audioLIDAC = UInt(OUTPUT, AUDIOBITLENGTH)
    val audioRIDAC = UInt(OUTPUT, AUDIOBITLENGTH)
    val enDacO = UInt(OUTPUT, 1) // enable signal
    val dacLrcI = UInt(INPUT, 1) // used to sync
    //val busyDac = UInt(INPUT, 1) //needed???
  }

  val BUFFERLENGTH : Int = (Math.pow(2, BUFFERPOWER)).asInstanceOf[Int]

  //Registers for output audio data
  val audioLIReg = Reg(init = UInt(0, AUDIOBITLENGTH))
  val audioRIReg = Reg(init = UInt(0, AUDIOBITLENGTH))
  io.audioLIDAC := audioLIReg
  io.audioRIDAC := audioRIReg

  //FIFO buffer registers
  val audioBufferL = Vec.fill(BUFFERLENGTH) { Reg(init = UInt(0, AUDIOBITLENGTH)) }
  val audioBufferR = Vec.fill(BUFFERLENGTH) { Reg(init = UInt(0, AUDIOBITLENGTH)) }
  val w_pnt = Reg(init = UInt(0, BUFFERPOWER))
  val r_pnt = Reg(init = UInt(0, BUFFERPOWER))
  val fullReg  = Reg(init = UInt(0, 1))
  val emptyReg = Reg(init = UInt(1, 1)) // starts empty
  val w_inc = Reg(init = UInt(0, 1)) // write pointer increment
  val r_inc = Reg(init = UInt(0, 1)) // read pointer increment


  // output handshake state machine
  val sOutIdle :: sOutWrote :: Nil = Enum(UInt(), 2)
  val stateOut = Reg(init = sOutIdle)

  // input handshake state machine
  val sInIdle :: sInReqHi :: sInAckHi :: sInReqLo :: Nil = Enum(UInt(), 4)
  val stateIn = Reg(init = sInIdle)

  // full and empty state machine
  val sFEIdle :: sFEAlmostFull :: sFEFull :: sFEAlmostEmpty :: sFEEmpty :: Nil = Enum(UInt(), 5)
  val stateFE = Reg(init = sFEIdle)

  // audio output handshake: if input enable and buffer not empty
  when ( (io.enDacI === UInt(1)) && (emptyReg === UInt(0)) ) {
    //enable output
    io.enDacO := UInt(1)
    //state machine
    switch (stateOut) {
      is (sOutIdle) {
        //wait until posEdge dacLrcI
        when(io.dacLrcI === UInt(1)) {
          //write output, increment read pointer
          audioLIReg := audioBufferL(r_pnt)
          audioRIReg := audioBufferR(r_pnt)
          r_pnt := r_pnt + UInt(1)
          r_inc := UInt(1)
          //update state
          stateOut := sOutWrote
        }
      }
      is (sOutWrote) {
        //wait until negEdge dacLrcI
        when(io.dacLrcI === UInt(0)) {
          //update state
          stateOut := sOutIdle
        }
      }
    }
  }
  .otherwise {
    io.enDacO := UInt(0)
    stateOut := sOutIdle
  }

  // audio input handshake: if enable
  when (io.enDacI === UInt(1)) {
    //state machine
    switch (stateIn) {
      is (sInIdle) {
        io.ackO := UInt(0)
        when(io.reqI === UInt(1)) {
          //update state
          stateIn := sInReqHi
        }
      }
      is (sInReqHi) {
        //check if buffer is not full
        when(fullReg === UInt(0)) {
          stateIn := sInAckHi
        }
        //check if PATMOS cancels handshake
        when(io.reqI === UInt(0)) {
          stateIn := sInIdle
        }
      }
      is (sInAckHi) {
        io.ackO := UInt(1)
        when(io.reqI === UInt(0)) {
          stateIn := sInReqLo
        }
      }
      is (sInReqLo) {
        when(fullReg === UInt(0)) { // for safety, wait until buffer not full
          //read and store input, increment write pointer
          audioBufferL(w_pnt) := io.audioLIPatmos
          audioBufferR(w_pnt) := io.audioRIPatmos
          w_pnt := w_pnt + UInt(1)
          w_inc := UInt(1)
          //update state
          stateIn := sInIdle
        }
      }
    }
  }
  .otherwise {
    stateIn := sInIdle
    io.ackO := UInt(0)
  }

  //update full and empty states
  when ( (w_inc === UInt(1)) || (r_inc === UInt(1)) ) {
    //default: set back variables
    w_inc := UInt(0)
    r_inc := UInt(0)
    //state machine
    switch (stateFE) {
      is (sFEIdle) {
        fullReg  := UInt(0)
        emptyReg := UInt(0)
        when( (w_inc === UInt(1)) && (w_pnt === (r_pnt - UInt(1))) && (r_inc === UInt(0)) ) {
          stateFE := sFEAlmostFull
        }
        .elsewhen( (r_inc === UInt(1)) && (r_pnt === (w_pnt - UInt(1))) && (w_inc === UInt(0)) ) {
          stateFE := sFEAlmostEmpty
        }
      }
      is(sFEAlmostFull) {
        fullReg  := UInt(0)
        emptyReg := UInt(0)
        when( (r_inc === UInt(1)) && (w_inc === UInt(0)) ) {
          stateFE := sFEIdle
        }
        .elsewhen( (w_inc === UInt(1)) && (r_inc === UInt(0)) ) {
          stateFE := sFEFull
        }
      }
      is(sFEFull) {
        fullReg  := UInt(1)
        emptyReg := UInt(0)
        when( (r_inc === UInt(1)) && (w_inc === UInt(0)) ) {
          stateFE := sFEAlmostFull
        }
      }
      is(sFEAlmostEmpty) {
        fullReg  := UInt(0)
        emptyReg := UInt(0)
        when( (w_inc === UInt(1)) && (r_inc === UInt(0)) ) {
          stateFE := sFEIdle
        }
        .elsewhen( (r_inc === UInt(1)) && (w_inc === UInt(0)) ) {
          stateFE := sFEEmpty
        }
      }
      is(sFEEmpty) {
        fullReg  := UInt(0)
        emptyReg := UInt(1)
        when( (w_inc === UInt(1)) && (r_inc === UInt(0)) ) {
          stateFE := sFEAlmostEmpty
        }
      }
    }
  }
}
