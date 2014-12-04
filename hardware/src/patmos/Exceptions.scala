/*
   Copyright 2013 Technical University of Denmark, DTU Compute.
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
 * Exceptions for Patmos.
 *
 * Author: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patmos

import Chisel._
import Node._

import Constants._

import ocp._

class Exceptions extends Module {
  val io = new ExcIO()

  val EXC_ADDR_WIDTH = 8

  val masterReg = Reg(next = io.ocp.M)

  val statusReg = Reg(init = Bits(0, width = DATA_WIDTH))
  val maskReg   = Reg(Bits(width = DATA_WIDTH))
  val sourceReg = Reg(Bits(width = DATA_WIDTH))

  val vec    = Mem(UInt(width = DATA_WIDTH), EXC_COUNT)
  val vecDup = Mem(UInt(width = DATA_WIDTH), EXC_COUNT)

  val sleepReg = Reg(init = Bool(false))

  // Latches for incoming exceptions and interrupts
  val excPend     = Vec.fill(EXC_COUNT) { Bool() }
  val excPendReg  = Vec.fill(EXC_COUNT) { Reg(init = Bool(false)) }
  val intrPend    = Vec.fill(EXC_COUNT) { Bool() }
  val intrPendReg = Vec.fill(EXC_COUNT) { Reg(init = Bool(false)) }
  excPend := excPendReg
  intrPend := intrPendReg

  // Default OCP response
  io.ocp.S.Resp := OcpResp.NULL
  io.ocp.S.Data := Bits(0, width = DATA_WIDTH)

  // No resetting by default
  io.invalMCache := Bool(false)
  io.invalDCache := Bool(false)

  // Handle OCP reads and writes
  when(masterReg.Cmd === OcpCmd.RD) {
    io.ocp.S.Resp := OcpResp.DVA

    switch(masterReg.Addr(EXC_ADDR_WIDTH-1, 2)) {
      is(Bits("b000000")) { io.ocp.S.Data := statusReg }
      is(Bits("b000001")) { io.ocp.S.Data := maskReg }
      is(Bits("b000011")) { io.ocp.S.Data := sourceReg }
      is(Bits("b000010")) { io.ocp.S.Data := intrPendReg.toBits }
    }
    when(masterReg.Addr(EXC_ADDR_WIDTH-1) === Bits("b1")) {
      io.ocp.S.Data := vec(masterReg.Addr(EXC_ADDR_WIDTH-2, 2))
    }
  }

  when(masterReg.Cmd === OcpCmd.WR) {
    io.ocp.S.Resp := OcpResp.DVA
    switch(masterReg.Addr(EXC_ADDR_WIDTH-1, 2)) {
      is(Bits("b000000")) { statusReg := masterReg.Data }
      is(Bits("b000001")) { maskReg := masterReg.Data }
      is(Bits("b000011")) { sourceReg := masterReg.Data }
      is(Bits("b000010")) {
        for(i <- 0 until EXC_COUNT) {
          intrPend(i) := intrPendReg(i) & masterReg.Data(i)
        }
      }
      is(Bits("b000100")) { // Go to sleep
                            io.ocp.S.Resp := OcpResp.NULL
                            sleepReg := Bool(true) }
      is(Bits("b000101")) { io.invalDCache := masterReg.Data(0)
                            io.invalMCache := masterReg.Data(1) }
    }
    when(masterReg.Addr(EXC_ADDR_WIDTH-1) === Bits("b1")) {
      vec(masterReg.Addr(EXC_ADDR_WIDTH-2, 2)) := masterReg.Data.toUInt
      vecDup(masterReg.Addr(EXC_ADDR_WIDTH-2, 2)) := masterReg.Data.toUInt
    }
  }

  // Acknowledgement of exception
  when(io.memexc.call) {
    excPend(io.memexc.src) := Bool(false)
    intrPend(io.memexc.src) := Bool(false)
    when(io.ena) {
      sourceReg := io.memexc.src
      statusReg := statusReg << UInt(1)
    }
  }
  // Return from exception
  when(io.memexc.ret) {
    when(io.ena) {
      statusReg := statusReg >> UInt(1)
    }
  }

  // Latch interrupt pins
  for (i <- 0 until INTR_COUNT) {
    when(Reg(next = io.intrs(i))) {
      intrPend(16+i) := Bool(true)
    }
  }

  // Trigger internal exceptions
  val excAddrReg = Reg(UInt(width = PC_SIZE))
  when(io.memexc.exc) {
    excPend(io.memexc.src) := Bool(true)
    when(io.ena) {
      excAddrReg := io.memexc.excAddr
    }
  }

  // Latch new pending flags
  excPendReg := excPend
  intrPendReg := intrPend

  // Compute next exception source
  val src = Bits(width = EXC_SRC_BITS)
  val srcReg = Reg(next = src)
  src := Bits(0)
  for (i <- (0 until EXC_COUNT).reverse) {
    when(intrPend(i) && (maskReg(i) === Bits(1))) { src := Bits(i) }
  }
  for (i <- (0 until EXC_COUNT).reverse) {
    when(excPend(i)) { src := Bits(i) }
  }

  // Create signals to decode stage
  val exc = Reg(next = excPend.toBits != Bits(0))
  val intr = Reg(next = (intrPend.toBits & maskReg) != Bits(0))

  io.excdec.exc  := exc
  io.excdec.intr := intr && statusReg(0) === Bits(1)
  io.excdec.addr := vecDup(srcReg)
  io.excdec.src  := srcReg

  io.excdec.excAddr := excAddrReg

  // Wake up
  when (sleepReg && (exc === Bits(1) || (intr && statusReg(0) === Bits(1)))) {
    io.ocp.S.Resp := OcpResp.DVA
    sleepReg := Bool(false)
  }
}
