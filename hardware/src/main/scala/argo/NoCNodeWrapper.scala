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
 * Blackbox for ~/t-crest/argo/src/noc/synchronous/noc_node.vhd. 
 * Requires ~/t-crest/argo/src/noc/synchronous/noc_node_wrapper.vhd
 *
 * Authors: Eleftherios Kyriakakis (elky@dtu.dk)
 *
 */

package argo

import Chisel._
import ocp._
import patmos.Constants._

/**
  * Dummy for ~/t-crest/argo/src/noc/synchronous/noc_node_wrapper.vhd
  * It emulates a same-cycle Ocp.RD and a delayed accept Ocp.WR
  * it emulates three different Argo config_bus registers for DMA_BASE, SCHED_BASE and TDM_BASE 
  * @param argoConf
  * @param master
  */
class NoCNodeDummy(argoConf: ArgoConfig, master: Boolean) extends Module {
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

  val dmaReg = Reg(init = UInt(Bits("h0000"), width = DATA_WIDTH))
  val schReg = Reg(init = UInt(Bits("h2000"), width = DATA_WIDTH))
  val tdmReg = Reg(init = UInt(Bits("h4000"), width = DATA_WIDTH))
  val respReg = Reg(init = OcpResp.NULL)
  val acceptReg = Reg(init = false.B)

  acceptReg := (io.proc.M.Cmd === OcpCmd.WR) && ~acceptReg

  when (io.proc.M.Cmd===OcpCmd.WR && acceptReg) {
    when(io.proc.M.Addr(15, 12) === Bits("h0")){
      dmaReg := io.proc.M.Data
      respReg := OcpResp.DVA
    }.elsewhen (io.proc.M.Addr(15, 12) === Bits("h2")) {
      schReg := io.proc.M.Data
      respReg := OcpResp.DVA
    } .elsewhen(io.proc.M.Addr(15, 12) === Bits("h4")){
      tdmReg := io.proc.M.Data
      respReg := OcpResp.DVA
    } .otherwise{
      respReg := OcpResp.ERR
    }
  } .otherwise {
    respReg := OcpResp.NULL
  }

  when(io.proc.M.Addr(15, 12) === Bits("h0")){
    io.proc.S.Data := dmaReg 
  }.elsewhen (io.proc.M.Addr(15, 12) === Bits("h2")) {
    io.proc.S.Data := schReg
  } .elsewhen(io.proc.M.Addr(15, 12) === Bits("h4")){
    io.proc.S.Data := tdmReg 
  } .otherwise{
    io.proc.S.Data := Bits("h0")
  }

  io.proc.S.CmdAccept := Mux(io.proc.M.Cmd===OcpCmd.WR, acceptReg, io.proc.M.Cmd===OcpCmd.RD)
  io.proc.S.Resp := Mux(io.proc.M.Cmd===OcpCmd.WR, respReg, Mux(io.proc.M.Cmd===OcpCmd.RD, OcpResp.DVA, OcpResp.NULL))

  debug(io.proc.M.Addr)
  debug(io.proc.M.RespAccept)
}

/**
  * Wrapper for ~/t-crest/argo/src/noc/synchronous/noc_node_wrapper.vhd
  * @param argoConf
  * @param master
  */
class NoCNodeWrapper(argoConf: ArgoConfig, master: Boolean) extends BlackBox {
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
  setModuleName("noc_node_wrapper")
  addClock(Driver.implicitClock)
  renameClock("clk", "clk")
  renameReset("reset")
  if(master) {
    setVerilogParameters("#(.MASTER(" + 1 + "))")
  } else {
    setVerilogParameters("#(.MASTER(" + 0 + "))")
  }
}