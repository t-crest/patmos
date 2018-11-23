
/*
 * EthMac interface for Patmos
 *
 * Author: Luca Pezzarossa (lpez@dtu.dk)
 *
 */

package io

import Chisel._
import ocp._

import ptp1588assist._

object EthMac extends DeviceObject {
  var extAddrWidth = 32
  var dataWidth = 32
  var withPTP = false
  var initialTime = 0L
  var secondsWidth = 0
  var nanoWidth = 0

  def init(params : Map[String, String]) = {
    extAddrWidth = getPosIntParam(params, "extAddrWidth")
    dataWidth = getPosIntParam(params, "dataWidth")
    withPTP = getBoolParam(params, "withPTP")
    if(withPTP){
      initialTime = 1522763228L
      secondsWidth = getPosIntParam(params, "secondsWidth")
      nanoWidth = getPosIntParam(params, "nanoWidth")
    }
  }

  def create(params: Map[String, String]) : EthMac = {
    if(withPTP)
      Module(new EthMac(extAddrWidth-1, dataWidth, withPTP, secondsWidth, nanoWidth, initialTime))
    else
      Module(new EthMac(extAddrWidth, dataWidth))
  }

  trait Pins {
    val ethMacPins = new Bundle() {
      // Tx
      val mtx_clk_pad_i = Bool(INPUT)  // Transmit clock (from PHY)
      val mtxd_pad_o    = Bits(OUTPUT, width = 4) // Transmit niethle (to PHY)
      val mtxen_pad_o   = Bits(OUTPUT, width = 1) // Transmit enable (to PHY)
      val mtxerr_pad_o  = Bits(OUTPUT, width = 1) // Transmit error (to PHY)

      // Rx
      val mrx_clk_pad_i = Bool(INPUT) // Receive clock (from PHY)
      val mrxd_pad_i    = Bits(INPUT, width = 4) // Receive niethle (from PHY)
      val mrxdv_pad_i   = Bits(INPUT, width = 1) // Receive data valid (from PHY)
      val mrxerr_pad_i  = Bits(INPUT, width = 1) // Receive data error (from PHY)

      // Common Tx and Rx
      val mcoll_pad_i   = Bits(INPUT, width = 1) // Collision (from PHY)
      val mcrs_pad_i    = Bits(INPUT, width = 1) // Carrier sense (from PHY)

      // MII Management interface
      val md_pad_i      = Bits(INPUT, width = 1)  // MII data input (from I/O cell)
      val mdc_pad_o     = Bits(OUTPUT, width = 1) // MII Management data clock (to PHY)
      val md_pad_o      = Bits(OUTPUT, width = 1) // MII data output (to I/O cell)
      val md_padoe_o    = Bits(OUTPUT, width = 1) // MII data output enable (to I/O cell)

      // PTP Debug Signals
      val ptpPPS = Bits(OUTPUT, width=1)
      val ledPHY = Bits(OUTPUT, width=1)
      val ledSOF = Bits(OUTPUT, width=1)
      val ledEOF = Bits(OUTPUT, width=1)
      val ledSFD = Bits(OUTPUT, width=8)
      // val rtcDisp = Vec.fill(8) {Bits(OUTPUT, 7)}
    }
  }

  trait Intrs{
    val ethMacIntrs = Vec.fill(3) { Bool(OUTPUT) }
  }
}

class EthMacBB(extAddrWidth : Int = 32, dataWidth : Int = 32) extends BlackBox {
  val io = new OcpCoreSlavePort(extAddrWidth, dataWidth) with EthMac.Pins
  // rename component
  setModuleName("eth_controller_top")

  // rename signals
  renameClock(clock, "clk")
  reset.setName("rst")

  io.M.Cmd.setName("MCmd")
  io.M.Addr.setName("MAddr")
  io.M.Data.setName("MData")
  io.M.ByteEn.setName("MByteEn")
  io.S.Resp.setName("SResp")
  io.S.Data.setName("SData")

  io.ethMacPins.mtx_clk_pad_i.setName("mtx_clk_pad_i")
  io.ethMacPins.mtxd_pad_o.setName("mtxd_pad_o")
  io.ethMacPins.mtxen_pad_o.setName("mtxen_pad_o")
  io.ethMacPins.mtxerr_pad_o.setName("mtxerr_pad_o")
  io.ethMacPins.mrx_clk_pad_i.setName("mrx_clk_pad_i")
  io.ethMacPins.mrxd_pad_i.setName("mrxd_pad_i")
  io.ethMacPins.mrxdv_pad_i.setName("mrxdv_pad_i")
  io.ethMacPins.mrxerr_pad_i.setName("mrxerr_pad_i")
  io.ethMacPins.mcoll_pad_i.setName("mcoll_pad_i")
  io.ethMacPins.mcrs_pad_i.setName("mcrs_pad_i")
  io.ethMacPins.md_pad_i.setName("md_pad_i")
  io.ethMacPins.mdc_pad_o.setName("mdc_pad_o")
  io.ethMacPins.md_pad_o.setName("md_pad_o")
  io.ethMacPins.md_padoe_o.setName("md_padoe_o")

