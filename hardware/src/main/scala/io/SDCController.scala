package io

import chisel3._
import ocp._

object SDCController extends DeviceObject{
  var extAddrWidth = 16
  var dataWidth = 32

  def init(params: Map[String, String]) = {
    extAddrWidth = getPosIntParam(params, "extAddrWidth")
    dataWidth = getPosIntParam(params, "dataWidth")
  }
  def create(params: Map[String, String]): SDCController = Module(new SDCController(extAddrWidth, dataWidth))

  trait Pins extends patmos.HasPins{
    override val pins = new Bundle() {
      // sdcard port
      val sd_dat_dat = Input(UInt(4.W))
      val sd_dat_out = Output(UInt(4.W))
      val sd_dat_oe = Output(Bool())
      val sd_cmd_dat = Input(Bool())
      val sd_cmd_out = Output(Bool())
      val sd_cmd_oe = Output(Bool())
      val sd_clk_o_pad = Output(Bool())
      val int_a = Output(Bool())
      val int_b = Output(Bool())
      val int_c = Output(Bool())
    }
  }
}

class SDCControllerBB(extAddrWidth: Int = 16, dataWidth: Int = 32) extends BlackBox(
  Map("BUFF_ADDR_WIDTH" -> extAddrWidth)) {
  val io = IO(new OcpCoreSlavePort(extAddrWidth, dataWidth) {
    val clk = Input(Clock())
    val rst = Input(Bool())
    // sdcard port
    val sd_dat_dat = Input(UInt(4.W))
    val sd_dat_out = Output(UInt(4.W))
    val sd_dat_oe = Output(Bool())
    val sd_cmd_dat = Input(Bool())
    val sd_cmd_out = Output(Bool())
    val sd_cmd_oe = Output(Bool())
    val sd_clk_o_pad = Output(Bool())
    // interrupts
    val int_a = Output(Bool())
    val int_b = Output(Bool())
    val int_c = Output(Bool())
  })

  // rename signals
  io.clk.suggestName("clk")
  io.rst.suggestName("rst")

  io.M.Cmd.suggestName("M_Cmd")
  io.M.Addr.suggestName("M_Addr")
  io.M.Data.suggestName("M_Data")
  io.M.ByteEn.suggestName("M_ByteEn")
  io.S.Resp.suggestName("S_Resp")
  io.S.Data.suggestName("S_Data")

  io.sd_dat_dat.suggestName("sd_dat_dat")
  io.sd_dat_out.suggestName("sd_dat_out")
  io.sd_dat_oe.suggestName("sd_dat_oe")
  io.sd_cmd_dat.suggestName("sd_cmd_dat")
  io.sd_cmd_out.suggestName("sd_cmd_out")
  io.sd_cmd_oe.suggestName("sd_cmd_oe")
  io.sd_clk_o_pad.suggestName("sd_clk_o_pad")
  io.int_a.suggestName("int_a")
  io.int_b.suggestName("int_b")
  io.int_c.suggestName("int_c")

  override def desiredName: String = "sdc_controller_top"
}

class SDCController(extAddrWidth: Int = 16, dataWidth: Int = 32) extends CoreDevice() {
  override val io = IO(new CoreDeviceIO() with SDCController.Pins with patmos.HasInterrupts {
    override val interrupts = Vec(3, Output(Bool()))
  })

  val sdc = Module(new SDCControllerBB(extAddrWidth, dataWidth))
  // wire all pins
  sdc.io.clk <> clock
  sdc.io.rst <> reset
  sdc.io.M <> io.ocp.M
  sdc.io.S <> io.ocp.S
  sdc.io.sd_dat_dat <> io.pins.sd_dat_dat
  sdc.io.sd_dat_out <> io.pins.sd_dat_out
  sdc.io.sd_dat_oe <> io.pins.sd_dat_oe
  sdc.io.sd_cmd_dat <> io.pins.sd_cmd_dat
  sdc.io.sd_cmd_out <> io.pins.sd_cmd_out
  sdc.io.sd_cmd_oe <> io.pins.sd_cmd_oe
  sdc.io.sd_clk_o_pad <> io.pins.sd_clk_o_pad
  sdc.io.int_a <> io.pins.int_a
  sdc.io.int_b <> io.pins.int_b
  sdc.io.int_c <> io.pins.int_c
  // sync interrupts
  val syncIntA = RegNext(sdc.io.int_a)
  val syncIntB = RegNext(sdc.io.int_b)
  val syncIntC = RegNext(sdc.io.int_c)
  // connect to interrupt controller
  io.interrupts(0) := RegNext(RegNext(syncIntA) === 0.U && syncIntA === 0x1.U)
  io.interrupts(1) := RegNext(RegNext(syncIntB) === 0.U && syncIntB === 0x1.U)
  io.interrupts(2) := RegNext(RegNext(syncIntC) === 0.U && syncIntC === 0x1.U)
}
