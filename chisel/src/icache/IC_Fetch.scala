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
 * Fetch stage of Patmos.
 * 
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * 
 */

package icache

import Chisel._
import Node._

//class Fetch(fileName: String) extends Component {
class Fetch() extends Component {
  val io = new FetchIO()

  val pc = Reg(resetVal = UFix(0, Constants.PC_SIZE))
  val addr_even = Reg(resetVal = UFix(0, Constants.PC_SIZE - 1))
  val addr_odd = Reg(resetVal = UFix(1, Constants.PC_SIZE - 1))

  val instr_a = io.icache_out.instr_a
  val instr_b = io.icache_out.instr_b

  //NOT used since we don't have two instr. with direct mapped cache
  /*
   val data_even = io.icache_out.instr_a
   val data_odd = io.icache_out.instr_b
   val instr_a = Mux(pc(0) === Bits(0), data_even, data_odd)
   val instr_b = Mux(pc(0) === Bits(0), data_odd, data_even)
   */

  // This becomes an issue when no bit 31 is set in the ROM!
  // Too much optimization happens here. We set the unused words with bit 31 set.
  // Probably an instruction SPM will help to avoid this optimization.
  val b_valid = instr_a(31) === Bits(1)
  val pc_next = Mux(io.exfe.doBranch,
    io.exfe.branchPc,
    pc + Mux(b_valid, UFix(2), Bits(1)))

  // TODO clean up
  //  val addEven = Mux(pc_next(0) === Bits(1), UFix(0), UFix(1))
  val xyz = Cat(pc_next(Constants.PC_SIZE - 1, 1), Bits(0))
  val abc = Cat(pc_next(Constants.PC_SIZE - 1, 1) + UFix(1), Bits(0))
  val even_next = Mux(pc_next(0) === Bits(1), abc, xyz)

  when (io.icache_out.ena) {
    addr_even := even_next.toUFix
    addr_odd := Cat(pc_next(Constants.PC_SIZE - 1, 1), Bits(1)).toUFix
    pc := pc_next
  }

  io.fedec.pc := pc
  io.fedec.instr_a := instr_a
  io.fedec.instr_b := instr_b
  io.fedec.b_valid := b_valid // not used at the moment

  io.icache_in.address := pc_next
}
