/*
 * Fetch stage of Patmos.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package patmos

import Chisel._

import Constants._

import util.Utility

class Fetch(fileName : String) extends Module {
  val io = IO(new FetchIO())

  val pcReg = RegInit(UInt(1, PC_SIZE))
  val addrEven = Wire(UInt())
  val addrOdd = Wire(UInt())
  val addrEvenReg = Reg(init = UInt(2, PC_SIZE), next = addrEven)
  val addrOddReg = Reg(init = UInt(1, PC_SIZE), next = addrOdd)

  val rom = Utility.readBin(fileName, INSTR_WIDTH)
  val romAddrBits = log2Up(rom.length / 2)
  // Split the ROM into two blocks for dual fetch
  val romGroups = rom.iterator.grouped(2).withPadding(Bits(0)).toSeq
  val romEven = Vec(romGroups.map(_(0)).padTo(1 << romAddrBits, Bits(0)))
  val romOdd  = Vec(romGroups.map(_(1)).padTo(1 << romAddrBits, Bits(0)))

  val instr_a_ispm = Wire(Bits())
  val instr_b_ispm = Wire(Bits())
  instr_a_ispm := Bits(0)
  instr_b_ispm := Bits(0)
  
  if (ISPM_SIZE > 0) {
    val ispmAddrBits = log2Up(ISPM_SIZE / 4 / 2)
    val memEven = MemBlock(ISPM_SIZE / 4 / 2, INSTR_WIDTH, bypass = false)
    val memOdd = MemBlock(ISPM_SIZE / 4 / 2, INSTR_WIDTH, bypass = false)

    // write from EX - use registers - ignore stall, as reply does not hurt
    val selWrite = (io.memfe.store & (io.memfe.addr(DATA_WIDTH-1, ISPM_ONE_BIT) === Bits(0x1)))
    val wrEven = selWrite & (io.memfe.addr(2) === Bits(0))
    val wrOdd = selWrite & (io.memfe.addr(2) === Bits(1))
    memEven.io <= (wrEven, io.memfe.addr(ispmAddrBits+2, 3), io.memfe.data)
    memOdd.io <= (wrOdd, io.memfe.addr(ispmAddrBits+2, 3), io.memfe.data)

    //select even/odd from ispm
    val ispm_even = memEven.io(addrEven(ispmAddrBits, 1))
    val ispm_odd = memOdd.io(addrOdd(ispmAddrBits, 1))
    instr_a_ispm := Mux(pcReg(0) === Bits(0), ispm_even, ispm_odd)
    instr_b_ispm := Mux(pcReg(0) === Bits(0), ispm_odd, ispm_even)
  } else if (Driver.backend.isInstanceOf[CppBackend]) {
    // dummy blocks to keep the emulator happy
    val memEven = MemBlock(1, INSTR_WIDTH, bypass = false)
    val memOdd = MemBlock(1, INSTR_WIDTH, bypass = false)
  }

  val selSpm = RegInit(Bool(false))
  val selCache = RegInit(Bool(false))
  when (io.ena) {
    selSpm := io.icachefe.memSel(1)
    selCache := io.icachefe.memSel(0)
  }

  //need to register these values to save them in  memory stage at call/return
  val baseReg = RegInit(UInt(0, width = ADDR_WIDTH))
  val relBaseReg = RegInit(UInt(1, width = MAX_OFF_WIDTH))
  val relocReg = RegInit(UInt(0, DATA_WIDTH))
  when(io.ena) {
    baseReg := io.icachefe.base
    when (io.memfe.doCallRet) {
      relBaseReg := io.icachefe.relBase
      relocReg := io.icachefe.reloc
    }
  }

  //select even/odd from rom
  // For some weird reason, Quartus infers the ROM as memory block
  // only if the output is registered
  val data_even = RegNext(romEven(addrEven(romAddrBits, 1)))
  val data_odd = RegNext(romOdd(addrOdd(romAddrBits, 1)))
  val instr_a_rom = Mux(pcReg(0) === Bits(0), data_even, data_odd)
  val instr_b_rom = Mux(pcReg(0) === Bits(0), data_odd, data_even)

  //select even/odd from method cache
  val instr_a_cache = Mux(pcReg(0) === Bits(0), io.icachefe.instrEven, io.icachefe.instrOdd)
  val instr_b_cache = Mux(pcReg(0) === Bits(0), io.icachefe.instrOdd, io.icachefe.instrEven)

  //Icache/ISPM/ROM Mux
  val instr_a = Mux(selSpm, instr_a_ispm,
                    Mux(selCache, instr_a_cache, instr_a_rom))
  val instr_b = Mux(selSpm, instr_b_ispm,
                    Mux(selCache, instr_b_cache, instr_b_rom))

  val b_valid = instr_a(31) === Bits(1)

  val pc_cont = Mux(b_valid, pcReg + UInt(2), pcReg + UInt(1))
  val pc_next =
    Mux(io.memfe.doCallRet, io.icachefe.relPc.toUInt,
            Mux(io.exfe.doBranch, io.exfe.branchPc,
                pc_cont))
  val pc_cont2 = Mux(b_valid, pcReg + UInt(4), pcReg + UInt(3))
  val pc_next2 =
    Mux(io.memfe.doCallRet, io.icachefe.relPc.toUInt + UInt(2),
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

  val relPc = pcReg - relBaseReg

  io.fedec.pc := pcReg
  io.fedec.base := baseReg
  io.fedec.reloc := relocReg
  io.fedec.relPc := relPc
  io.fedec.instr_a := instr_a
  io.fedec.instr_b := instr_b

  io.feex.pc := Mux(b_valid, relPc + UInt(2), relPc + UInt(1))

  //outputs to icache
  io.feicache.addrEven := addrEven
  io.feicache.addrOdd := addrOdd

}
