
/*
 * EthMac interface for Patmos
 *
 * Author: Luca Pezzarossa (lpez@dtu.dk)
 *         Eleftherios Kyriakakis (elky@dtu.dk)
 *
 */

package io

import Chisel._
import chisel3.internal.HasId
import ocp._
import patmos.Constants.CLOCK_FREQ
import ptp1588assist._

object EthMac extends DeviceObject {
  val currentTime: Long = System.currentTimeMillis / 1000
  var extAddrWidth = 32
  var dataWidth = 32
  var withPTP = false
  var initialTime = 0L
  var secondsWidth = 32
  var nanoWidth = 32
  var ppsDuration = 25

  def init(params: Map[String, String]) = {
    extAddrWidth = getPosIntParam(params, "extAddrWidth")
    dataWidth = getPosIntParam(params, "dataWidth")
    withPTP = getBoolParam(params, "withPTP")
    if (withPTP) {
      initialTime = currentTime
      secondsWidth = getPosIntParam(params, "secondsWidth")
      nanoWidth = getPosIntParam(params, "nanoWidth")
      ppsDuration = getPosIntParam(params, "ppsDuration")
    }
  }

  def create(params: Map[String, String]): EthMac = {
    if (withPTP)
      Module(new EthMac(extAddrWidth - 1, dataWidth, withPTP, secondsWidth, nanoWidth, initialTime, ppsDuration))
    else
      Module(new EthMac(extAddrWidth, dataWidth))
  }

  trait Pins extends patmos.HasPins {
    override val pins = new Bundle() {
      // Tx
      val mtx_clk_pad_i = Bool(INPUT) // Transmit clock (from PHY)
      val mtxd_pad_o = Bits(OUTPUT, width = 4) // Transmit niethle (to PHY)
      val mtxen_pad_o = Bits(OUTPUT, width = 1) // Transmit enable (to PHY)
      val mtxerr_pad_o = Bits(OUTPUT, width = 1) // Transmit error (to PHY)

      // Rx
      val mrx_clk_pad_i = Bool(INPUT) // Receive clock (from PHY)
      val mrxd_pad_i = Bits(INPUT, width = 4) // Receive niethle (from PHY)
      val mrxdv_pad_i = Bits(INPUT, width = 1) // Receive data valid (from PHY)
      val mrxerr_pad_i = Bits(INPUT, width = 1) // Receive data error (from PHY)

      // Common Tx and Rx
      val mcoll_pad_i = Bits(INPUT, width = 1) // Collision (from PHY)
      val mcrs_pad_i = Bits(INPUT, width = 1) // Carrier sense (from PHY)

      // MII Management interface
      val md_pad_i = Bits(INPUT, width = 1) // MII data input (from I/O cell)
      val mdc_pad_o = Bits(OUTPUT, width = 1) // MII Management data clock (to PHY)
      val md_pad_o = Bits(OUTPUT, width = 1) // MII data output (to I/O cell)
      val md_padoe_o = Bits(OUTPUT, width = 1) // MII data output enable (to I/O cell)

      val int_o = Bits(OUTPUT, width = 1) // Ethernet intr output

      // PTP Debug Signals
      val ptpPPS = Bits(OUTPUT, width = 1)
      val ledPHY = Bits(OUTPUT, width = 1)
      val ledSOF = Bits(OUTPUT, width = 1)
      val ledEOF = Bits(OUTPUT, width = 1)
      // val ledSFD = Bits(OUTPUT, width=8)
      // val rtcDisp = Vec.fill(8) {Bits(OUTPUT, 7)}
    }
  }

}

