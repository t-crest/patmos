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
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * 
 */

package patmos

import Chisel._
import Node._

import Constants._

class RegisterFile() extends Component {
  val io = new RegFileIO()

  // val rf = Vec(REG_COUNT){ Reg() { Bits(width = DATA_WIDTH) } }
  // the reset version generates more logic and a slower fmax
  // Probably due to the synchronous reset
  val rf = Vec(REG_COUNT) { Reg(resetVal = Bits(0, width = DATA_WIDTH)) }

  // We are registering the inputs here, similar as it would
  // be with an on-chip memory for the register file
  val addr0Reg = Reg(UFix(width=REG_BITS))
  val addr1Reg = Reg(UFix(width=REG_BITS))
  val wrReg = Reg(new Result())
  val fw0Reg = Reg(Bool())
  val fw1Reg = Reg(Bool())
  
  // With an on-chip RAM enable would need for implementation:
  //   additional register and a MUX feeding the old value into
  //   the registers
  when (io.ena) {
    addr0Reg := io.rfRead.rsAddr(0).toUFix
    addr1Reg := io.rfRead.rsAddr(1).toUFix
    wrReg := io.rfWrite
    fw0Reg := io.rfRead.rsAddr(0) === io.rfWrite.addr && io.rfWrite.valid
    fw1Reg := io.rfRead.rsAddr(1) === io.rfWrite.addr && io.rfWrite.valid
  }

  // RF internal forwarding
  io.rfRead.rsData(0) := Mux(fw0Reg, wrReg.data, rf(addr0Reg))
  io.rfRead.rsData(1) := Mux(fw1Reg, wrReg.data, rf(addr1Reg))

  // R0 handling could be done here, in decode, or as part of forwarding.
  // At the moment we are just happy with relying on the fact that the
  // registers are reset and just disable writing to register 0

  when(wrReg.valid) {
    rf(wrReg.addr.toUFix) := wrReg.data
  }

  // Output for co-simulation with pasim
  io.rfDebug := rf
}
