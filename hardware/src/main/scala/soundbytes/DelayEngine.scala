package soundbytes

import scala.math._
import chisel3._
import chisel3.util._

/**
 * The basic delay engine required for delay and feedback computation based on two parallel mixers.
 * May not be the most ideal setup but should be sufficient for now.
 *
 * The registerInputs flag adds input registers.
 */
class DelayEngine(dataWidth: Int = 16, mixWidth: Int = 6, feedbackWidth: Int = 6, registerInputs: Boolean = false) extends Module {
  val io = IO(new Bundle {
    val in = Flipped(Decoupled(Vec(2, SInt(dataWidth.W))))
    val out = Decoupled(Vec(2, SInt(dataWidth.W)))
    val mix = Input(UInt(mixWidth.W))
    val feedback = Input(UInt(feedbackWidth.W))
  })
  
  val delayMix = Module(new Mixer(dataWidth, mixWidth))
  val feedbackMix = Module(new Mixer(dataWidth, feedbackWidth))

  val inData = if(registerInputs) {
    RegNext(io.in.bits)
  } else {
    io.in.bits
  }
  val inValid = if (registerInputs) {
    val regValid = RegInit(false.B)
    when (io.in.ready & io.in.valid) {
      regValid := true.B
    }
    when (regValid) {
      regValid := false.B
    }
    regValid
  } else {
    io.in.valid
  }
  
  delayMix.io.in.valid := inValid
  delayMix.io.mix := io.mix
  feedbackMix.io.in.valid := inValid
  feedbackMix.io.mix := io.feedback
  if (registerInputs) {
    io.in.ready := delayMix.io.in.ready & feedbackMix.io.in.ready & !inValid
  } else {
    io.in.ready := delayMix.io.in.ready & feedbackMix.io.in.ready
  }
  
  io.out.valid := delayMix.io.out.valid & feedbackMix.io.out.valid
  delayMix.io.out.ready := io.out.ready
  feedbackMix.io.out.ready := io.out.ready
  
  // Input is always present in the output signal (delay < 1)
  delayMix.io.in.bits(0) := inData(0)
  delayMix.io.in.bits(1) := inData(1)
  io.out.bits(0) := delayMix.io.out.bits
  
  // Input is always present in the delay output signal (feedback < 1)
  feedbackMix.io.in.bits(0) := inData(0)
  feedbackMix.io.in.bits(1) := inData(1)
  io.out.bits(1) := feedbackMix.io.out.bits
}

/**
 * Variant of the above delay engine using the piplined mixers.
 */
class DelayEnginePipelined(dataWidth: Int = 16, mixWidth: Int = 6, feedbackWidth: Int = 6) extends Module {
  val io = IO(new Bundle {
    val in = Flipped(Valid(Vec(2, SInt(dataWidth.W))))
    val out = Valid(Vec(2, SInt(dataWidth.W)))
    val mix = Input(UInt(mixWidth.W))
    val feedback = Input(UInt(feedbackWidth.W))
  })
  
  val delayMix = Module(new MixerPipelined(dataWidth, mixWidth))
  val feedbackMix = Module(new MixerPipelined(dataWidth, feedbackWidth))
  
  delayMix.io.in.valid := io.in.valid
  delayMix.io.mix := io.mix
  feedbackMix.io.in.valid := io.in.valid
  feedbackMix.io.mix := io.feedback
  
  io.out.valid := delayMix.io.out.valid & feedbackMix.io.out.valid
  
  // Input is always present in the output signal (delay < 1)
  delayMix.io.in.bits(0) := io.in.bits(0)
  delayMix.io.in.bits(1) := io.in.bits(1)
  io.out.bits(0) := delayMix.io.out.bits
  
  // Input is always present in the delay output signal (feedback < 1)
  feedbackMix.io.in.bits(0) := io.in.bits(0)
  feedbackMix.io.in.bits(1) := io.in.bits(1)
  io.out.bits(1) := feedbackMix.io.out.bits
}
