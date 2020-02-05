package io

import Chisel._
import axi._
import ocp._

object AXI4LiteMMBridge extends DeviceObject {
  var extAddrWidth = 32
  var dataWidth = 32

  def init(params: Map[String, String]) = {
    extAddrWidth = getPosIntParam(params, "extAddrWidth")
    dataWidth = getPosIntParam(params, "dataWidth")
  }

  def create(params: Map[String, String]): AXI4LiteMMBridge = {
    Module(new AXI4LiteMMBridge(extAddrWidth, dataWidth))
  }

}

class AXI4LiteMMBridge(addrWidth: Int = 32, dataWidth: Int = 32) extends CoreDevice() {
  override val io = new CoreDeviceIO() with patmos.HasPins {
    override val pins = new Bundle() {
      val araddr = Bits(OUTPUT, addrWidth)
      val arready = Bool(INPUT)
      val arvalid = Bool(OUTPUT)
      val awaddr = Bits(OUTPUT, addrWidth)
      val awready = Bool(INPUT)
      val awvalid = Bool(OUTPUT)
      val bready = Bool(OUTPUT)
      val bresp = Bits(INPUT, 2)
      val bvalid = Bool(INPUT)
      val rdata = Bits(INPUT, dataWidth)
      val rready = Bool(OUTPUT)
      val rresp = Bits(INPUT, 2)
      val rvalid = Bool(INPUT)
      val wdata = Bits(OUTPUT, dataWidth)
      val wready = Bool(INPUT)
      val wstrb = Bits(OUTPUT, dataWidth / 8)
      val wvalid = Bool(OUTPUT)
    }
  }

  val mAxiPort = new AxiLiteMasterPort(addrWidth, dataWidth)

  val mOcpReg = Reg(init = io.ocp.M)

  // For simplicity we register new commands only when the AXI slave has answered fully
  when(mOcpReg.Cmd === OcpCmd.IDLE || mAxiPort.b.valid || mAxiPort.r.valid) {
    mOcpReg := io.ocp.M
  }

  // Write channel
  mAxiPort.aw.valid := mOcpReg.Cmd === OcpCmd.WR
  mAxiPort.aw.bits.addr := mOcpReg.Addr
  mAxiPort.w.valid := mOcpReg.Cmd === OcpCmd.WR
  mAxiPort.w.bits.data := mOcpReg.Data
  mAxiPort.w.bits.strb := mOcpReg.ByteEn

  // Read channel
  mAxiPort.ar.bits.addr := mOcpReg.Addr
  mAxiPort.ar.valid := mOcpReg.Cmd === OcpCmd.RD
  mAxiPort.r.ready := true.B // the ocp bus is always ready to accept data
  mAxiPort.b.ready := true.B

  // Drive OCP slave
  io.ocp.S.Data := mAxiPort.r.bits.data
  io.ocp.S.Resp := OcpResp.NULL
  io.ocp.S.Resp := Mux(mAxiPort.b.valid || mAxiPort.r.valid, OcpResp.DVA, OcpResp.NULL)

  // Xilinx naming convention for nice block diagrams and inferring interfaces
  // TODO: investigate erroneous generated Verilog
  
  // io.aXI4LiteMMBridgePins.araddr.setName("m_axi_araddr")
  // io.aXI4LiteMMBridgePins.arready.setName("m_axi_arready")
  // io.aXI4LiteMMBridgePins.arvalid.setName("m_axi_arvalid")
  // io.aXI4LiteMMBridgePins.awaddr.setName("m_axi_awaddr")
  // io.aXI4LiteMMBridgePins.awready.setName("m_axi_awready")
  // io.aXI4LiteMMBridgePins.awvalid.setName("m_axi_awvalid")
  // io.aXI4LiteMMBridgePins.bready.setName("m_axi_bready")
  // io.aXI4LiteMMBridgePins.bresp.setName("m_axi_bresp")
  // io.aXI4LiteMMBridgePins.bvalid.setName("m_axi_bvalid")
  // io.aXI4LiteMMBridgePins.rready.setName("m_axi_rready")
  // io.aXI4LiteMMBridgePins.rdata.setName("m_axi_rdata")
  // io.aXI4LiteMMBridgePins.rvalid.setName("m_axi_rvalid")
  // io.aXI4LiteMMBridgePins.rresp.setName("m_axi_rresp")
  // io.aXI4LiteMMBridgePins.wdata.setName("m_axi_wdata")
  // io.aXI4LiteMMBridgePins.wready.setName("m_axi_wready")
  // io.aXI4LiteMMBridgePins.wstrb.setName("m_axi_wstrb")
  // io.aXI4LiteMMBridgePins.wvalid.setName("m_axi_wvalid")

  // IO plumbing
  io.pins.araddr := mAxiPort.ar.bits.addr
  mAxiPort.ar.ready := io.pins.arready
  io.pins.arvalid := mAxiPort.ar.valid
  io.pins.awaddr := mAxiPort.aw.bits.addr
  mAxiPort.aw.ready := io.pins.awready
  io.pins.awvalid := mAxiPort.aw.valid
  io.pins.bready := mAxiPort.b.ready
  mAxiPort.b.bits.resp := io.pins.bresp
  mAxiPort.b.valid := io.pins.bvalid
  mAxiPort.r.bits.data := io.pins.rdata
  io.pins.rready := mAxiPort.r.ready
  mAxiPort.r.bits.resp := io.pins.rresp
  mAxiPort.r.valid := io.pins.rvalid
  io.pins.wdata := mAxiPort.w.bits.data
  mAxiPort.w.ready := io.pins.wready
  io.pins.wstrb := mAxiPort.w.bits.strb
  io.pins.wvalid := mAxiPort.w.valid

}