  // set Verilog parameters
  setVerilogParameters("#(.BUFF_ADDR_WIDTH("+extAddrWidth+"))")

  // keep some sigals for emulation
  debug(io.M.Cmd)
  debug(io.M.Addr)
  debug(io.M.Data)

  // registers to help emulation
  val respReg = Reg(Bits(width = 2))
  val dataReg = Reg(Bits(width = dataWidth))
  io.S.Resp := respReg
  io.S.Data := dataReg
}

class EthMac(extAddrWidth: Int = 32, dataWidth: Int = 32, withPTP: Boolean = false, secondsWidth: Int = 32, nanoWidth: Int = 32, initialTime: BigInt = 0L) extends CoreDevice() {
  override val io = new CoreDeviceIO() with EthMac.Pins with EthMac.Intrs

  val eth = Module(new EthMacBB(extAddrWidth, dataWidth))
  eth.io.ethMacPins <> io.ethMacPins

  if(withPTP) {    
    println("EthMac w/ PTP1588 hardware (eth_addrWidth="+extAddrWidth+", ptp_addrWidth="+(extAddrWidth)+")")
    val ptp = Module(new PTP1588Assist(addrWidth = extAddrWidth, dataWidth = dataWidth, secondsWidth = secondsWidth, nanoWidth = nanoWidth, initialTime = initialTime))
    val masterReg = Reg(next = io.ocp.M)
    eth.io.M.Data := masterReg.Data
    eth.io.M.ByteEn := masterReg.ByteEn
    eth.io.M.Addr := masterReg.Addr
    ptp.io.ocp.M.Data := masterReg.Data
    ptp.io.ocp.M.ByteEn := masterReg.ByteEn
    ptp.io.ocp.M.Addr := masterReg.Addr
    //Arbitrate OCP master
    when(masterReg.Addr(15, 12) === Bits("hE")){
      eth.io.M.Cmd := OcpCmd.IDLE
      ptp.io.ocp.M.Cmd := masterReg.Cmd   //PTP
    }.otherwise{
      eth.io.M.Cmd := masterReg.Cmd       //EthMac
      ptp.io.ocp.M.Cmd := OcpCmd.IDLE
    }
    //Arbitrate OCP slave based on response
    val replyRegPTP = Reg(next = ptp.io.ocp.S)
    val replyRegETH = Reg(next = eth.io.S)
    when(replyRegETH.Resp =/= OcpResp.NULL){
      io.ocp.S := replyRegETH             //ETH
    }.elsewhen(replyRegPTP.Resp =/= OcpResp.NULL){
      io.ocp.S.Resp := replyRegPTP.Resp   //PTP
      io.ocp.S.Data := replyRegPTP.Data
    }.otherwise{
      io.ocp.S.Resp := OcpResp.NULL
      io.ocp.S.Data := 0.U                //NONE
    }
    //Rest of IO connections
    ptp.io.ethMacRX.clk := io.ethMacPins.mrx_clk_pad_i
    ptp.io.ethMacRX.data := io.ethMacPins.mrxd_pad_i
    ptp.io.ethMacRX.dv := io.ethMacPins.mrxdv_pad_i
    ptp.io.ethMacRX.err := io.ethMacPins.mrxerr_pad_i
    ptp.io.ethMacTX.clk := eth.io.ethMacPins.mtx_clk_pad_i
    ptp.io.ethMacTX.data := eth.io.ethMacPins.mtxd_pad_o
    ptp.io.ethMacTX.dv := eth.io.ethMacPins.mtxen_pad_o
    ptp.io.ethMacTX.err := eth.io.ethMacPins.mtxerr_pad_o
    io.ethMacIntrs := ptp.io.intrs
    io.ethMacPins.ptpPPS := ptp.io.rtcPPS
    io.ethMacPins.ledPHY := ptp.io.ledPHY
    io.ethMacPins.ledSOF := ptp.io.ledSOF
    io.ethMacPins.ledEOF := ptp.io.ledEOF
    io.ethMacPins.ledSFD := ptp.io.ledSFD
  } else {
    println("EthMac (eth_addrWidth="+extAddrWidth+")")
    eth.io.M <> io.ocp.M
    eth.io.S <> io.ocp.S
    io.ethMacIntrs := false.B
  }
}


