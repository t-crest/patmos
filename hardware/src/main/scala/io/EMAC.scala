/*
 * OCP interface for LogiCORE IP Virtex-6 FPGA Embedded Tri-Mode Ethernet
 * MAC Wrapper v2.3
 *
 * Authors: Torur Strom (torur.strom@gmail.com)
 *
 */

package io
import chisel3._
import chisel3.util._

import ocp._

class EMACPins extends Bundle {
  // asynchronous reset
  val glbl_rst = Input(Bool())
  
  // Clock inputs
  val gtx_clk_bufg = Input(Bool()) // 125 MHz
  val cpu_clk = Input(Bool()) // 66.666 MHz
  val refclk_bufg = Input(Bool()) // 200 MHz
  val dcm_lck = Input(Bool())
  
  val phy_resetn = Output(Bool())
  
  // GMII Interface
  //---------------
  
  val gmii_txd = Output(UInt(8.W))
  val gmii_tx_en = Output(Bool())
  val gmii_tx_er = Output(Bool())
  val gmii_tx_clk = Output(Bool())
  
  val gmii_rxd = Input(UInt(8.W))
  val gmii_rx_dv = Input(Bool())
  val gmii_rx_er = Input(Bool())
  val gmii_rx_clk = Input(Bool())
  
  val gmii_col = Input(Bool())
  val gmii_crs = Input(Bool())
  val mii_tx_clk = Input(Bool())
}

class EMACIO extends EMACPins {

  // Receiver (AXI-S) Interface
  //----------------------------------------
  val rx_axis_fifo_tdata = Output(UInt(8.W))
  val rx_axis_fifo_tvalid = Output(Bool())
  val rx_axis_fifo_tready = Input(Bool())
  val rx_axis_fifo_tlast = Output(Bool())
  
  // Transmitter (AXI-S) Interface
  //-------------------------------------------
  val tx_axis_fifo_tdata = Input(UInt(8.W))
  val tx_axis_fifo_tvalid = Input(Bool())
  val tx_axis_fifo_tready = Output(Bool())
  val tx_axis_fifo_tlast = Input(Bool())
}

class v6_emac_v2_3_wrapper extends BlackBox {
  val io = new EMACIO()
  // Remove "io_" prefix from connection 
  var methods = classOf[EMACPins].getDeclaredMethods()
  //throw new Error("BalckBox wrapper for EMAC/Xilinx needs update for Chisel 3")
  //should be commented out when chisel3 is used
 /* methods.foreach{m => m.invoke(io).asInstanceOf[Chisel.Node].setName(m.getName())}
  methods = classOf[EMACIO].getDeclaredMethods()
  methods.foreach{m => m.invoke(io).asInstanceOf[Chisel.Node].setName(m.getName())}*/
   
}

object EMAC extends DeviceObject {

  def init(params: Map[String, String]) = {
  }

  def create(params: Map[String, String]) : EMAC = {
    Module(new EMAC())
  }
}

class EMAC() extends CoreDevice() {

  override val io = new CoreDeviceIO() with patmos.HasPins {
    override val pins = new EMACPins()
  }
  
  val bb = Module(new v6_emac_v2_3_wrapper())
  io.pins <> bb.io

  val rx_axis_fifo_tready_Reg = RegInit(init = false.B)
  rx_axis_fifo_tready_Reg := false.B
  bb.io.rx_axis_fifo_tready := rx_axis_fifo_tready_Reg

  val tx_axis_fifo_tvalid_Reg = RegInit(init = false.B)
  tx_axis_fifo_tvalid_Reg := false.B
  bb.io.tx_axis_fifo_tvalid := tx_axis_fifo_tvalid_Reg

  // Default response
  val respReg = RegInit(init = OcpResp.NULL)
  respReg := OcpResp.NULL

  // Data from EMAC
  val dataRdReg = RegInit(init = 0.U(32.W))
 

  // Data to EMAC
  val dataWrReg = RegInit(init = 0.U(32.W))
  bb.io.tx_axis_fifo_tlast := dataWrReg(30)
  bb.io.tx_axis_fifo_tdata := dataWrReg(7,0)

  val sIdle :: sWait :: sRead :: Nil = Enum(3)
  val state = RegInit(init = sIdle)
  
  when(io.ocp.M.Cmd === OcpCmd.WR) {
    respReg := OcpResp.DVA
    tx_axis_fifo_tvalid_Reg := true.B
    dataWrReg := io.ocp.M.Data
  }
  
  when(state === sIdle) {
    when(io.ocp.M.Cmd === OcpCmd.RD) {
      when(io.ocp.M.Addr(0) === false.B) {
        state := sWait
        rx_axis_fifo_tready_Reg := true.B
      }
      .otherwise {
        respReg := OcpResp.DVA
        dataRdReg := Cat(bb.io.tx_axis_fifo_tready,0.U(31.W))
      }
    }
  }
  when(state === sWait) {
    state := sRead
  }
  when(state === sRead) {
    state := sIdle
    respReg := OcpResp.DVA
    dataRdReg := Cat(bb.io.rx_axis_fifo_tvalid,Cat(bb.io.rx_axis_fifo_tlast,Cat(0.U(22.W),bb.io.rx_axis_fifo_tdata)))
  }

  // Connections to master
  io.ocp.S.Resp := respReg
  io.ocp.S.Data := dataRdReg

}
