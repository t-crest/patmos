package io

import chisel3._
import chisel3.util._
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
  override val io = IO(new CoreDeviceIO() with patmos.HasPins {
    val pins: Bundle {
      val araddr: UInt
      val arready: Bool
      val arvalid: Bool
      val awaddr: UInt
      val awready: Bool
      val awvalid: Bool
      val bready: Bool
      val bresp: UInt
      val bvalid: Bool
      val rdata: UInt
      val rready: Bool
      val rresp: UInt
      val rvalid: Bool
      val wdata: UInt
      val wready: Bool
      val wstrb: UInt
      val wvalid: Bool
    } = new Bundle() {
      val araddr = Output(UInt(addrWidth.W))
      val arready = Input(Bool())
      val arvalid = Output(Bool())
      val awaddr = Output(UInt(addrWidth.W))
      val awready = Input(Bool())
      val awvalid = Output(Bool())
      val bready = Output(Bool())
      val bresp = Input(UInt(2.W))
      val bvalid = Input(Bool())
      val rdata = Input(UInt(dataWidth.W))
      val rready = Output(Bool())
      val rresp = Input(UInt(2.W))
      val rvalid = Input(Bool())
      val wdata = Output(UInt(dataWidth.W))
      val wready = Input(Bool())
      val wstrb = Output(UInt((dataWidth / 8).W))
      val wvalid = Output(Bool())
      
    }
  })

  val mAxiPort = Wire(new AxiLiteMasterPort(addrWidth, dataWidth))
  mAxiPort.ar.bits.prot := 0.U
  mAxiPort.aw.bits.prot := 0.U

  val mOcpReg = RegInit(io.ocp.M)
  val holdBusyReg = RegInit(false.B)

  // For simplicity we register new commands only when the AXI slave has answered fully
  when(~holdBusyReg) {
    mOcpReg := io.ocp.M
  }
  
  when(~holdBusyReg) {
    when(io.ocp.M.Cmd === OcpCmd.RD) {
      holdBusyReg := ~mAxiPort.ar.ready
    }.elsewhen(io.ocp.M.Cmd === OcpCmd.WR) {
      holdBusyReg := ~mAxiPort.aw.ready && ~mAxiPort.w.ready
    }
  }.otherwise {
    when(mOcpReg.Cmd === OcpCmd.RD) {
      holdBusyReg := ~mAxiPort.ar.ready
    }.elsewhen(mOcpReg.Cmd === OcpCmd.WR) {
      holdBusyReg := ~mAxiPort.aw.ready && ~mAxiPort.w.ready
    }
  }

  // Write channel
  mAxiPort.aw.valid := mOcpReg.Cmd === OcpCmd.WR && holdBusyReg
  mAxiPort.aw.bits.addr := mOcpReg.Addr
  mAxiPort.w.valid := mOcpReg.Cmd === OcpCmd.WR && holdBusyReg
  mAxiPort.w.bits.data := mOcpReg.Data
  mAxiPort.w.bits.strb := mOcpReg.ByteEn

  // Read channel
  mAxiPort.ar.bits.addr := mOcpReg.Addr
  mAxiPort.ar.valid := mOcpReg.Cmd === OcpCmd.RD && holdBusyReg
  mAxiPort.r.ready := true.B // the ocp bus is always ready to accept data
  mAxiPort.b.ready := true.B

  // Drive OCP slave
  io.ocp.S.Data := mAxiPort.r.bits.data
 
  io.ocp.S.Resp := Mux(mAxiPort.b.valid || mAxiPort.r.valid, OcpResp.DVA, OcpResp.NULL)

//  // Xilinx naming convention for nice block diagrams and inferring interfaces
//  // TODO: investigate erroneous generated Verilog

   io.pins.araddr.suggestName("m_axi_araddr")
   io.pins.arready.suggestName("m_axi_arready")
   io.pins.arvalid.suggestName("m_axi_arvalid")
   io.pins.awaddr.suggestName("m_axi_awaddr")
   io.pins.awready.suggestName("m_axi_awready")
   io.pins.awvalid.suggestName("m_axi_awvalid")
   io.pins.bready.suggestName("m_axi_bready")
   io.pins.bresp.suggestName("m_axi_bresp")
   io.pins.bvalid.suggestName("m_axi_bvalid")
   io.pins.rready.suggestName("m_axi_rready")
   io.pins.rdata.suggestName("m_axi_rdata")
   io.pins.rvalid.suggestName("m_axi_rvalid")
   io.pins.rresp.suggestName("m_axi_rresp")
   io.pins.wdata.suggestName("m_axi_wdata")
   io.pins.wready.suggestName("m_axi_wready")
   io.pins.wstrb.suggestName("m_axi_wstrb")
   io.pins.wvalid.suggestName("m_axi_wvalid")
//
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