package cmp

import chisel3._
import ocp.OcpCoreSlavePort
import patmos.Constants.{ADDR_WIDTH, DATA_WIDTH}

class CmpIO(val corecnt: Int) extends Bundle {
  val cores = Vec(corecnt, new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH))
}
