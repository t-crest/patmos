// FIFO buffer for audio input from WM8731 Audio coded.


package io

import Chisel._

class AudioADCBuffer(AUDIOBITLENGTH: Int, MAXADCBUFFERPOWER: Int) extends Module {

  // IOs
  val io = new Bundle {
    // to/from AudioADC
    val audioLAdcI = UInt(INPUT, AUDIOBITLENGTH)
    val audioRAdcI = UInt(INPUT, AUDIOBITLENGTH)
    val enAdcO = UInt(OUTPUT, 1)
    val readEnAdcI = UInt(INPUT, 1) //used to sync reads
    // to/from PATMOS
    val enAdcI = UInt(INPUT, 1)
    val audioLPatmosO = UInt(OUTPUT, AUDIOBITLENGTH)
    val audioRPatmosO = UInt(OUTPUT, AUDIOBITLENGTH)
    val reqO = UInt(OUTPUT, 1) // handshake REQ
    val ackI = UInt(INPUT, 1) // handshake ACK
    val bufferSizeI = UInt(INPUT, MAXADCBUFFERPOWER+1) // maximum bufferSizeI: (2^MAXADCBUFFERPOWER) + 1
  }

  val BUFFERLENGTH : Int = (Math.pow(2, MAXADCBUFFERPOWER)).asInstanceOf[Int]

  //Registers for output audio data (to PATMOS)
  val audioLReg = Reg(init = UInt(0, AUDIOBITLENGTH))
  val audioRReg = Reg(init = UInt(0, AUDIOBITLENGTH))
  io.audioLPatmosO := audioLReg
  io.audioRPatmosO := audioRReg

  //FIFO buffer registers
  val audioBufferL = Vec.fill(BUFFERLENGTH) { Reg(init = UInt(0, AUDIOBITLENGTH)) }
  val audioBufferR = Vec.fill(BUFFERLENGTH) { Reg(init = UInt(0, AUDIOBITLENGTH)) }
  val w_pnt = Reg(init = UInt(0, MAXADCBUFFERPOWER))
  val r_pnt = Reg(init = UInt(0, MAXADCBUFFERPOWER))
  val fullReg  = Reg(init = UInt(0, 1))
  val emptyReg = Reg(init = UInt(1, 1)) // starts empty
  val w_inc = Reg(init = UInt(0, 1)) // write pointer increment
  val r_inc = Reg(init = UInt(0, 1)) // read pointer increment

  // input handshake state machine (from AudioADC)
  val sInIdle :: sInRead :: Nil = Enum(UInt(), 2)
  val stateIn = Reg(init = sInIdle)

  // output handshake state machine (to Patmos)
  val sOutIdle :: sOutReqHi :: sOutAckHi :: sOutReqLo :: Nil = Enum(UInt(), 4)
  val stateOut = Reg(init = sOutIdle)

  // full and empty state machine
  val sFEIdle :: sFEAlmostFull :: sFEFull :: sFEAlmostEmpty :: sFEEmpty :: Nil = Enum(UInt(), 5)
  val stateFE = Reg(init = sFEEmpty)

  // register to keep track of buffer size
  val bufferSizeReg = Reg(init = UInt(0, MAXADCBUFFERPOWER+1))
  //update buffer size register
  when(bufferSizeReg =/= io.bufferSizeI) {
    bufferSizeReg := io.bufferSizeI
    r_pnt := r_pnt & (io.bufferSizeI - UInt(1))
    w_pnt := w_pnt & (io.bufferSizeI - UInt(1))
  }

  // output enable: just wire from input enable
  io.enAdcO := io.enAdcI

  // audio input handshake: if enable
  when (io.enAdcI === UInt(1)) {
    //state machine
    switch (stateIn) {
      is (sInIdle) {
        //wait until posEdge readEnAdcI
        when(io.readEnAdcI === UInt(1)) {
          // read only when its not full
          when(fullReg === UInt(0)) {
            //read input, increment write pointer
            audioBufferL(w_pnt) := io.audioLAdcI
            audioBufferR(w_pnt) := io.audioRAdcI
            w_pnt := (w_pnt + UInt(1)) & (io.bufferSizeI - UInt(1))
            w_inc := UInt(1)
          }
          //update state
          stateIn := sInRead
        }
      }
      is (sInRead) {
        //wait until negEdge readEnAdcI
        when(io.readEnAdcI === UInt(0)) {
          //update state
          stateIn := sInIdle
        }
      }
    }
  }
  .otherwise {
    stateIn := sInIdle
    w_inc := UInt(0)
  }



  // audio output handshake: if enable
  when (io.enAdcI === UInt(1)) {
    //state machine
    switch (stateOut) {
      is (sOutIdle) {
        io.reqO := UInt(0)
        //present data, but don't increment
        audioLReg := audioBufferL(r_pnt)
        audioRReg := audioBufferR(r_pnt)
        // update state if ack is low, and not empty
        when( (io.ackI === UInt(0)) && (emptyReg === UInt(0)) ) {
          stateOut := sOutReqHi
        }
      }
      is (sOutReqHi) {
        io.reqO := UInt(1)
        when(io.ackI === UInt(1)) {
          stateOut := sOutAckHi
        }
      }
      is (sOutAckHi) {
        io.reqO := UInt(1)
        // now yes, increment pointers, read signal
        r_pnt := (r_pnt + UInt(1)) & (io.bufferSizeI - UInt(1))
        r_inc := UInt(1)
        stateOut := sOutReqLo
      }
      is (sOutReqLo) {
        io.reqO := UInt(0)
        when(io.ackI === UInt(0)) {
          //update state
          stateOut := sOutIdle
        }
      }
    }
  }
  .otherwise {
    stateOut := sOutIdle
    io.reqO := UInt(0)
    r_inc := UInt(0)
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
        when( (w_inc === UInt(1)) && (w_pnt === ( (r_pnt - UInt(1)) & (io.bufferSizeI - UInt(1)) ) ) && (r_inc === UInt(0)) ) {
          stateFE := sFEAlmostFull
        }
        .elsewhen( (r_inc === UInt(1)) && (r_pnt === ( (w_pnt - UInt(1)) & (io.bufferSizeI - UInt(1)) ) ) && (w_inc === UInt(0)) ) {
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
          fullReg := UInt(1)
        }
      }
      is(sFEFull) {
        fullReg  := UInt(1)
        emptyReg := UInt(0)
        when( (r_inc === UInt(1)) && (w_inc === UInt(0)) ) {
          stateFE := sFEAlmostFull
          fullReg := UInt(0)
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
          emptyReg := UInt(1)
        }
      }
      is(sFEEmpty) {
        fullReg  := UInt(0)
        emptyReg := UInt(1)
        when( (w_inc === UInt(1)) && (r_inc === UInt(0)) ) {
          stateFE := sFEAlmostEmpty
          emptyReg := UInt(0)
        }
      }
    }
  }
}
