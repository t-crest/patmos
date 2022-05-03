
/*
 * A generic dual-ported (one read-, one write-port) memory block
 *
 * Author: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patmos

import Chisel._
import chisel3.SyncReadMem
import util.SRAM

object MemBlock {
  def apply(size : Int, width : Int) = {
    Module(new MemBlock(size, width))
  }
}

class MemBlockIO(size : Int, width : Int) extends Bundle {
  val rdAddr = UInt(INPUT, log2Up(size))
  val rdData = UInt(OUTPUT, width)
  val wrAddr = UInt(INPUT, log2Up(size))
  val wrEna  = UInt(INPUT, 1)
  val wrData = UInt(INPUT, width)

  var read = false
  var write = false

  def <= (ena : UInt, addr : UInt, data : UInt) = {
    // This memory supports only one write port
    if (write) { throw new Error("Only one write port supported") }
    write = true

    wrAddr := addr
    wrEna := ena
    wrData := data
  }

  def apply(addr : UInt) : UInt = {
    // This memory supports only one read port
    if (read) { throw new Error("Only one read port supported") }
    read = true

    rdAddr := addr
    rdData
  }
}

class MemBlock(size : Int, width : Int) extends Module {
  val io = new MemBlockIO(size, width)

  // switch between chisel SyncReadMem and SRAM using a verilog model
  val useSimSRAM = false

  val memData = Wire(UInt(width.W))

  if(useSimSRAM) {

    val mem = Module(new SRAM(size, width))

    mem.io.rdAddr := io.rdAddr
    mem.io.wrAddr := io.wrAddr
    mem.io.wrData := io.wrData
    mem.io.wrEna := io.wrEna.asBool()
    memData := mem.io.rdData

  } else {

    val mem = SyncReadMem(size, UInt(width = width))
    // write
    when(io.wrEna === UInt(1)) {
      mem.write(io.wrAddr, io.wrData)
    }
    // read
    memData := mem.read(io.rdAddr)

  }

  // bypass for read during write
  val wrDataReg = RegNext(io.wrData)
  val doForwardingReg = RegNext(io.wrAddr === io.rdAddr && io.wrEna.asBool)
  io.rdData := Mux(doForwardingReg, wrDataReg, memData)

}

