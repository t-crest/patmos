/*
   Copyright 2015 Technical University of Denmark, DTU Compute.
   All rights reserved.

   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

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
  val ALIGN_BITS = 10

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
  val io = new MMUIO()

  val masterReg = Reg(next = io.ctrl.M)

  def checked(action: => Unit) {
    when (io.superMode) (action) .otherwise { io.ctrl.S.Resp := OcpResp.ERR }
  }

  val segmentVec = Reg(Vec.fill(SEG_COUNT)(new Segment()))

  // Default OCP response
  io.ctrl.S.Resp := OcpResp.NULL
  io.ctrl.S.Data := Bits(0, width = DATA_WIDTH)

  // Handle OCP reads and writes for control interface
  when(masterReg.Cmd === OcpCmd.RD) {
    io.ctrl.S.Resp := OcpResp.DVA
  }
  when(masterReg.Cmd === OcpCmd.WR) {
    io.ctrl.S.Resp := OcpResp.DVA
    checked{
      val seg = segmentVec(masterReg.Addr(SEG_BITS+2, 3))
      when (masterReg.Addr(2) === Bits(0)) {
        seg.base := masterReg.Data(DATA_WIDTH-1, ALIGN_BITS)
      } .otherwise {
        seg.perm := masterReg.Data(DATA_WIDTH-1, DATA_WIDTH-PERM_BITS)
        seg.length := masterReg.Data(DATA_WIDTH-PERM_BITS-1, ALIGN_BITS)
      }
    }
  }

  // Address translation
  val virtReg = Reg(next = io.virt.M)
  val execReg = Reg(next = io.exec)
  val segment = segmentVec(virtReg.Addr(ADDR_WIDTH-1, ADDR_WIDTH-SEG_BITS))
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
