package argo

import Chisel._
import ocp._
import patmos.Constants._

/**
  * Wrapper for ~/t-crest/argo/src/noc/synchronous/noc_node_wrapper.vhd
  * @param argoConf
  */
class NoCNodeWrapper(argoConf: ArgoConfig, master: Boolean) extends BlackBox {
  setModuleName("noc_node_wrapper")
  renameClock(clock, "clk")
  reset.setName("reset")
  if(master) {
    setVerilogParameters("#(.MASTER(" + 1 + "))")
  }
  val io = new Bundle(){
    val irq = Bits(width = 2).asOutput()
    val run = Bool(INPUT)
    val supervisor = Bool(INPUT)
    val masterRun = Bool(OUTPUT)
    val proc = new OcpIOSlavePort(ADDR_WIDTH, DATA_WIDTH)
    val spm = new SPMMasterPort(argoConf.HEADER_FIELD_WIDTH, argoConf.HEADER_CTRL_WIDTH)
    val north_in = new RouterPort(argoConf)
    val east_in = new RouterPort(argoConf)
    val south_in = new RouterPort(argoConf)
    val west_in = new RouterPort(argoConf)
    val north_out = new OutputPort(argoConf)
    val east_out = new OutputPort(argoConf)
    val south_out = new OutputPort(argoConf)
    val west_out = new OutputPort(argoConf)
  }
}