/*
   Copyright 2014 Technical University of Denmark, DTU Compute.
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
 * Method cache without actual functionality
 * 
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *        Philipp Degasperi (philipp.degasperi@gmail.com)
 */

package patmos

import Chisel._
import Node._
import Constants._
import ocp._

class NullICache() extends Module {
  val io = new ICacheIO()

  val callRetBaseReg = Reg(init = UInt(1, DATA_WIDTH))
  val callAddrReg = Reg(init = UInt(1, DATA_WIDTH))
  val selIspmReg = Reg(init = Bool(false))

  io.ena_out := Bool(true)

  when (io.exicache.doCallRet && io.ena_in) {
    callRetBaseReg := io.exicache.callRetBase
    callAddrReg := io.exicache.callRetAddr
    selIspmReg := io.exicache.callRetBase(ADDR_WIDTH-1, ISPM_ONE_BIT-2) === Bits(0x1)
  }

  io.icachefe.instrEven := Bits(0)
  io.icachefe.instrOdd := Bits(0)
  io.icachefe.base := callRetBaseReg
  io.icachefe.relBase := callRetBaseReg(ISPM_ONE_BIT-3, 0)
  io.icachefe.relPc := callAddrReg + callRetBaseReg(ISPM_ONE_BIT-3, 0)
  io.icachefe.reloc := Mux(selIspmReg, UInt(1 << (ISPM_ONE_BIT - 2)), UInt(0))
  io.icachefe.memSel := Cat(selIspmReg, Bits(0))

  io.ocp_port.M.Cmd := OcpCmd.IDLE
  io.ocp_port.M.Addr := Bits(0)
  io.ocp_port.M.Data := Bits(0)
  io.ocp_port.M.DataValid := Bits(0)
  io.ocp_port.M.DataByteEn := Bits(0)
}
