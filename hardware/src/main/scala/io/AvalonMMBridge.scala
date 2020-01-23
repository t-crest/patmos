/*
 * A connection to an Avalon-MM device
 *
 * Authors: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
 *
 */

package io

import Chisel._
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
    override val pins = new Bundle() {
      val avs_waitrequest = Bits(INPUT,1)
      val avs_readdata = UInt(INPUT,dataWidth)
      val avs_readdatavalid = Bits(INPUT,1)
      val avs_burstcount = Bits(OUTPUT,1)
      val avs_writedata = UInt(OUTPUT,dataWidth)
      val avs_address = UInt(OUTPUT,extAddrWidth)
      val avs_write = Bool(OUTPUT)
      val avs_read = Bool(OUTPUT)
      val avs_byteenable = Bits(OUTPUT, dataWidth/8)
      val avs_debugaccess = Bool(OUTPUT)
      val avs_intr = Bits(INPUT,numIntrs)
    }
    override val interrupts = Vec.fill(numIntrs) { Bool(OUTPUT) }
  }

  val coreBus = Module(new OcpCoreBus(ADDR_WIDTH,dataWidth))
  val ioBus = Module(new OcpIOBus(ADDR_WIDTH,dataWidth))

  io.ocp <> coreBus.io.slave

  val bridge = new OcpIOBridge(coreBus.io.master,ioBus.io.slave)
  

  val intrVecReg0 = Reg(Bits(width = numIntrs))
  val intrVecReg1 = Reg(Bits(width = numIntrs))

  //for( i <- 0 until numIntrs) {
  //  intrVecReg0(i) := io.avalonMMBridgePins.avs_intr(i)
  //}
  intrVecReg0 := io.pins.avs_intr
  intrVecReg1 := intrVecReg0

  // Generate interrupts on rising edges
  for (i <- 0 until numIntrs) {
    io.interrupts(i) := (intrVecReg0(i) === Bits("b1")) && (intrVecReg1(i) === Bits("b0"))
  }

  val cmdType = Reg(init = OcpCmd.IDLE)
  
  //val respReg = Reg(init = OcpResp.NULL)
  //val dataReg = Reg(init = Bits(0, dataWidth))

  val ReadWriteActive = Bool(true)
  val ReadWriteInactive = Bool(false)
  // Default values in case of ILDE command
  //respReg := OcpResp.NULL
  //dataReg := Bits(0)
  io.pins.avs_write := ReadWriteInactive
  io.pins.avs_read := ReadWriteInactive

  ioBus.io.master.S.Resp := OcpResp.NULL
  ioBus.io.master.S.CmdAccept := Bits(0)
  ioBus.io.master.S.Data := Bits(0)

  // Constant connections
  io.pins.avs_burstcount := Bits("b1")
  //for( i <- 3 to 0) {
  //  io.avalonMMBridgePins.avs_byteenable(3-i) := ioBus.io.master.M.ByteEn(i) 
  //}
  io.pins.avs_byteenable := ioBus.io.master.M.ByteEn
  io.pins.avs_debugaccess := Bits("b0")
  // Connecting address and data signal straight through
  io.pins.avs_address := ioBus.io.master.M.Addr(extAddrWidth-1, 0)
  io.pins.avs_writedata := ioBus.io.master.M.Data
  ioBus.io.master.S.Data := io.pins.avs_readdata

  cmdType := cmdType
  
  
  when(io.pins.avs_waitrequest === Bits("b0")) {
    ioBus.io.master.S.CmdAccept := Bits("b1")
  }

  when(ioBus.io.master.M.Cmd === OcpCmd.WR) {
    io.pins.avs_write := ReadWriteActive
    io.pins.avs_read := ReadWriteInactive
    when(io.pins.avs_waitrequest === Bits("b0")) {
      ioBus.io.master.S.Resp := OcpResp.DVA
    }
  }

  when(ioBus.io.master.M.Cmd === OcpCmd.RD) {
    io.pins.avs_write := ReadWriteInactive
    io.pins.avs_read := ReadWriteActive
    cmdType := OcpCmd.RD
  }

  when(io.pins.avs_readdatavalid === Bits("b1") && cmdType === OcpCmd.RD) {
    ioBus.io.master.S.Resp := OcpResp.DVA
    cmdType := OcpCmd.IDLE
  }

}
