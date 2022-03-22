package soundbytes

import scala.math._
import chisel3._
import chisel3.util._

/**
 * Basic variable linear gain module (outputWidth must not be larger than dataWidth + gainWidth).
 */
class Gain(dataWidth: Int = 16, gainWidth: Int = 6, outputWidth: Int = 16) extends Module {
  val io = IO(new Bundle {
    val in = Flipped(Decoupled(SInt(dataWidth.W)))
    val out = Decoupled(SInt(outputWidth.W))
    val gain = Input(UInt(gainWidth.W))
  })

  // State Variables.
  val idle :: distort :: hasValue :: Nil = Enum(3)
  val regState = RegInit(idle)

  // Output Code.
  val regMul = RegInit(0.S(outputWidth.W))
  io.out.bits := regMul

  // FSM.
  io.in.ready := regState === idle
  io.out.valid := regState === hasValue
  switch (regState) {
    is (idle) {
      when(io.in.valid) {
        regMul := (io.in.bits * io.gain) >> (dataWidth + gainWidth - outputWidth)
        regState := hasValue
      }
    }
    is (hasValue) {
      when(io.out.ready) {
        regState := idle
      }
    }
  }
}

/*
 * Pipelined variant of the above gain unit.
 */
class GainPipelined(dataWidth: Int = 16, gainWidth: Int = 6, outputWidth: Int = 16) extends Module {
  val io = IO(new Bundle {
    val in = Flipped(Valid(SInt(dataWidth.W)))
    val out = Valid(SInt(outputWidth.W))
    val gain = Input(UInt(gainWidth.W))
  })

  
  val regMul = RegInit(0.S(outputWidth.W))
  val regValid = RegInit(false.B)

  regMul := (io.in.bits * io.gain) >> (dataWidth + gainWidth - outputWidth)
  regValid := io.in.valid
  
  io.out.bits := regMul
  io.out.valid := regValid
}