class EthMacBB(extAddrWidth: Int = 32, dataWidth: Int = 32) extends BlackBox(
  Map("BUFF_ADDR_WIDTH" -> extAddrWidth)) {
    val io = IO(new OcpCoreSlavePort(extAddrWidth, dataWidth) {
    val clk = Input(Clock())
    val rst = Input(Bool())
    // Tx
    val mtx_clk_pad_i = Bool(INPUT) // Transmit clock (from PHY)
    val mtxd_pad_o = Bits(OUTPUT, width = 4) // Transmit niethle (to PHY)
    val mtxen_pad_o = Bits(OUTPUT, width = 1) // Transmit enable (to PHY)
    val mtxerr_pad_o = Bits(OUTPUT, width = 1) // Transmit error (to PHY)

    // Rx
    val mrx_clk_pad_i = Bool(INPUT) // Receive clock (from PHY)
    val mrxd_pad_i = Bits(INPUT, width = 4) // Receive niethle (from PHY)
    val mrxdv_pad_i = Bits(INPUT, width = 1) // Receive data valid (from PHY)
    val mrxerr_pad_i = Bits(INPUT, width = 1) // Receive data error (from PHY)

    // Common Tx and Rx
    val mcoll_pad_i = Bits(INPUT, width = 1) // Collision (from PHY)
    val mcrs_pad_i = Bits(INPUT, width = 1) // Carrier sense (from PHY)

    // MII Management interface
    val md_pad_i = Bits(INPUT, width = 1) // MII data input (from I/O cell)
    val mdc_pad_o = Bits(OUTPUT, width = 1) // MII Management data clock (to PHY)
    val md_pad_o = Bits(OUTPUT, width = 1) // MII data output (to I/O cell)
    val md_padoe_o = Bits(OUTPUT, width = 1) // MII data output enable (to I/O cell)

    val int_o = Bits(OUTPUT, width = 1) // Ethernet intr output
  })

  // rename signals
  io.clk.suggestName("clk")
  io.rst.suggestName("rst")

  io.M.Cmd.suggestName("MCmd")
  io.M.Addr.suggestName("MAddr")
  io.M.Data.suggestName("MData")
  io.M.ByteEn.suggestName("MByteEn")
  io.S.Resp.suggestName("SResp")
  io.S.Data.suggestName("SData")

  io.mtx_clk_pad_i.suggestName("mtx_clk_pad_i")
  io.mtxd_pad_o.suggestName("mtxd_pad_o")
  io.mtxen_pad_o.suggestName("mtxen_pad_o")
  io.mtxerr_pad_o.suggestName("mtxerr_pad_o")
  io.mrx_clk_pad_i.suggestName("mrx_clk_pad_i")
  io.mrxd_pad_i.suggestName("mrxd_pad_i")
  io.mrxdv_pad_i.suggestName("mrxdv_pad_i")
  io.mrxerr_pad_i.suggestName("mrxerr_pad_i")
  io.mcoll_pad_i.suggestName("mcoll_pad_i")
  io.mcrs_pad_i.suggestName("mcrs_pad_i")
  io.md_pad_i.suggestName("md_pad_i")
  io.mdc_pad_o.suggestName("mdc_pad_o")
  io.md_pad_o.suggestName("md_pad_o")
  io.md_padoe_o.suggestName("md_padoe_o")
  io.int_o.suggestName("int_o")

  override def desiredName: String = "eth_controller_top"
}

class EthMac(extAddrWidth: Int = 32, dataWidth: Int = 32, withPTP: Boolean = false, secondsWidth: Int = 32, nanoWidth: Int = 32, initialTime: BigInt = 0L, ppsDuration: Int = 10) extends CoreDevice() {
  override val io = IO(new CoreDeviceIO() with EthMac.Pins with patmos.HasInterrupts {
    override val interrupts = Vec(1, Output(Bool()))
  })

  val eth = Module(new EthMacBB(extAddrWidth, dataWidth))
  //Wire IO pins straight through
  eth.io.clk              := clock
  eth.io.rst              := reset
  eth.io.mtx_clk_pad_i    := io.pins.mtx_clk_pad_i
  io.pins.mtxd_pad_o      := eth.io.mtxd_pad_o
  io.pins.mtxen_pad_o     := eth.io.mtxen_pad_o
  io.pins.mtxerr_pad_o    := eth.io.mtxerr_pad_o
  eth.io.mrx_clk_pad_i    := io.pins.mrx_clk_pad_i
  eth.io.mrxd_pad_i       := io.pins.mrxd_pad_i
  eth.io.mrxdv_pad_i      := io.pins.mrxdv_pad_i
  eth.io.mrxerr_pad_i     := io.pins.mrxerr_pad_i
  eth.io.mcoll_pad_i      := io.pins.mcoll_pad_i
  eth.io.mcrs_pad_i       := io.pins.mcrs_pad_i
  eth.io.md_pad_i         := io.pins.md_pad_i
  io.pins.mdc_pad_o       := eth.io.mdc_pad_o
  io.pins.md_pad_o        := eth.io.md_pad_o
  io.pins.md_padoe_o      := eth.io.md_padoe_o
  io.pins.int_o           := eth.io.int_o

