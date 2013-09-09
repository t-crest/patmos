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

package patmos

import Chisel._
import Node._

import Constants._

class Fetch(fileName : String) extends Component {
  val io = new FetchIO()

  val pcReg = Reg(resetVal = UFix(1, PC_SIZE))
  val addrEvenReg = Reg(resetVal = UFix(2, PC_SIZE))
  val addrOddReg = Reg(resetVal = UFix(1, PC_SIZE))

  val rom = Utility.readBin(fileName)
  // Split the ROM into two blocks for dual fetch
  //  val len = rom.length / 2
  //  val rom_a = Vec(len) { Bits(width = INSTR_WIDTH) }
  //  val rom_b = Vec(len) { Bits(width = INSTR_WIDTH) }
  //  for (i <- 0 until len) {
  //    rom_a(i) = rom(i * 2)
  //    rom_b(i) = rom(i * 2 + 1)
  //    val a:Bits = rom_a(i)
  //    val b:Bits = rom_b(i)
  //    println(i+" "+a.toUFix.litValue()+" "+b.toUFix.litValue())
  //  }
  //
  //  // addr_even and odd count in words. Shall this be optimized?
  //  val data_even: Bits = rom_a(addr_even(PC_SIZE-1, 1))
  //  val data_odd: Bits = rom_b(addr_odd(PC_SIZE-1, 1))
  // relay on the optimization to recognize that those addresses are always even and odd
  // TODO: maybe make it explicit

  val ispmSize = 1 << ISPM_BITS // in bytes
  val ispmAddrBits = log2Up(ispmSize / 4 / 2)
  val memEven = { Mem(ispmSize / 4 / 2, seqRead = true) { Bits(width = INSTR_WIDTH) } }
  val memOdd = { Mem(ispmSize / 4 / 2, seqRead = true) { Bits(width = INSTR_WIDTH) } }

  // write from EX - use registers - ignore stall, as reply does not hurt
  val selWrite = (io.memfe.store & (io.memfe.addr(DATA_WIDTH-1, ISPM_ONE_BIT) === Bits(0x1)))
  val wrEvenReg = Reg(selWrite & (io.memfe.addr(2) === Bits(0)))
  val wrOddReg = Reg(selWrite & (io.memfe.addr(2) === Bits(1)))
  val addrReg = Reg(io.memfe.addr)
  val dataReg = Reg(io.memfe.data)
  when(wrEvenReg) { memEven(addrReg(ispmAddrBits + 3 - 1, 3)) := dataReg }
  when(wrOddReg) { memOdd(addrReg(ispmAddrBits + 3 - 1, 3)) := dataReg }
  // This would not work with asynchronous reset as the address
  // registers are set on reset. However, chisel uses synchronous
  // reset, which 'just' generates some more logic. And it looks
  // like the synthesize tool is able to duplicate the register.

  val selIspm = Reg(io.mcachefe.mem_sel(1))
  val selMCache = Reg(io.mcachefe.mem_sel(0))

  //need to register these values to save them in  memory stage at call/return
  val relBaseReg = Reg(resetVal = UFix(1, DATA_WIDTH))
  val relocReg = Reg(resetVal = UFix(1, DATA_WIDTH))
  when(io.memfe.doCallRet && io.ena) {
    relBaseReg := io.mcachefe.relBase
    relocReg := io.mcachefe.reloc
  }

  //select even/odd from ispm
  val ispm_even = memEven(addrEvenReg(ispmAddrBits, 1))
  val ispm_odd = memOdd(addrOddReg(ispmAddrBits, 1))
  val instr_a_ispm = Mux(pcReg(0) === Bits(0), ispm_even, ispm_odd)
  val instr_b_ispm = Mux(pcReg(0) === Bits(0), ispm_odd, ispm_even)

  //select even/odd from rom
  val data_even = rom(addrEvenReg)
  val data_odd = rom(addrOddReg)
  val instr_a_rom = Mux(pcReg(0) === Bits(0), data_even, data_odd)
  val instr_b_rom = Mux(pcReg(0) === Bits(0), data_odd, data_even)

  //MCache/ISPM/ROM Mux
  val instr_a = Mux(selIspm, instr_a_ispm,
                    Mux(selMCache, io.mcachefe.instr_a, instr_a_rom))
  val instr_b = Mux(selIspm, instr_b_ispm,
                    Mux(selMCache, io.mcachefe.instr_b, instr_b_rom))

  val b_valid = instr_a(31) === Bits(1)

  val pc_cont = Mux(b_valid, pcReg + UFix(2), pcReg + UFix(1))
  val pc_next =
    Mux(io.memfe.doCallRet, io.mcachefe.relPc.toUFix,
        	Mux(io.exfe.doBranch, io.exfe.branchPc,
        		pc_cont))
  val pc_cont2 = Mux(b_valid, pcReg + UFix(4), pcReg + UFix(3))
  val pc_next2 =
    Mux(io.memfe.doCallRet, io.mcachefe.relPc.toUFix + UFix(2),
		Mux(io.exfe.doBranch, io.exfe.branchPc + UFix(2),
			pc_cont2))

  val pc_inc = Mux(pc_next(0), pc_next2, pc_next)
  when(io.ena) {
    addrEvenReg := Cat((pc_inc)(PC_SIZE - 1, 1), Bits(0)).toUFix
    addrOddReg := Cat((pc_next)(PC_SIZE - 1, 1), Bits(1)).toUFix
    pcReg := pc_next
  }

  io.fedec.pc := pcReg
  io.fedec.relPc := pcReg - relBaseReg
  io.fedec.reloc := relocReg
  io.fedec.instr_a := instr_a
  io.fedec.instr_b := instr_b

  io.femem.pc := pc_cont - relBaseReg

  //outputs to mcache
  io.femcache.address := pc_next
  io.femcache.request := selMCache

}
