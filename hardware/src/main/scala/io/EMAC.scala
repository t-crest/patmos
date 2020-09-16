/*
 * OCP interface for LogiCORE IP Virtex-6 FPGA Embedded Tri-Mode Ethernet
 * MAC Wrapper v2.3
 *
 * Authors: Torur Strom (torur.strom@gmail.com)
 *
 */

package io
import Chisel._

import ocp._

class EMACPins extends Bundle {
  // asynchronous reset
  val glbl_rst = Bool(INPUT)
  
  // Clock inputs
  val gtx_clk_bufg = Bool(INPUT) // 125 MHz 
  val cpu_clk = Bool(INPUT) // 66.666 MHz 
  val refclk_bufg = Bool(INPUT) // 200 MHz
  val dcm_lck = Bool(INPUT)
  
  val phy_resetn = Bool(OUTPUT)
  
  // GMII Interface
  //---------------
  
  val gmii_txd = UInt(OUTPUT,8)
  val gmii_tx_en = Bool(OUTPUT)
  val gmii_tx_er = Bool(OUTPUT)
  val gmii_tx_clk = Bool(OUTPUT)
  
  val gmii_rxd = UInt(INPUT,8)
  val gmii_rx_dv = Bool(INPUT)
  val gmii_rx_er = Bool(INPUT)
  val gmii_rx_clk = Bool(INPUT)
  
  val gmii_col = Bool(INPUT)
  val gmii_crs = Bool(INPUT)
  val mii_tx_clk = Bool(INPUT)
}

class EMACIO extends EMACPins {

  // Receiver (AXI-S) Interface
  //----------------------------------------
  val rx_axis_fifo_tdata = UInt(OUTPUT,8)
  val rx_axis_fifo_tvalid = Bool(OUTPUT)
  val rx_axis_fifo_tready = Bool(INPUT)
  val rx_axis_fifo_tlast = Bool(OUTPUT)
  
  // Transmitter (AXI-S) Interface
  //-------------------------------------------
  val tx_axis_fifo_tdata = UInt(INPUT,8)
  val tx_axis_fifo_tvalid = Bool(INPUT)
  val tx_axis_fifo_tready = Bool(OUTPUT)
  val tx_axis_fifo_tlast = Bool(INPUT)
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

  val rx_axis_fifo_tready_Reg = Reg(init = Bool(false))
  rx_axis_fifo_tready_Reg := Bool(false)
  bb.io.rx_axis_fifo_tready := rx_axis_fifo_tready_Reg

  val tx_axis_fifo_tvalid_Reg = Reg(init = Bool(false))
  tx_axis_fifo_tvalid_Reg := Bool(false)
  bb.io.tx_axis_fifo_tvalid := tx_axis_fifo_tvalid_Reg

  // Default response
  val respReg = Reg(init = OcpResp.NULL)
  respReg := OcpResp.NULL

  // Data from EMAC
  val dataRdReg = Reg(init = Bits(0,32))
 

  // Data to EMAC
  val dataWrReg = Reg(init = Bits(0,32))
  bb.io.tx_axis_fifo_tlast := dataWrReg(30)
  bb.io.tx_axis_fifo_tdata := dataWrReg(7,0)

  val sIdle :: sWait :: sRead :: Nil = Enum(UInt(), 3)
  val state = Reg(init = sIdle)
  
  when(io.ocp.M.Cmd === OcpCmd.WR) {
    respReg := OcpResp.DVA
    tx_axis_fifo_tvalid_Reg := Bool(true)
    dataWrReg := io.ocp.M.Data
  }
  
  when(state === sIdle) {
    when(io.ocp.M.Cmd === OcpCmd.RD) {
      when(io.ocp.M.Addr(0) === Bool(false)) {
        state := sWait
        rx_axis_fifo_tready_Reg := Bool(true)
      }
      .otherwise {
        respReg := OcpResp.DVA
        dataRdReg := Cat(bb.io.tx_axis_fifo_tready,Bits(0,31))
      }
    }
  }
  when(state === sWait) {
    state := sRead
  }
  when(state === sRead) {
    state := sIdle
    respReg := OcpResp.DVA
    dataRdReg := Cat(bb.io.rx_axis_fifo_tvalid,Cat(bb.io.rx_axis_fifo_tlast,Cat(Bits(0,22),bb.io.rx_axis_fifo_tdata)))
  }

  // Connections to master
  io.ocp.S.Resp := respReg
  io.ocp.S.Data := dataRdReg

}
