/*
 * Onc-chip memory controller with OCP burst interface.
 *
 * Author: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package io

import Chisel._

import patmos.Constants._
import ocp._

import patmos.MemBlock
import patmos.MemBlockIO

object OCRamCtrl extends DeviceObject {
  var addrWidth = 16

  def init(params: Map[String, String]) = {
    addrWidth = getPosIntParam(params, "sramAddrWidth")
  }

  def create(params: Map[String, String]) : OCRamCtrl = {
    Module(new OCRamCtrl(addrWidth,BURST_LENGTH))
  }
}

class OCRamCtrl(addrWidth : Int, ocpBurstLen : Int=4) extends BurstDevice(addrWidth) {

  override val io = IO(new BurstDeviceIO(addrWidth))

  val BYTE_WIDTH = 8
  val BYTES_PER_WORD = io.ocp.M.Data.getWidth / BYTE_WIDTH

  val size = 1 << addrWidth

  val idle :: read :: write :: Nil = Enum(UInt(), 3)
  val stateReg = Reg(init = idle)

  val ramAddrWidth = addrWidth - log2Up(BYTES_PER_WORD)
  val addrReg = Reg(init = UInt(0, width = ramAddrWidth - log2Up(ocpBurstLen)))

  val burstCntReg = Reg(init = UInt(0, width = log2Up(ocpBurstLen)))
  val burstCntNext = burstCntReg + UInt(1);

  val addr = Wire(UInt())
  addr := addrReg ## burstCntNext
  val wrEn = Wire(Bool())
  wrEn := stateReg === write

  burstCntReg := burstCntNext
  // end transaction after a burst
  when (burstCntReg === UInt(ocpBurstLen-1)) {
    stateReg := idle
    wrEn := Bool(false)
  }

  // start a new transaction
  when (io.ocp.M.Cmd === OcpCmd.RD || io.ocp.M.Cmd === OcpCmd.WR) {
    val ocpAddr = io.ocp.M.Addr(addrWidth-1,
                                log2Up(ocpBurstLen) + log2Up(BYTES_PER_WORD))
    addrReg := ocpAddr
    burstCntReg := UInt(0)

    addr := ocpAddr ## UInt(0, width = log2Up(ocpBurstLen))
  }
  when (io.ocp.M.Cmd === OcpCmd.RD) {
    stateReg := read
  }
  when (io.ocp.M.Cmd === OcpCmd.WR) {
    stateReg := write
    wrEn := Bool(true)
  }

  // generate byte memories
  val mem = new Array[MemBlockIO](BYTES_PER_WORD)
  for (i <- 0 until BYTES_PER_WORD) {
    mem(i) = MemBlock(size / BYTES_PER_WORD, BYTE_WIDTH, bypass = false).io
  }

  // store
  val stmsk = Mux(wrEn, io.ocp.M.DataByteEn, Bits(0))
  for (i <- 0 until BYTES_PER_WORD) {
    mem(i) <= (stmsk(i), addr, io.ocp.M.Data(BYTE_WIDTH*(i+1)-1, BYTE_WIDTH*i))
  }

  // load
  io.ocp.S.Data := mem.map(_(addr)).reduceLeft((x,y) => y ## x)

  // respond
  io.ocp.S.Resp := OcpResp.NULL
  when (stateReg === read ||
        (stateReg === write && burstCntReg === UInt(ocpBurstLen-1))) {
    io.ocp.S.Resp := OcpResp.DVA
  }

  // always accept
  io.ocp.S.CmdAccept := Bits(1)
  io.ocp.S.DataAccept := Bits(1)
}

/*
 Used to instantiate a single OCRAM control component
 */
object OCRamMain {
  def main(args: Array[String]): Unit = {
    val chiselArgs = args.slice(1,args.length)
    val addrWidth = args(0).toInt
    chiselMain(chiselArgs, () => Module(new OCRamCtrl(addrWidth)))
  }
}
