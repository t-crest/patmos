package cmp

import chisel3._
import ocp.OcpCoreSlavePort
import patmos.Constants.{ADDR_WIDTH, DATA_WIDTH}

class CmpIO(val corecnt: Int) extends Bundle with patmos.HasPins {
  val cores = Vec(corecnt, new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH))
  // TODO: just for now to move to Chisel 3.5, needs a btter fix in the future
  override val pins = new Bundle {
    val tx = Output(Bits(1.W))
    val rx = Input(Bits(1.W))
  }
}
