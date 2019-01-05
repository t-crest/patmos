package argo

import Chisel._
import patmos.Constants._
import ocp._

// Wrapper for an aegean generated noc_wrapper_2x2.vhd
class NoCWrapper(argoConf: ArgoConfig) extends BlackBox {
  val io = new Bundle(){
    val irq = Bits(OUTPUT, width = 8)
    val supervisor = Bits(INPUT, width = 4)
    val ocpPorts = Vec.fill(4) {
      new OcpIOSlavePort(ADDR_WIDTH, DATA_WIDTH)
    }
    val spmPorts = Vec.fill(4) {
      new SPMMasterPort(16, 2)
    }
  }
  setModuleName("noc_wrapper_2x2")
  addClock(Driver.implicitClock)
  renameClock("clk", "clk")
  renameReset("reset")
  io.supervisor.setName("supervisor")
  io.irq.setName("irq")
}
