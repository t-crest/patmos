package soundbytes

import scala.math._
import chisel3._
import chisel3.util._

/**
 * Demultiplexer for Decoupled interfaces.
 */
class DecoupledDemux[T <: Data](gen : T) extends Module {
  val io = IO(new Bundle {
    val in = Flipped(Decoupled(gen))
    val sel = Input(Bool())
    val out1 = Decoupled(gen)
    val out2 = Decoupled(gen)
  })
  
  // Default assignments for ready.
  io.out1.valid := false.B
  io.out1.bits := DontCare
  io.out2.valid := false.B
  io.out2.bits := DontCare
  
  // The actual multiplexer.
  when(io.sel) {
    io.in <> io.out1
  }.otherwise {
    io.in <> io.out2
  }
}
