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
 * Execution stage of Patmos.
 * 
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * 
 */

package patmos

import Chisel._
import Node._

class Execute() extends Component {
  val io = new ExecuteIO()

  val exReg = Reg(new DecEx())
  when(io.ena) {
    exReg := io.decex
  }
  // no access to io.decex after this point!!!

  // data forwarding
  val fwEx0 = exReg.rsAddr(0) === io.exResult.addr && io.exResult.valid
  val fwMem0 = exReg.rsAddr(0) === io.memResult.addr && io.memResult.valid
  val ra = Mux(fwEx0, io.exResult.data, Mux(fwMem0, io.memResult.data, exReg.rsData(0)))
  val fwEx1 = exReg.rsAddr(1) === io.exResult.addr && io.exResult.valid
  val fwMem1 = exReg.rsAddr(1) === io.memResult.addr && io.memResult.valid
  val rb = Mux(fwEx1, io.exResult.data, Mux(fwMem1, io.memResult.data, exReg.rsData(1)))

  val op2 = Mux(exReg.immOp, exReg.immVal, rb)

  // ALU operation
  val result = UFix(width=32)
  result := UFix(0)
  switch(exReg.func) {
    is (Bits("b0000")) { result := ra + op2 }
    is (Bits("b0001")) { result := ra - op2 }
  }

  io.exmem.rd.addr := exReg.rdAddr(0)
  io.exmem.rd.data := result
  io.exmem.rd.valid := exReg.wrReg

  // TODO: we should have a dummy field for this kind of nonsense
  io.exmem.pc := exReg.pc + exReg.rsData(0) + exReg.rsData(1) +
    exReg.rsAddr(0) + exReg.rsAddr(1) + exReg.func + io.exmem.rd.data

}