  // Connection to controller interrupt
  val syncEthIntrReg = RegNext(eth.io.int_o)

  // Generate interrupts on rising edges
  val pulseEthIntrReg = RegNext(RegNext(syncEthIntrReg) === Bits("b0") && syncEthIntrReg(0) === Bits("b1"))
  io.interrupts(0) := pulseEthIntrReg

  //Check for PTP features
  if (withPTP) {
    println("EthMac w/ PTP1588 hardware (eth_addrWidth=" + extAddrWidth + ", ptp_addrWidth=" + (extAddrWidth) + ")")
    val ptp = Module(new PTP1588Assist(extAddrWidth, dataWidth, CLOCK_FREQ, secondsWidth, nanoWidth, initialTime, ppsDuration))
    val masterReg = Reg(next = io.ocp.M)
    eth.io.M.Data := masterReg.Data
    eth.io.M.ByteEn := masterReg.ByteEn
    eth.io.M.Addr := masterReg.Addr
    ptp.io.ocp.M.Data := masterReg.Data
    ptp.io.ocp.M.ByteEn := masterReg.ByteEn
    ptp.io.ocp.M.Addr := masterReg.Addr
    //Arbitrate OCP master
    when(masterReg.Addr(15, 12) === Bits("hE")) {
      eth.io.M.Cmd := OcpCmd.IDLE
      ptp.io.ocp.M.Cmd := masterReg.Cmd //PTP
    }.otherwise {
      eth.io.M.Cmd := masterReg.Cmd //EthMac
      ptp.io.ocp.M.Cmd := OcpCmd.IDLE
    }
    //Arbitrate OCP slave based on response
    val replyRegPTP = Reg(next = ptp.io.ocp.S)
    val replyRegETH = Reg(next = eth.io.S)
    when(replyRegETH.Resp =/= OcpResp.NULL) {
      io.ocp.S := replyRegETH //ETH
    }.elsewhen(replyRegPTP.Resp =/= OcpResp.NULL) {
      io.ocp.S.Resp := replyRegPTP.Resp //PTP
      io.ocp.S.Data := replyRegPTP.Data
    }.otherwise {
      io.ocp.S.Resp := OcpResp.NULL
      io.ocp.S.Data := 0.U //NONE
    }
    //Rest of IO connections
    ptp.io.ethMacRX.clk := io.pins.mrx_clk_pad_i
    ptp.io.ethMacRX.data := io.pins.mrxd_pad_i
    ptp.io.ethMacRX.dv := io.pins.mrxdv_pad_i
    ptp.io.ethMacRX.err := io.pins.mrxerr_pad_i
    ptp.io.ethMacTX.clk := eth.io.mtx_clk_pad_i
    ptp.io.ethMacTX.data := eth.io.mtxd_pad_o
    ptp.io.ethMacTX.dv := eth.io.mtxen_pad_o
    ptp.io.ethMacTX.err := eth.io.mtxerr_pad_o
    io.pins.ptpPPS := ptp.io.rtcPPS
    io.pins.ledSOF := ptp.io.ledSOF
    io.pins.ledEOF := ptp.io.ledEOF
    io.pins.ledPHY := ptp.io.ledTS
  } else {
    println("EthMac (eth_addrWidth=" + extAddrWidth + ")")
    eth.io.M <> io.ocp.M
    eth.io.S <> io.ocp.S
    io.pins.ptpPPS := true.B
  }
}


