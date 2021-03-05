/*
 * Blackbox for ~/t-crest/argo/src/mem/com_spm.vhd. 
 * Requires ~/t-crest/argo/src/mem/com_spm_wrapper.vhd
 *
 * Authors: Eleftherios Kyriakakis (elky@dtu.dk)
 *
 */

package argo

import chisel3._
import chisel3.util._
import ocp._
import patmos.Constants._


/**
  * Dummy for ~/t-crest/argo/src/mem/com_spm_wrapper.vhd
  * It emulates a single-port SPM for testing Patmos access
  * @param argoConf
  */
class ComSpmDummy(val argoConf: ArgoConfig) extends Module {
  val io = new Bundle(){
    val ocp = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
    val spm = new SPMSlavePort(argoConf.HEADER_FIELD_WIDTH, argoConf.HEADER_CTRL_WIDTH)
  }
  val cmdReg = RegNext(io.ocp.M.Cmd)
  val dataOut = Reg(UInt(DATA_WIDTH.W))
  val memOCP = Mem(argoConf.SPM_BYTES, UInt(DATA_WIDTH.W))
  when (io.ocp.M.Cmd===OcpCmd.WR) { 
    memOCP.write(io.ocp.M.Addr, io.ocp.M.Data)
  }
  when (io.ocp.M.Cmd===OcpCmd.RD) { 
    dataOut := memOCP.read(io.ocp.M.Addr)
  }
  io.ocp.S.Data := dataOut
  io.ocp.S.Resp := Mux(cmdReg === OcpCmd.WR || cmdReg === OcpCmd.RD, OcpResp.DVA, OcpResp.NULL)
}

/**
  * Wrapper for ~/t-crest/argo/src/mem/com_spm_wrapper.vhd
  * @param argoConf
  */
class ComSpmWrapper(val argoConf: ArgoConfig) extends BlackBox(Map("SPM_IDX_SIZE" -> argoConf.SPM_IDX_SIZE)) {
  val io = IO(new Bundle(){
    val ocp = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
    val spm = new SPMSlavePort(argoConf.HEADER_FIELD_WIDTH, argoConf.HEADER_CTRL_WIDTH)
    val clk = Input(Clock())
    val reset = Input(Reset())
  })
  // rename signals
  override def desiredName: String = "com_spm_wrapper"
  io.clk.suggestName("clk")
  io.reset.suggestName("reset")
}
