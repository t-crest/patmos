package cmp

import chisel3._

/**
  * Basis for a multicore (CMP) device.
  * All Cmp devices shall inherit from this.
  * @param cnt
  */
abstract class CmpDevice(cnt: Int) extends Module {
  val io = IO(new CmpIO(cnt))
}
