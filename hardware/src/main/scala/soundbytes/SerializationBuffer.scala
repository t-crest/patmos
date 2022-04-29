package soundbytes

import scala.math._
import chisel3._
import chisel3.util._

/**
 * Serialization Buffer to serialize a Vec into its elements.
 */
class SerializationBuffer[T <: Data](gen: T, val size: Int) extends Module() {
  
  val io = IO(new Bundle {
    val in = Flipped(Decoupled(Vec(size, gen)))
    val out = Decoupled(gen)
  })
  
  // State.
  val empty :: full :: Nil = Enum(2)
  val regState = RegInit(empty)
  val counter = Counter(size)

  val regBuffer = Reg(Vec(size, gen))

  // FSM.
  io.in.ready := regState === empty
  io.out.valid := regState === full
  io.out.bits := regBuffer(counter.value)
  
  switch (regState) {
    is (empty) {
      when (io.in.valid) {
        regBuffer := io.in.bits
        regState := full
        counter.reset()
      }
    }
    is (full) {
      when (io.out.ready) {
        when (counter.inc()) {
          regState := empty
        }
      }
    }
  }
}
