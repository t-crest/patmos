/*
   Copyright 2013 Technical University of Denmark, DTU Compute.
   All rights reserved.

   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

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
  setModuleName("com_spm_wrapper")
  addClock(Driver.implicitClock)
  renameClock("clk", "clk")
  renameReset("reset")
  val io = new Bundle(){
    val ocp = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
    val spm = new SPMSlavePort(argoConf.HEADER_FIELD_WIDTH, argoConf.HEADER_CTRL_WIDTH)
  }
  setVerilogParameters("#(.SPM_IDX_SIZE(" + argoConf.SPM_IDX_SIZE + "))")
}
