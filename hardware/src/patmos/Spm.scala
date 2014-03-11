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

class Spm(size: Int) extends Module {
  val io = new OcpCoreSlavePort(log2Up(size), DATA_WIDTH)

  val addrBits = log2Up(size / BYTES_PER_WORD)

  // generate byte memories
  val mem = new Array[MemBlockIO](BYTES_PER_WORD)
  for (i <- 0 until BYTES_PER_WORD) {
    mem(i) = MemBlock(size / BYTES_PER_WORD, BYTE_WIDTH).io
  }

  // store
  val stmsk = Mux(io.M.Cmd === OcpCmd.WR, io.M.ByteEn,  Bits("b0000"))
  for (i <- 0 until BYTES_PER_WORD) {
    mem(i) <= (stmsk(i), io.M.Addr(addrBits + 1, 2),
               io.M.Data(BYTE_WIDTH*(i+1)-1, BYTE_WIDTH*i))
  }

  // load
  val rdData = mem.map(_(io.M.Addr(addrBits + 1, 2))).reduceLeft((x,y) => y ## x)

  // Respond and return data
  val cmdReg = Reg(next = io.M.Cmd)
  io.S.Resp := Mux(cmdReg === OcpCmd.WR || cmdReg === OcpCmd.RD,
                   OcpResp.DVA, OcpResp.NULL)
  io.S.Data := rdData
}
