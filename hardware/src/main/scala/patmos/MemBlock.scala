
/*
 * A generic dual-ported (one read-, one write-port) memory block
 *
 * Author: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patmos

import Chisel._
import Node._

object MemBlock {
  def apply(size : Int, width : Int, bypass : Boolean = true) = {
    Module(new MemBlock(size, width, bypass))
    // Module(new BlackBlock(size, width))
  }
}

class MemBlockIO(size : Int, width : Int) extends Bundle {
  val rdAddr = Bits(INPUT, log2Up(size))
  val rdData = Bits(OUTPUT, width)
  val wrAddr = Bits(INPUT, log2Up(size))
  val wrEna  = Bits(INPUT, 1)
  val wrData = Bits(INPUT, width)

  var read = false
  var write = false

  def <= (ena : Bits, addr : Bits, data : Bits) = {
    // This memory supports only one write port
    if (write) { ChiselError.error("Only one write port supported") }
    write = true

    wrAddr := addr
    wrEna := ena
    wrData := data
  }

  def apply(addr : Bits) : Bits = {
    // This memory supports only one read port
    if (read) { ChiselError.error("Only one read port supported") }
    read = true

    rdAddr := addr
    rdData
  }
}

class MemBlock(size : Int, width : Int, bypass : Boolean = true) extends Module {
  val io = new MemBlockIO(size, width)
  val mem = Mem(Bits(width = width), size)

  // write
  when(io.wrEna === Bits(1)) {
    mem(io.wrAddr) := io.wrData
  }

  // read
  val rdAddrReg = Reg(next = io.rdAddr)
  io.rdData := mem(rdAddrReg)

  if (bypass) {
    // force read during write behavior
    when (Reg(next = io.wrEna) === Bits(1) &&
          Reg(next = io.wrAddr) === rdAddrReg) {
            io.rdData := Reg(next = io.wrData)
          }
  }
}

class BlackBlock(size : Int, width : Int) extends BlackBox {
  val io = new MemBlockIO(size, width)

  // Set entity name
  setName("Ram"+size+"x"+width)
  // Override port names as necessary
  io.rdAddr.setName("RdA")
  io.rdData.setName("Q")
  io.wrAddr.setName("WrA")
  io.wrEna.setName("WrEn")
  io.wrData.setName("D")
}

