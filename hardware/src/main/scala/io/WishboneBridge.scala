/*
 * OCP (Master) -> Wishbone (Master) bridge for Patmos
 *
 * Author: Luca Pezzarossa (lpez@dtu.dk)
 *
 */

package io

import chisel3._
import ocp._

object WishboneBridge extends DeviceObject {
  var extAddrWidth = 32
  var dataWidth = 32

  def init(params : Map[String, String]) = {
    extAddrWidth = getPosIntParam(params, "extAddrWidth")
    dataWidth = getPosIntParam(params, "dataWidth")
  }

  def create(params: Map[String, String]) : WishboneBridge = {
    Module(new WishboneBridge(extAddrWidth=extAddrWidth, dataWidth=dataWidth))
  }
}

class WishboneBridge(extAddrWidth : Int = 32,
                     dataWidth : Int = 32) extends CoreDevice() {
  override val io = new CoreDeviceIO() with patmos.HasPins {
    val pins: Bundle {
      val wb_addr_o: UInt
      val wb_data_i: UInt
      val wb_err_i: Bool
      val wb_data_o: UInt
      val wb_we_o: Bool
      val wb_sel_o: UInt
      val wb_stb_o: Bool
      val wb_ack_i: Bool
      val wb_cyc_o: Bool
    } = new Bundle() {
      val wb_addr_o = Output(UInt(extAddrWidth.W))
      val wb_data_i = Input(UInt(dataWidth.W))
      val wb_err_i = Input(Bool())
      val wb_data_o = Output(UInt(dataWidth.W))
      val wb_we_o = Output(Bool())
      val wb_sel_o = Output(UInt(4.W))
      val wb_stb_o = Output(Bool())
      val wb_ack_i = Input(Bool())
      val wb_cyc_o = Output(Bool())
    }
  }
  // Registers
  val we_o_Reg = RegInit(init = 0.U(1.W))
  val stb_o_Reg = RegInit(init = 0.U(1.W))
  val cyc_o_Reg = RegInit(init = 0.U(1.W))
  val addr_o_Reg = RegInit(init = 0.U(extAddrWidth.W))
  val data_o_Reg = RegInit(init = 0.U(dataWidth.W))
  
  val ocp_S_data_Reg = RegInit(init = 0.U(dataWidth.W))
  val ocp_S_resp_Reg = RegInit(init = OcpResp.NULL)

  // Default values and assigments of outputs
  ocp_S_resp_Reg := OcpResp.NULL
  io.ocp.S.Resp := ocp_S_resp_Reg
  io.ocp.S.Data := ocp_S_data_Reg
  io.pins.wb_sel_o := "b1111".U
  io.pins.wb_we_o := we_o_Reg 
  io.pins.wb_stb_o := stb_o_Reg
  io.pins.wb_cyc_o := cyc_o_Reg
  io.pins.wb_addr_o := addr_o_Reg
  io.pins.wb_data_o := data_o_Reg

  // Read command
  when(io.ocp.M.Cmd === OcpCmd.RD) {
  we_o_Reg := false.B
  stb_o_Reg := true.B
  cyc_o_Reg := true.B
  addr_o_Reg := io.ocp.M.Addr(extAddrWidth-1, 0)
  }

  // Write command
  when(io.ocp.M.Cmd === OcpCmd.WR) {
  we_o_Reg := true.B
  stb_o_Reg := true.B
  cyc_o_Reg := true.B
  addr_o_Reg := io.ocp.M.Addr(extAddrWidth-1, 0)
  data_o_Reg := io.ocp.M.Data
  }

  // Transaltion of the WB's ack into OCP's DVA
  when(io.pins.wb_ack_i === 1.U) {
  we_o_Reg := false.B
  stb_o_Reg := false.B
  cyc_o_Reg := false.B
  ocp_S_resp_Reg := OcpResp.DVA
  ocp_S_data_Reg := io.pins.wb_data_i
  }
}
