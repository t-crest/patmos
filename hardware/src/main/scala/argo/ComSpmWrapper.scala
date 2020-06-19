/*
 * Blackbox for ~/t-crest/argo/src/mem/com_spm.vhd. 
 * Requires ~/t-crest/argo/src/mem/com_spm_wrapper.vhd
 *
 * Authors: Eleftherios Kyriakakis (elky@dtu.dk)
 *
 */

package argo

import Chisel._
import ocp._
import patmos.Constants._

/**
  * Dummy for ~/t-crest/argo/src/mem/com_spm_wrapper.vhd
  * It emulates a single-port SPM for testing Patmos access
  * @param argoConf
  */
class ComSpmDummy(argoConf: ArgoConfig) extends Module {
  val io = new Bundle(){
    val ocp = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
    val spm = new SPMSlavePort(argoConf.HEADER_FIELD_WIDTH, argoConf.HEADER_CTRL_WIDTH)
  }
  val cmdReg = Reg(next = io.ocp.M.Cmd)
  val dataOut = Reg(UInt(width = DATA_WIDTH))
  val memOCP = Mem(UInt(width = DATA_WIDTH), argoConf.SPM_BYTES)
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
class ComSpmWrapper(argoConf: ArgoConfig) extends BlackBox {
 // throw new Error("BlackBox wrapper for ComSpm needs update for Chisel 3")
  // should be commented out in order to compile for Chisel3
  setModuleName("com_spm_wrapper")
  addClock(Driver.implicitClock)
  renameClock("clk", "clk")
  renameReset("reset")
  val io = IO(new Bundle(){
    val ocp = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
    val spm = new SPMSlavePort(argoConf.HEADER_FIELD_WIDTH, argoConf.HEADER_CTRL_WIDTH)
  })
  //setVerilogParameters("#(.SPM_IDX_SIZE(" + argoConf.SPM_IDX_SIZE + "))")  //Commented out to compile with Chisel3
}
