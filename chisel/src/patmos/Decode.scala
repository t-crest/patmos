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
  io.rfRead.rsAddr(0) := io.in.instr_a(16, 12)
  io.rfRead.rsAddr(1) := io.in.instr_a(11, 7)
  
  // on R0 destiantion just disable wrEna
  
  val decReg = Reg(io.in)
  
  val instr = decReg.instr_a
  // keep it in a way that is easy to refactor into a function for
  // dual issue decode
  val func = Bits(width=4)
  func := instr(3, 0)
  when (instr(26, 25) === Bits("b00")) {
    func := Cat(Bits(0), instr(24, 22))
  }
  
  io.out.pc := decReg.pc
  io.out.func := func
  // forward RF addresses and data
  io.out.rsAddr(0) := decReg.instr_a(16, 12)
  io.out.rsAddr(1) := decReg.instr_a(11, 7)
  io.out.rsData(0) := io.rfRead.rsData(0)
  io.out.rsData(1) := io.rfRead.rsData(1)
}