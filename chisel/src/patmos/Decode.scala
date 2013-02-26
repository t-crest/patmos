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
 * Decode stage of Patmos.
 * 
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * 
 */

package patmos

import Chisel._
import Node._

class Decode() extends Component {
  val io = new DecodeIO()
  
  // register file is connected with unregistered instruction word
  io.rfRead.rsAddr(0) := io.fedec.instr_a(16, 12)
  io.rfRead.rsAddr(1) := io.fedec.instr_a(11, 7)
  
  // on R0 destiantion just disable wrEna

//  val decReg = Reg(resetVal = new FeDec())
  val decReg = Reg(new FeDec())
  when (io.ena) {
    decReg := io.fedec
  }
  
  val instr = decReg.instr_a
  // keep it in a way that is easy to refactor into a function for
  // dual issue decode
  
  val func = Bits(width=4)
  io.decex.immOp := Bool(false)
  // ALU register and long immediate
  func := instr(3, 0)
  // ALU immediate
  when (instr(26, 25) === Bits("b00")) {
    func := Cat(Bits(0), instr(24, 22))
    io.decex.immOp := Bool(true)  
  }
  // TODO sign extend
  io.decex.immVal := Cat(Bits(0), instr(11, 0))
  
  io.decex.pc := decReg.pc
  io.decex.func := func
  // forward RF addresses and data
  io.decex.rsAddr(0) := decReg.instr_a(16, 12)
  io.decex.rsAddr(1) := decReg.instr_a(11, 7)
  io.decex.rdAddr(0) := decReg.instr_a(21, 17)
  io.decex.rsData(0) := io.rfRead.rsData(0)
  io.decex.rsData(1) := io.rfRead.rsData(1)
}