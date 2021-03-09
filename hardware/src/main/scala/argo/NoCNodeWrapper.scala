/*
 * Blackbox for ~/t-crest/argo/src/noc/synchronous/noc_node.vhd. 
 * Requires ~/t-crest/argo/src/noc/synchronous/noc_node_wrapper.vhd
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
  * Dummy for ~/t-crest/argo/src/noc/synchronous/noc_node_wrapper.vhd
  * It emulates a same-cycle Ocp.RD and a delayed accept Ocp.WR
  * it emulates three different Argo config_bus registers for DMA_BASE, SCHED_BASE and TDM_BASE 
  * @param argoConf
  * @param master
  */
class NoCNodeDummy(val argoConf: ArgoConfig, master: Boolean) extends Module {
  val io = IO(new Bundle(){
    val irq = Output(UInt(2.W))
    val run = Input(Bool())
    val supervisor = Input(Bool())
    val masterRun = Output(Bool())
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
  })

  val dmaReg = RegInit(Integer.parseInt("0000", 16).U(DATA_WIDTH.W))
  val schReg = RegInit(Integer.parseInt("2000", 16).U(DATA_WIDTH.W))
  val tdmReg = RegInit(Integer.parseInt("4000", 16).U(DATA_WIDTH.W))
  val respReg = RegInit(OcpResp.NULL)
  val acceptReg = RegInit(false.B)

  acceptReg := (io.proc.M.Cmd === OcpCmd.WR) && ~acceptReg

  when (io.proc.M.Cmd===OcpCmd.WR && acceptReg) {
    when(io.proc.M.Addr(15, 12) === 0.U){
      dmaReg := io.proc.M.Data
      respReg := OcpResp.DVA
    }.elsewhen (io.proc.M.Addr(15, 12) === 2.U) {
      schReg := io.proc.M.Data
      respReg := OcpResp.DVA
    } .elsewhen(io.proc.M.Addr(15, 12) === 4.U){
      tdmReg := io.proc.M.Data
      respReg := OcpResp.DVA
    } .otherwise{
      respReg := OcpResp.ERR
    }
  } .otherwise {
    respReg := OcpResp.NULL
  }

  when(io.proc.M.Addr(15, 12) === 0.U){
    io.proc.S.Data := dmaReg 
  }.elsewhen (io.proc.M.Addr(15, 12) === 2.U) {
    io.proc.S.Data := schReg
  } .elsewhen(io.proc.M.Addr(15, 12) === 4.U){
    io.proc.S.Data := tdmReg 
  } .otherwise{
    io.proc.S.Data := 0.U
  }

  io.proc.S.CmdAccept := Mux(io.proc.M.Cmd===OcpCmd.WR, acceptReg, io.proc.M.Cmd===OcpCmd.RD)
  io.proc.S.Resp := Mux(io.proc.M.Cmd===OcpCmd.WR, respReg, Mux(io.proc.M.Cmd===OcpCmd.RD, OcpResp.DVA, OcpResp.NULL))

  //debug(io.proc.M.Addr) does nothing in chisel3 (no proning in frontend of chisel3 anyway)
  //debug(io.proc.M.RespAccept)
}

/**
  * Wrapper for ~/t-crest/argo/src/noc/synchronous/noc_node_wrapper.vhd
  * @param argoConf
  * @param master
  */
class NoCNodeWrapper(val argoConf: ArgoConfig, master: Boolean) extends  BlackBox(Map("MASTER" -> (if (master) 1 else 0))) {
  val io = IO(new Bundle(){
    val irq = Output(UInt(2.W))
    val run = Input(Bool())
    val supervisor = Input(Bool())
    val masterRun = Output(Bool())
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
    val clk = Input(Clock())
    val reset = Input(Reset())
  })
  // rename signals
  override def desiredName: String = "noc_node_wrapper"
  io.clk.suggestName("clk")
  io.reset.suggestName("reset")
}
