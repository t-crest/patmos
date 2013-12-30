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

import util._

class Fetch(fileName : String) extends Module {
  val io = new FetchIO()

  val pcReg = Reg(init = UInt(1, PC_SIZE))
  val addrEven = UInt()
  val addrOdd = UInt()
  val addrEvenReg = Reg(init = UInt(2, PC_SIZE), next = addrEven)
  val addrOddReg = Reg(init = UInt(1, PC_SIZE), next = addrOdd)

  val rom = Utility.readBin(fileName, INSTR_WIDTH)
  val romAddrBits = log2Up(rom.length / 2)
  // Split the ROM into two blocks for dual fetch
  val romGroups = rom.iterator.grouped(2).withPadding(Bits(0)).toSeq
  val romEven = Vec(romGroups.map(_(0)))
  val romOdd  = Vec(romGroups.map(_(1)))

  val ispmAddrBits = log2Up(ISPM_SIZE / 4 / 2)
  val memEven = MemBlock(ISPM_SIZE / 4 / 2, INSTR_WIDTH)
  val memOdd = MemBlock(ISPM_SIZE / 4 / 2, INSTR_WIDTH)

  // write from EX - use registers - ignore stall, as reply does not hurt
  val selWrite = (io.memfe.store & (io.memfe.addr(DATA_WIDTH-1, ISPM_ONE_BIT) === Bits(0x1)))
  val wrEven = selWrite & (io.memfe.addr(2) === Bits(0))
  val wrOdd = selWrite & (io.memfe.addr(2) === Bits(1))
  memEven.io <= (wrEven, io.memfe.addr, io.memfe.data)
  memOdd.io <= (wrOdd, io.memfe.addr, io.memfe.data)

  val selIspm = Reg(next = io.mcachefe.memSel(1))
  val selMCache = Reg(next = io.mcachefe.memSel(0))

  //need to register these values to save them in  memory stage at call/return
  val relBaseReg = Reg(init = UInt(1, width = MAX_OFF_WIDTH))
  val relocReg = Reg(init = UInt(0, DATA_WIDTH))
  when(io.memfe.doCallRet && io.ena) {
    relBaseReg := io.mcachefe.relBase
    relocReg := io.mcachefe.reloc
  }

  //select even/odd from ispm
  val ispm_even = memEven.io(addrEven(ispmAddrBits, 1))
  val ispm_odd = memOdd.io(addrOdd(ispmAddrBits, 1))
  val instr_a_ispm = Mux(pcReg(0) === Bits(0), ispm_even, ispm_odd)
  val instr_b_ispm = Mux(pcReg(0) === Bits(0), ispm_odd, ispm_even)

  //select even/odd from rom
  // For some weird reason, Quartus infers the ROM as memory block
  // only if the output is registered
  val data_even = Reg(next = romEven(addrEven(romAddrBits, 1)))
  val data_odd = Reg(next = romOdd(addrOdd(romAddrBits, 1)))
  val instr_a_rom = Mux(pcReg(0) === Bits(0), data_even, data_odd)
  val instr_b_rom = Mux(pcReg(0) === Bits(0), data_odd, data_even)

  //MCache/ISPM/ROM Mux
  val instr_a = Mux(selIspm, instr_a_ispm,
                    Mux(selMCache, io.mcachefe.instrA, instr_a_rom))
  val instr_b = Mux(selIspm, instr_b_ispm,
                    Mux(selMCache, io.mcachefe.instrB, instr_b_rom))

  val b_valid = instr_a(31) === Bits(1)

  val pc_cont = Mux(b_valid, pcReg + UInt(2), pcReg + UInt(1))
  val pc_next =
    Mux(io.memfe.doCallRet, io.mcachefe.relPc.toUInt,
        	Mux(io.exfe.doBranch, io.exfe.branchPc,
        		pc_cont))
  val pc_cont2 = Mux(b_valid, pcReg + UInt(4), pcReg + UInt(3))
  val pc_next2 =
    Mux(io.memfe.doCallRet, io.mcachefe.relPc.toUInt + UInt(2),
		Mux(io.exfe.doBranch, io.exfe.branchPc + UInt(2),
			pc_cont2))

  val pc_inc = Mux(pc_next(0), pc_next2, pc_next)
  addrEven := addrEvenReg
  addrOdd := addrOddReg
  when(io.ena && !reset) {
    addrEven := Cat((pc_inc)(PC_SIZE - 1, 1), Bits(0)).toUInt
    addrOdd := Cat((pc_next)(PC_SIZE - 1, 1), Bits(1)).toUInt
    pcReg := pc_next
  }

  io.fedec.pc := pcReg
  io.fedec.reloc := relocReg
  io.fedec.instr_a := instr_a
  io.fedec.instr_b := instr_b

  val relPc = pcReg - relBaseReg
  io.femem.pc := Mux(b_valid, relPc + UInt(2), relPc + UInt(1))

  //outputs to mcache
  io.femcache.addrEven := Mux(io.ena, pc_inc, pcReg+pcReg(0))
  io.femcache.addrOdd := Mux(io.ena, pc_next, pcReg)
  io.femcache.request := selMCache
  io.femcache.doCallRet := io.memfe.doCallRet
  io.femcache.callRetBase := io.memfe.callRetBase

}
