package soundbytes

import scala.math._
import chisel3._
import chisel3.util._

/**
 * Multiplexer for Decoupled interfaces.
 */
class DecoupledMux[T <: Data](gen : T) extends Module {
  val io = IO(new Bundle {
    val in1 = Flipped(Decoupled(gen))
    val in2 = Flipped(Decoupled(gen))
    val sel = Input(Bool())
    val out = Decoupled(gen)
  })
  
  // Default assignments for ready.
  io.in1.ready := false.B
  io.in2.ready := false.B
  
  // The actual multiplexer.
  when(io.sel) {
    io.out <> io.in1
  }.otherwise {
    io.out <> io.in2
  }
}
