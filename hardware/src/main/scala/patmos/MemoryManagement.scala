/*
 * A memory management unit for Patmos.
 *
 * Author: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patmos

import Chisel._
import Node._

import Constants._

import ocp._

object MemoryManagement {
  val SEG_BITS = 3
  val SEG_COUNT = 1 << SEG_BITS
  val PERM_BITS = 3
  val ALIGN_BITS = log2Up(BURST_LENGTH)+2

  val PERM_R = 2
  val PERM_W = 1
  val PERM_X = 0

  class Segment extends Bundle {
    val perm   = Bits(width = PERM_BITS)
    val length = UInt(width = ADDR_WIDTH-ALIGN_BITS-PERM_BITS)
    val base   = UInt(width = ADDR_WIDTH-ALIGN_BITS)
  }
}

import MemoryManagement._

class MemoryManagement extends Module {
  val io = IO(new MMUIO())

  val masterReg = RegNext(io.ctrl.M)

  def checked(action: => Unit) {
    when (io.superMode) (action) .otherwise { io.ctrl.S.Resp := OcpResp.ERR }
  }

  val segInfoVec = Mem(Bits(width = ADDR_WIDTH-ALIGN_BITS), SEG_COUNT)
  val segBaseVec = Mem(Bits(width = ADDR_WIDTH-ALIGN_BITS), SEG_COUNT)

  // Default OCP response
  io.ctrl.S.Resp := OcpResp.NULL
  io.ctrl.S.Data := Bits(0, width = DATA_WIDTH)

  // Handle OCP reads and writes for control interface
  when(masterReg.Cmd === OcpCmd.RD) {
    io.ctrl.S.Resp := OcpResp.DVA
  }
  when(masterReg.Cmd === OcpCmd.WR) {
    io.ctrl.S.Resp := OcpResp.DVA
    checked {
      when (masterReg.Addr(2) === Bits(0)) {
        segBaseVec(masterReg.Addr(SEG_BITS+2, 3)) := masterReg.Data(DATA_WIDTH-1, ALIGN_BITS)
      } .otherwise {
        segInfoVec(masterReg.Addr(SEG_BITS+2, 3)) := masterReg.Data(DATA_WIDTH-1, ALIGN_BITS)
      }
    }
  }

  // Address translation
  val virtReg = RegNext(io.virt.M)
  val execReg = RegNext(io.exec)

  val segment = new Segment() // Wire of a Bundle?
  val info = segInfoVec(virtReg.Addr(ADDR_WIDTH-1, ADDR_WIDTH-SEG_BITS))
  val base = segBaseVec(virtReg.Addr(ADDR_WIDTH-1, ADDR_WIDTH-SEG_BITS))
  segment.perm   := info(ADDR_WIDTH-ALIGN_BITS-1, ADDR_WIDTH-ALIGN_BITS-PERM_BITS)
  segment.length := info(ADDR_WIDTH-ALIGN_BITS-PERM_BITS-1, 0)
  segment.base   := base

  val translated = new OcpBurstMasterSignals(ADDR_WIDTH, DATA_WIDTH)
  translated := virtReg
  translated.Addr := Cat(segment.base, Bits(0, width = ALIGN_BITS)) + virtReg.Addr(ADDR_WIDTH-SEG_BITS-1, 0)

  // Connect return path
  io.virt.S := io.phys.S

  // Buffer signals towards external memory
  val buffer = Module(new Queue(new OcpBurstMasterSignals(EXTMEM_ADDR_WIDTH, DATA_WIDTH), BURST_LENGTH))

  buffer.io.enq.bits := translated
  buffer.io.enq.valid := !reset
  io.virt.S.CmdAccept := buffer.io.enq.ready
  io.virt.S.DataAccept := buffer.io.enq.ready

  io.phys.M := buffer.io.deq.bits
  buffer.io.deq.ready := io.phys.S.CmdAccept | io.phys.S.DataAccept

  // Check permissions
  val permViol = ((virtReg.Cmd === OcpCmd.RD && !execReg && segment.perm(PERM_R) === Bits(0)) ||
                  (virtReg.Cmd === OcpCmd.RD && execReg && segment.perm(PERM_X) === Bits(0)) ||
                  (virtReg.Cmd === OcpCmd.WR && segment.perm(PERM_W) === Bits(0)))
  val permViolReg = Reg(next = permViol)
  debug(permViolReg)

  val lengthViol = (segment.length =/= Bits(0) &&
                    virtReg.Addr(ADDR_WIDTH-SEG_BITS-1, ALIGN_BITS) >= segment.length)
  val lengthViolReg = Reg(next = lengthViol)
  debug(lengthViolReg)  

  // State machine for handling violations
  val idle :: respRd :: waitWr :: respWr :: Nil = Enum(UInt(), 4)
  val stateReg = Reg(init = idle)
  val burstReg = Reg(UInt())

  // Trigger violation handling
  when (!io.superMode && (permViol || lengthViol)) {
    translated.Cmd := OcpCmd.IDLE
    when (stateReg === idle) {
      when (virtReg.Cmd === OcpCmd.RD) {
        stateReg := respRd
        burstReg := UInt(io.virt.burstLength-1)
      }
      when (virtReg.Cmd === OcpCmd.WR) {
        stateReg := waitWr
        burstReg := UInt(io.virt.burstLength-2)
      }
    }
  }

  // Respond to CPU to signal violation
  when (stateReg === respRd) {
    io.virt.S.Resp := OcpResp.ERR
    burstReg := burstReg - UInt(1)
    when (burstReg === UInt(0)) {
      stateReg := idle
    }
  }
  when (stateReg === waitWr) {
    burstReg := burstReg - UInt(1)
    when (burstReg === UInt(0)) {
      stateReg := respWr
    }
  }
  when (stateReg === respWr) {
    io.virt.S.Resp := OcpResp.ERR
    stateReg := idle
  }

}
