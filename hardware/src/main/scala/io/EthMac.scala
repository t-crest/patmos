
/*
 * EthMac interface for Patmos
 *
 * Author: Luca Pezzarossa (lpez@dtu.dk)
 *         Eleftherios Kyriakakis (elky@dtu.dk)
 *
 */

package io

import Chisel._
import ocp._
import patmos.Constants.CLOCK_FREQ
import ptp1588assist._

object EthMac extends DeviceObject {
  var extAddrWidth = 32
  var dataWidth = 32
  var withPTP = false
  var initialTime = 0L
  var secondsWidth = 32
  var nanoWidth = 32
  var ppsDuration = 25
  val currentTime: Long = System.currentTimeMillis / 1000

  def init(params : Map[String, String]) = {
    extAddrWidth = getPosIntParam(params, "extAddrWidth")
    dataWidth = getPosIntParam(params, "dataWidth")
    withPTP = getBoolParam(params, "withPTP")
    if(withPTP){
      initialTime = currentTime
      secondsWidth = getPosIntParam(params, "secondsWidth")
      nanoWidth = getPosIntParam(params, "nanoWidth")
      ppsDuration = getPosIntParam(params, "ppsDuration")
    }
  }

  def create(params: Map[String, String]) : EthMac = {
    if(withPTP)
      Module(new EthMac(extAddrWidth-1, dataWidth, withPTP, secondsWidth, nanoWidth, initialTime, ppsDuration))
    else
      Module(new EthMac(extAddrWidth, dataWidth))
  }

  trait Pins extends patmos.HasPins {
    override val pins = new Bundle() {
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

      val int_o         = Bits(OUTPUT, width = 1) // Ethernet intr output

      // PTP Debug Signals
      val ptpPPS = Bits(OUTPUT, width=1)
       val ledPHY = Bits(OUTPUT, width=1)
       val ledSOF = Bits(OUTPUT, width=1)
       val ledEOF = Bits(OUTPUT, width=1)
      // val ledSFD = Bits(OUTPUT, width=8)
      // val rtcDisp = Vec.fill(8) {Bits(OUTPUT, 7)}
    }
  }
}

class EthMacBB(extAddrWidth : Int = 32, dataWidth : Int = 32) extends BlackBox {
  val io = new OcpCoreSlavePort(extAddrWidth, dataWidth) with EthMac.Pins
  // rename component
  // TODO: Commented out to compile for chisel3
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

  io.pins.mtx_clk_pad_i.setName("mtx_clk_pad_i")
  io.pins.mtxd_pad_o.setName("mtxd_pad_o")
  io.pins.mtxen_pad_o.setName("mtxen_pad_o")
  io.pins.mtxerr_pad_o.setName("mtxerr_pad_o")
  io.pins.mrx_clk_pad_i.setName("mrx_clk_pad_i")
  io.pins.mrxd_pad_i.setName("mrxd_pad_i")
  io.pins.mrxdv_pad_i.setName("mrxdv_pad_i")
  io.pins.mrxerr_pad_i.setName("mrxerr_pad_i")
  io.pins.mcoll_pad_i.setName("mcoll_pad_i")
  io.pins.mcrs_pad_i.setName("mcrs_pad_i")
  io.pins.md_pad_i.setName("md_pad_i")
  io.pins.mdc_pad_o.setName("mdc_pad_o")
  io.pins.md_pad_o.setName("md_pad_o")
  io.pins.md_padoe_o.setName("md_padoe_o")
  io.pins.int_o.setName("int_o")
  
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

class EthMac(extAddrWidth: Int = 32, dataWidth: Int = 32, withPTP: Boolean = false, secondsWidth: Int = 32, nanoWidth: Int = 32, initialTime: BigInt = 0L, ppsDuration: Int = 10) extends CoreDevice() {
  override val io = new CoreDeviceIO() with EthMac.Pins with patmos.HasInterrupts {
    override val interrupts = Vec.fill(1) { Bool(OUTPUT) }
  }

  val eth = Module(new EthMacBB(extAddrWidth, dataWidth))
  //Wire IO pins straight through
  io.pins <> eth.io.pins

  // Connection to controller interrupt
  val syncEthIntrReg = RegNext(eth.io.pins.int_o)

  // Generate interrupts on rising edges
  val pulseEthIntrReg = RegNext(RegNext(syncEthIntrReg) === Bits("b0") && syncEthIntrReg(0) === Bits("b1"))
  io.interrupts := Cat(Bits("b0"), pulseEthIntrReg)

  //Check for PTP features
  if(withPTP) {    
    println("EthMac w/ PTP1588 hardware (eth_addrWidth="+extAddrWidth+", ptp_addrWidth="+(extAddrWidth)+")")
    val ptp = Module(new PTP1588Assist(extAddrWidth, dataWidth, CLOCK_FREQ, secondsWidth, nanoWidth, initialTime, ppsDuration))
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
    ptp.io.ethMacRX.clk := io.pins.mrx_clk_pad_i
    ptp.io.ethMacRX.data := io.pins.mrxd_pad_i
    ptp.io.ethMacRX.dv := io.pins.mrxdv_pad_i
    ptp.io.ethMacRX.err := io.pins.mrxerr_pad_i
    ptp.io.ethMacTX.clk := eth.io.pins.mtx_clk_pad_i
    ptp.io.ethMacTX.data := eth.io.pins.mtxd_pad_o
    ptp.io.ethMacTX.dv := eth.io.pins.mtxen_pad_o
    ptp.io.ethMacTX.err := eth.io.pins.mtxerr_pad_o
    io.pins.ptpPPS := ptp.io.rtcPPS
    io.pins.ledSOF := ptp.io.ledSOF
    io.pins.ledEOF := ptp.io.ledEOF
    io.pins.ledPHY := ptp.io.ledTS
  } else {
    println("EthMac (eth_addrWidth="+extAddrWidth+")")
    eth.io.M <> io.ocp.M
    eth.io.S <> io.ocp.S
    io.pins.ptpPPS := true.B
  }
}


