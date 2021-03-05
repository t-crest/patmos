package argo

import chisel3._
import chisel3.util._
import patmos.Constants._
import ocp._

// Wrapper for an aegean generated noc_wrapper_2x2.vhd
class NoCWrapper(argoConf: ArgoConfig) extends BlackBox {
  val io =  IO(new Bundle(){
    val irq = Output(UInt(8.W))
    val supervisor = Input(UInt(4.W))
    val ocpPorts = Vec(4, new OcpIOSlavePort(ADDR_WIDTH, DATA_WIDTH))
    val spmPorts = Vec(4, new SPMMasterPort(16, 2))
    val clk = Input(Clock())
    val reset = Input(Reset())
  })
  // rename signals
  override def desiredName: String = "noc_wrapper_2x2"
  io.clk.suggestName("clk")
  io.reset.suggestName("reset")
  io.supervisor.suggestName("supervisor")
  io.irq.suggestName("irq")
}
