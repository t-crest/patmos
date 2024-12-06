/*
 * A connection to an Avalon-MM device
 *
 * Authors: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
 *
 */

package io

import chisel3._
import ocp._
import patmos.Constants._

object AvalonMMBridge extends DeviceObject {
  var extAddrWidth = 32
  var dataWidth = 32
  var numIntrs = 0

  def init(params : Map[String, String]) = {
    extAddrWidth = getPosIntParam(params, "extAddrWidth")
    dataWidth = getPosIntParam(params, "dataWidth")
    numIntrs = getIntParam(params, "numIntrs")
  }

  def create(params: Map[String, String]) : AvalonMMBridge = {
    Module(new AvalonMMBridge(extAddrWidth=extAddrWidth, dataWidth=dataWidth, numIntrs=numIntrs))
  }
}

class AvalonMMBridge(extAddrWidth : Int = 32,
                     dataWidth : Int = 32,
                     numIntrs : Int = 1) extends CoreDevice() {
  override val io = new CoreDeviceIO() with patmos.HasPins with patmos.HasInterrupts {
    val pins: Bundle {
      val avs_waitrequest: UInt
      val avs_readdata: UInt
      val avs_readdatavalid: UInt
      val avs_burstcount: UInt
      val avs_writedata: UInt
      val avs_address: UInt
      val avs_write: Bool
      val avs_read: Bool
      val avs_byteenable: UInt
      val avs_debugaccess: Bool
      val avs_intr: UInt
    } = new Bundle() {
      val avs_waitrequest = Input(UInt(1.W))
      val avs_readdata = Input(UInt(dataWidth.W))
      val avs_readdatavalid = Input(UInt(1.W))
      val avs_burstcount = Output(UInt(1.W))
      val avs_writedata = Output(UInt(dataWidth.W))
      val avs_address = Output(UInt(extAddrWidth.W))
      val avs_write = Output(Bool())
      val avs_read = Output(Bool())
      val avs_byteenable = Output(UInt((dataWidth/8).W))
      val avs_debugaccess = Output(Bool())
      val avs_intr = Input(UInt(numIntrs.W))
    }
    override val interrupts = Output(VecInit(Seq.fill(numIntrs)(Bool()))) // why is there a VecInit? a Vec would just be fine (MS)
  }

  val coreBus = Module(new OcpCoreBus(ADDR_WIDTH,dataWidth))
  val ioBus = Module(new OcpIOBus(ADDR_WIDTH,dataWidth))

  io.ocp <> coreBus.io.slave

  val bridge = new OcpIOBridge(coreBus.io.master,ioBus.io.slave)
  

  val intrVecReg0 = Reg(UInt(numIntrs.W))
  val intrVecReg1 = Reg(UInt(numIntrs.W))

  //for( i <- 0 until numIntrs) {
  //  intrVecReg0(i) := io.avalonMMBridgePins.avs_intr(i)
  //}
  intrVecReg0 := io.pins.avs_intr
  intrVecReg1 := intrVecReg0

  // Generate interrupts on rising edges
  for (i <- 0 until numIntrs) {
    io.interrupts(i) := (intrVecReg0(i) === "b1".U) && (intrVecReg1(i) === "b0".U)
  }

  val cmdType = RegInit(init = OcpCmd.IDLE)
  
  //val respReg = Reg(init = OcpResp.NULL)
  //val dataReg = Reg(init = Bits(0, dataWidth))

  val ReadWriteActive = true.B
  val ReadWriteInactive = false.B
  // Default values in case of ILDE command
  //respReg := OcpResp.NULL
  //dataReg := 0.U
  io.pins.avs_write := ReadWriteInactive
  io.pins.avs_read := ReadWriteInactive

  ioBus.io.master.S.Resp := OcpResp.NULL
  ioBus.io.master.S.CmdAccept := 0.U
  ioBus.io.master.S.Data := 0.U

  // Constant connections
  io.pins.avs_burstcount := "b1".U
  //for( i <- 3 to 0) {
  //  io.avalonMMBridgePins.avs_byteenable(3-i) := ioBus.io.master.M.ByteEn(i) 
  //}
  io.pins.avs_byteenable := ioBus.io.master.M.ByteEn
  io.pins.avs_debugaccess := "b0".U
  // Connecting address and data signal straight through
  io.pins.avs_address := ioBus.io.master.M.Addr(extAddrWidth-1, 0)
  io.pins.avs_writedata := ioBus.io.master.M.Data
  ioBus.io.master.S.Data := io.pins.avs_readdata

  cmdType := cmdType
  
  
  when(io.pins.avs_waitrequest === "b0".U) {
    ioBus.io.master.S.CmdAccept := "b1".U
  }

  when(ioBus.io.master.M.Cmd === OcpCmd.WR) {
    io.pins.avs_write := ReadWriteActive
    io.pins.avs_read := ReadWriteInactive
    when(io.pins.avs_waitrequest === "b0".U) {
      ioBus.io.master.S.Resp := OcpResp.DVA
    }
  }

  when(ioBus.io.master.M.Cmd === OcpCmd.RD) {
    io.pins.avs_write := ReadWriteInactive
    io.pins.avs_read := ReadWriteActive
    cmdType := OcpCmd.RD
  }

  when(io.pins.avs_readdatavalid === "b1".U && cmdType === OcpCmd.RD) {
    ioBus.io.master.S.Resp := OcpResp.DVA
    cmdType := OcpCmd.IDLE
  }

}
