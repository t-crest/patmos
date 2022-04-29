package soundbytes

import scala.math._
import chisel3._
import chisel3.util._

/**
 * Deserialization Buffer to deserialize elements into a Vec.
 */
class DeserializationBuffer[T <: Data](gen: T, val size: Int) extends Module() {
  
  val io = IO(new Bundle {
    val in = Flipped(Decoupled(gen))
    val out = Decoupled(Vec(size, gen))
  })
  
  // State.
  val empty :: full :: Nil = Enum(2)
  val regState = RegInit(empty)
  val counter = Counter(size)

  val regBuffer = Reg(Vec(size, gen))

  // FSM.
  io.in.ready := regState === empty
  io.out.valid := regState === full
  io.out.bits := regBuffer
  
  switch (regState) {
    is (empty) {
      when (io.in.valid) {
        regBuffer(counter.value) := io.in.bits
        when (counter.inc()) {
          regState := full
        }
      }
    }
    is (full) {
      when (io.out.ready) {
        counter.reset()
        regState := empty        
      }
    }
  }
}
