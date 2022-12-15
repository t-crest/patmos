package cmp

import Chisel.{Bundle, Vec}
import ocp.OcpCoreSlavePort
import patmos.Constants.{ADDR_WIDTH, DATA_WIDTH}

class CmpIO(val corecnt: Int) extends Bundle {
  val cores = Vec(corecnt, new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH))
}
