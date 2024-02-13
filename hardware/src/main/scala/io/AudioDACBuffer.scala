// FIFO buffer for audio output to WM8731 Audio coded.


package io

import Chisel._
import chisel3.VecInit

class AudioDACBuffer(AUDIOBITLENGTH: Int, MAXDACBUFFERPOWER: Int) extends Module {

  // IOs
  val io = new Bundle {
    // to/from PATMOS
    val audioLIPatmos = Input(UInt(AUDIOBITLENGTH.W))
    val audioRIPatmos = Input(UInt(AUDIOBITLENGTH.W))
    val enDacI = Input(UInt(1.W)) // enable signal
    val writePulseI = Input(UInt(1.W))
    val fullO = Output(UInt(1.W)) // full buffer indicator
    val bufferSizeI = Input(UInt((MAXDACBUFFERPOWER+1).W)) // maximum bufferSizeI: (2^MAXDACBUFFERPOWER) + 1
    // to/from AudioDAC
    val audioLIDAC = Output(UInt(AUDIOBITLENGTH.W))
    val audioRIDAC = Output(UInt(AUDIOBITLENGTH.W))
    val enDacO = Output(UInt(1.W)) // enable signal
    val writeEnDacI = Input(UInt(1.W)) // used to sync writes
    val convEndI = Input(UInt(1.W)) // indicates end of conversion
  }

  val BUFFERLENGTH : Int = (Math.pow(2, MAXDACBUFFERPOWER)).asInstanceOf[Int]

  //Registers for output audio data
  val audioLIReg = Reg(init = UInt(0, AUDIOBITLENGTH))
  val audioRIReg = Reg(init = UInt(0, AUDIOBITLENGTH))
  io.audioLIDAC := audioLIReg
  io.audioRIDAC := audioRIReg

  //FIFO buffer registers
  val audioBufferL = RegInit(VecInit(Seq.fill(BUFFERLENGTH)(0.U(AUDIOBITLENGTH.W))))
  val audioBufferR = RegInit(VecInit(Seq.fill(BUFFERLENGTH)(0.U(AUDIOBITLENGTH.W))))
  val w_pnt = Reg(init = UInt(0, MAXDACBUFFERPOWER))
  val r_pnt = Reg(init = UInt(0, MAXDACBUFFERPOWER))
  val fullReg  = Reg(init = UInt(0, 1))
  val emptyReg = Reg(init = UInt(1, 1)) // starts empty
  io.fullO := fullReg
  val w_inc = Reg(init = UInt(0, 1)) // write pointer increment
  val r_inc = Reg(init = UInt(0, 1)) // read pointer increment


  // output handshake state machine
  val sOutIdle :: sOutWrote :: Nil = Enum(UInt(), 2)
  val stateOut = Reg(init = sOutIdle)

  // input handshake state machine
  val sInIdle :: sInWriting :: Nil = Enum(UInt(), 2)
  val stateIn = Reg(init = sInIdle)

  // full and empty state machine
  val sFEIdle :: sFEAlmostFull :: sFEFull :: sFEAlmostEmpty :: sFEEmpty :: Nil = Enum(UInt(), 5)
  val stateFE = Reg(init = sFEEmpty)

  //output enable register: For AudioDAC and for output conversion
  val enOutReg = Reg(init = UInt(0, 1)) //starts low because its empty
  io.enDacO := enOutReg
  val lastOutputReg = Reg(init = UInt(0, 1)) // indicator of last output conversion

  // state machine for last output conversion
  val sCEWaitFirst :: sCEFirstPulse :: sCEWaitSecond :: Nil = Enum(UInt(), 3)
  val stateCE = Reg(init = sCEWaitFirst)

  // register to keep track of buffer size
  val bufferSizeReg = Reg(init = UInt(0, MAXDACBUFFERPOWER+1))
  //update buffer size register
  when(bufferSizeReg =/= io.bufferSizeI) {
    bufferSizeReg := io.bufferSizeI
    r_pnt := r_pnt & (io.bufferSizeI - UInt(1))
    w_pnt := w_pnt & (io.bufferSizeI - UInt(1))
  }

  // audio output handshake: if output handshake enabled
  when (enOutReg === UInt(1)) {
    //state machine
    switch (stateOut) {
      is (sOutIdle) {
        //wait until posEdge writeEnDacI
        when(io.writeEnDacI === UInt(1)) {
          // write only when its not empty (for last conversion case)
          when(emptyReg === UInt(0)) {
            //write output, increment read pointer
            audioLIReg := audioBufferL(r_pnt)
            audioRIReg := audioBufferR(r_pnt)
            r_pnt := (r_pnt + UInt(1)) & (io.bufferSizeI - UInt(1))
            r_inc := UInt(1)
          }
          //update state
          stateOut := sOutWrote
        }
      }
      is (sOutWrote) {
        //wait until negEdge writeEnDacI
        when(io.writeEnDacI === UInt(0)) {
          //update state
          stateOut := sOutIdle
        }
      }
    }
  }
  .otherwise {
    stateOut := sOutIdle
    r_inc := UInt(0)
  }



  // audio input handshake: if enable and not full
  when ( (io.enDacI === UInt(1)) && (fullReg === UInt(0)) ) {
    //state machine
    switch (stateIn) {
      is (sInIdle) {
        when(io.writePulseI === UInt(1)) {
          audioBufferL(w_pnt) := io.audioLIPatmos
          audioBufferR(w_pnt) := io.audioRIPatmos
          stateIn := sInWriting
        }
      }
      is (sInWriting) {
        when(io.writePulseI === UInt(0)) {
          w_pnt := (w_pnt + UInt(1)) & (io.bufferSizeI - UInt(1))
          w_inc := UInt(1)
          stateIn := sInIdle
        }
      }
    }
  }
  .otherwise {
    stateIn := sInIdle
  }



  //update output handshake enable register
  when (emptyReg === UInt(0)) { //if not empty, always enable
    enOutReg := UInt(1)
  }
  .otherwise { // empty
    when (lastOutputReg === UInt(1)) { // if last output conversion, enable
      enOutReg := UInt(1)
    }
    .otherwise {
      enOutReg := UInt(0)
    }
  }

  // when last conversion finishes:
  when(lastOutputReg === UInt(1)) {
    // if stateFE is not empty anymore, or if it is but conversion ends
    when(stateFE =/= sFEEmpty) { // if state is not empty anymore
      lastOutputReg := UInt(0)
    }
    .otherwise { // state machine to detect 2nd convEndI pulse
      switch (stateCE) {
        is (sCEWaitFirst) {
          when(io.convEndI === UInt(1)) {
            stateCE := sCEFirstPulse
          }
        }
        is (sCEFirstPulse) {
          when(io.convEndI === UInt(0)) {
            stateCE := sCEWaitSecond
          }
        }
        is (sCEWaitSecond) {
          when(io.convEndI === UInt(1)) {
            lastOutputReg := UInt(0)
            stateCE := sCEWaitFirst
          }
        }
      }
    }
  }
  .otherwise {
    stateCE := sCEWaitFirst
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
          lastOutputReg := UInt(1) // indicator of last output conversion
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
