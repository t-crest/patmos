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
 * Register file for Patmos.
 *
 * Needs to be extended to support two ALUs
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 */

package patmos

import Chisel._
import Node._

import Constants._

class RegisterFile() extends Module {
  val io = new RegFileIO()

  // Using Mem (instead of Vec) leads to smaller HW for single-issue config
  val rf = Mem(Bits(width = DATA_WIDTH), REG_COUNT)

  // We are registering the inputs here, similar as it would
  // be with an on-chip memory for the register file
  val addrReg = Vec.fill(2*PIPE_COUNT) { Reg(UInt(width=REG_BITS)) }
  val wrReg   = Vec.fill(PIPE_COUNT)   { Reg(new Result()) }
  val fwReg   = Vec.fill(2*PIPE_COUNT) { Vec.fill(PIPE_COUNT) { Reg(Bool()) } }

  when (io.ena) {
    addrReg := io.rfRead.rsAddr
    wrReg := io.rfWrite
    for (i <- 0 until 2*PIPE_COUNT) {
      for (k <- 0 until PIPE_COUNT) {
        fwReg(i)(k) := io.rfRead.rsAddr(i) === io.rfWrite(k).addr && io.rfWrite(k).valid
      }
    }
  }

  // RF internal forwarding
  for (i <- 0 until 2*PIPE_COUNT) {
    io.rfRead.rsData(i) := rf(addrReg(i))
    for (k <- 0 until PIPE_COUNT) {
      when (fwReg(i)(k)) {
        io.rfRead.rsData(i) := wrReg(k).data
      }
    }
    when(addrReg(i) === Bits(0)) {
      io.rfRead.rsData(i) := Bits(0)
    }
  }

  // Don't care about R0 here: reads return zero and writes to
  // register R0 are disabled in decode stage anyway
  for (k <- (0 until PIPE_COUNT).reverse) {
    when(io.ena && io.rfWrite(k).valid) {
      rf(io.rfWrite(k).addr.toUInt) := io.rfWrite(k).data
    }
  }

  // Signal for debugging register values
  val rfDebug = Vec.fill(REG_COUNT) { Reg(Bits(width = DATA_WIDTH)) }
  for(i <- 0 until REG_COUNT) {
    rfDebug(i) := rf(Bits(i))
    // Keep signal alive
    debug(rfDebug(i))
  }
}
