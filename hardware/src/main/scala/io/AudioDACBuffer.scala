// FIFO buffer for audio output to WM8731 Audio coded.


package io

import chisel3._
import chisel3.util._

class AudioDACBuffer(AUDIOBITLENGTH: Int, MAXDACBUFFERPOWER: Int) extends Module {

  // IOs
  val io = new Bundle {
    // to/from PATMOS
    val audioLIPatmos = Input(UInt(AUDIOBITLENGTH.W))
    val audioRIPatmos = Input(UInt(AUDIOBITLENGTH.W))
    val enDacI = Input(UInt(1.W)) // enable signal
    val writePulseI = Input(UInt(1.W))
    val fullO = Output(UInt(1.W)) // full buffer indicator
    val bufferSizeI = Input(UInt((MAXDACBUFFERPOWER + 1).W)) // maximum bufferSizeI: (2^MAXDACBUFFERPOWER) + 1
    // to/from AudioDAC
    val audioLIDAC = Output(UInt(AUDIOBITLENGTH.W))
    val audioRIDAC = Output(UInt(AUDIOBITLENGTH.W))
    val enDacO = Output(UInt(1.W)) // enable signal
    val writeEnDacI = Input(UInt(1.W)) // used to sync writes
    val convEndI = Input(UInt(1.W)) // indicates end of conversion
  }

  val BUFFERLENGTH: Int = (Math.pow(2, MAXDACBUFFERPOWER)).asInstanceOf[Int]

  //Registers for output audio data
  val audioLIReg = RegInit(init = 0.U(AUDIOBITLENGTH.W))
  val audioRIReg = RegInit(init = 0.U(AUDIOBITLENGTH.W))
  io.audioLIDAC := audioLIReg
  io.audioRIDAC := audioRIReg

  //FIFO buffer registers
  val audioBufferL = RegInit(VecInit(Seq.fill(BUFFERLENGTH)(0.U(AUDIOBITLENGTH.W))))
  val audioBufferR = RegInit(VecInit(Seq.fill(BUFFERLENGTH)(0.U(AUDIOBITLENGTH.W))))
  val w_pnt = RegInit(init = 0.U(MAXDACBUFFERPOWER.W))
  val r_pnt = RegInit(init = 0.U(MAXDACBUFFERPOWER.W))
  val fullReg = RegInit(init = 0.U(1.W))
  val emptyReg = RegInit(init = 1.U(1.W)) // starts empty
  io.fullO := fullReg
  val w_inc = RegInit(init = 0.U(1.W)) // write pointer increment
  val r_inc = RegInit(init = 0.U(1.W)) // read pointer increment


  // output handshake state machine
  val sOutIdle :: sOutWrote :: Nil = Enum(2)
  val stateOut = RegInit(init = sOutIdle)

  // input handshake state machine
  val sInIdle :: sInWriting :: Nil = Enum(2)
  val stateIn = RegInit(init = sInIdle)

  // full and empty state machine
  val sFEIdle :: sFEAlmostFull :: sFEFull :: sFEAlmostEmpty :: sFEEmpty :: Nil = Enum(5)
  val stateFE = RegInit(init = sFEEmpty)

  //output enable register: For AudioDAC and for output conversion
  val enOutReg = RegInit(init = 0.U(1.W)) //starts low because its empty
  io.enDacO := enOutReg
  val lastOutputReg = RegInit(init = 0.U(1.W)) // indicator of last output conversion

  // state machine for last output conversion
  val sCEWaitFirst :: sCEFirstPulse :: sCEWaitSecond :: Nil = Enum(3)
  val stateCE = RegInit(init = sCEWaitFirst)

  // register to keep track of buffer size
  val bufferSizeReg = RegInit(init = 0.U((MAXDACBUFFERPOWER + 1).W)) //update buffer size register
  when(bufferSizeReg =/= io.bufferSizeI) {
    bufferSizeReg := io.bufferSizeI
    r_pnt := r_pnt & (io.bufferSizeI - 1.U)
    w_pnt := w_pnt & (io.bufferSizeI - 1.U)
  }

  // audio output handshake: if output handshake enabled
  when(enOutReg === 1.U) { //state machine
    switch(stateOut) {
      is(sOutIdle) { //wait until posEdge writeEnDacI
        when(io.writeEnDacI === 1.U) { // write only when its not empty (for last conversion case)
          when(emptyReg === 0.U) { //write output, increment read pointer
            audioLIReg := audioBufferL(r_pnt)
            audioRIReg := audioBufferR(r_pnt)
            r_pnt := (r_pnt + 1.U) & (io.bufferSizeI - 1.U)
            r_inc := 1.U
          } //update state
          stateOut := sOutWrote
        }
      }
      is(sOutWrote) { //wait until negEdge writeEnDacI
        when(io.writeEnDacI === 0.U) { //update state
          stateOut := sOutIdle
        }
      }
    }
  }.otherwise {
    stateOut := sOutIdle
    r_inc := 0.U
  }



  // audio input handshake: if enable and not full
  when((io.enDacI === 1.U) && (fullReg === 0.U)) { //state machine
    switch(stateIn) {
      is(sInIdle) {
        when(io.writePulseI === 1.U) {
          audioBufferL(w_pnt) := io.audioLIPatmos
          audioBufferR(w_pnt) := io.audioRIPatmos
          stateIn := sInWriting
        }
      }
      is(sInWriting) {
        when(io.writePulseI === 0.U) {
          w_pnt := (w_pnt + 1.U) & (io.bufferSizeI - 1.U)
          w_inc := 1.U
          stateIn := sInIdle
        }
      }
    }
  }.otherwise {
    stateIn := sInIdle
  }



  //update output handshake enable register
  when(emptyReg === 0.U) { //if not empty, always enable
    enOutReg := 1.U
  }.otherwise { // empty
    when(lastOutputReg === 1.U) { // if last output conversion, enable
      enOutReg := 1.U
    }.otherwise {
      enOutReg := 0.U
    }
  }

  // when last conversion finishes:
  when(lastOutputReg === 1.U) { // if stateFE is not empty anymore, or if it is but conversion ends
    when(stateFE =/= sFEEmpty) { // if state is not empty anymore
      lastOutputReg := 0.U
    }.otherwise { // state machine to detect 2nd convEndI pulse
      switch(stateCE) {
        is(sCEWaitFirst) {
          when(io.convEndI === 1.U) {
            stateCE := sCEFirstPulse
          }
        }
        is(sCEFirstPulse) {
          when(io.convEndI === 0.U) {
            stateCE := sCEWaitSecond
          }
        }
        is(sCEWaitSecond) {
          when(io.convEndI === 1.U) {
            lastOutputReg := 0.U
            stateCE := sCEWaitFirst
          }
        }
      }
    }
  }.otherwise {
    stateCE := sCEWaitFirst
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
          lastOutputReg := 1.U // indicator of last output conversion
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
