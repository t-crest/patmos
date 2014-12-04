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

  def <= (ena : Bits, addr : Bits, data : Bits) = {
    wrAddr := addr
    wrEna := ena
    wrData := data
  }

  def apply(addr : Bits) : Bits = {
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

