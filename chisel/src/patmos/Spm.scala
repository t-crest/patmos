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
 * An on-chip memory.
 * 
 * Has input registers (without enable or reset).
 * Shall do byte enable.
 * Output multiplexing and bit filling at the moment also here.
 * That might move out again when more than one memory is involved.
 * 
 * Address decoding here. At the moment map to 0x00000000.
 * Only take care on a write.
 * 
 * Size is in bytes.
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 */

package patmos

import Chisel._
import Node._

import Constants._

class Spm(size: Int) extends Component {
  val io = new SimpCon().flip

  // Unconditional registers for the on-chip memory
  // All stall/enable handling has been done in the input with a MUX
  val ioReg = Reg(io)

  // Compute write enable
  val stmsk = Mux(io.wr, io.byteEna,  Bits("b0000"))
  val stmskReg = Reg(stmsk)

  // I would like to have a vector of memories.
  // val mem = Vec(4) { Mem(size, seqRead = true) { Bits(width = DATA_WIDTH) } }

  // ok, the dumb way
  val addrBits = log2Up(size / BYTES_PER_WORD)
  val mem0 = { Mem(size / BYTES_PER_WORD, seqRead = true) { Bits(width = BYTE_WIDTH) } }
  val mem1 = { Mem(size / BYTES_PER_WORD, seqRead = true) { Bits(width = BYTE_WIDTH) } }
  val mem2 = { Mem(size / BYTES_PER_WORD, seqRead = true) { Bits(width = BYTE_WIDTH) } }
  val mem3 = { Mem(size / BYTES_PER_WORD, seqRead = true) { Bits(width = BYTE_WIDTH) } }

  // store
  when(stmskReg(0)) { mem0(ioReg.address(addrBits + 1, 2)) := ioReg.wrData(0) }
  when(stmskReg(1)) { mem1(ioReg.address(addrBits + 1, 2)) := ioReg.wrData(1) }
  when(stmskReg(2)) { mem2(ioReg.address(addrBits + 1, 2)) := ioReg.wrData(2) }
  when(stmskReg(3)) { mem3(ioReg.address(addrBits + 1, 2)) := ioReg.wrData(3) }

  // load
  val rdData = Vec(BYTES_PER_WORD) { Bits(width = BYTE_WIDTH) }
  rdData(0) := mem0(ioReg.address(addrBits + 1, 2))
  rdData(1) := mem1(ioReg.address(addrBits + 1, 2))
  rdData(2) := mem2(ioReg.address(addrBits + 1, 2))
  rdData(3) := mem3(ioReg.address(addrBits + 1, 2))

  // Return data immediately
  io.rdData := rdData
  io.rdyCnt := Bits("b00")

  // Delay result by one cycle to test stalling
  // io.rdData := Reg(rdData)
  // io.rdyCnt := Reg(io.rd || io.wr)
}
