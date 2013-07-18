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

import ocp._

class Spm(size: Int) extends Component {
  val io = new OcpSlavePort(log2Up(size), DATA_WIDTH)

  // Unconditional registers for the on-chip memory
  // All stall/enable handling has been done in the input with a MUX
  val masterReg = Reg(io.M)

  // Compute write enable
  val stmsk = Mux(io.M.Cmd === OcpCmd.WRNP, io.M.ByteEn,  Bits("b0000"))
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
  when(stmskReg(0)) { mem0(masterReg.Addr(addrBits + 1, 2)) :=
					   masterReg.Data(BYTE_WIDTH-1, 0) }
  when(stmskReg(1)) { mem1(masterReg.Addr(addrBits + 1, 2)) :=
					   masterReg.Data(2*BYTE_WIDTH-1, BYTE_WIDTH) }
  when(stmskReg(2)) { mem2(masterReg.Addr(addrBits + 1, 2)) :=
					   masterReg.Data(3*BYTE_WIDTH-1, 2*BYTE_WIDTH) }
  when(stmskReg(3)) { mem3(masterReg.Addr(addrBits + 1, 2)) :=
					   masterReg.Data(DATA_WIDTH-1, 3*BYTE_WIDTH) }

  // load
  val rdData = Cat(mem3(masterReg.Addr(addrBits + 1, 2)),
				   mem2(masterReg.Addr(addrBits + 1, 2)),
				   mem1(masterReg.Addr(addrBits + 1, 2)),
				   mem0(masterReg.Addr(addrBits + 1, 2)))

  // Return data immediately
  io.S.Data := rdData
  io.S.Resp := Mux(masterReg.Cmd === OcpCmd.WRNP || masterReg.Cmd === OcpCmd.RD,
   				   OcpResp.DVA, OcpResp.NULL)

  // Delay result by one cycle to test stalling
  // io.S.Data := Reg(rdData)
  // io.S.Resp := Reg(Mux(masterReg.Cmd === OcpCmd.WRNP || masterReg.Cmd === OcpCmd.RD,
  // 					   OcpResp.DVA, OcpResp.NULL))
}
