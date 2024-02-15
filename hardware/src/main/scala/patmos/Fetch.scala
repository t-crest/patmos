/*
 * Fetch stage of Patmos.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * Edited: Bosse Bandowski (bosse.bandowski@outlook.com)
 *
 */

package patmos

import Chisel._

import chisel3.dontTouch

import Constants._

import util.Utility
import util.BlackBoxRom

class Fetch(fileName : String) extends Module {
  val io = IO(new FetchIO())

  val pcReg = RegInit(1.U(PC_SIZE.W))
  val pcNext = dontTouch(Wire(UInt(PC_SIZE.W))) // for emulator
  val addrEven = Wire(UInt())
  val addrOdd = Wire(UInt())
  val addrEvenReg = Reg(init = 2.U(PC_SIZE.W), next = addrEven)
  val addrOddReg = Reg(init = 1.U(PC_SIZE.W), next = addrOdd)

  // Instantiate dual issue ROM
  val romContents = Utility.binToDualRom(fileName, INSTR_WIDTH)
  val romAddrUInt = log2Up(romContents._1.length)
  val rom = Module(new BlackBoxRom(romContents, romAddrUInt))

  
  val instr_a_ispm = Wire(UInt())
  val instr_b_ispm = Wire(UInt())
  instr_a_ispm := 0.U
  instr_b_ispm := 0.U
  
  

  if (ISPM_SIZE > 0) {
    val ispmAddrUInt = log2Up(ISPM_SIZE / 4 / 2)
    val memEven = MemBlock(ISPM_SIZE / 4 / 2, INSTR_WIDTH)
    val memOdd = MemBlock(ISPM_SIZE / 4 / 2, INSTR_WIDTH)

    // write from EX - use registers - ignore stall, as reply does not hurt
    val selWrite = (io.memfe.store & (io.memfe.addr(DATA_WIDTH-1, ISPM_ONE_BIT) === 0x1.U))
    val wrEven = selWrite & (io.memfe.addr(2) === 0.U)
    val wrOdd = selWrite & (io.memfe.addr(2) === 1.U)
    memEven.io <= (wrEven, io.memfe.addr(ispmAddrUInt+2, 3), io.memfe.data)
    memOdd.io <= (wrOdd, io.memfe.addr(ispmAddrUInt+2, 3), io.memfe.data)

    //select even/odd from ispm
    val ispm_even = memEven.io(addrEven(ispmAddrUInt, 1))
    val ispm_odd = memOdd.io(addrOdd(ispmAddrUInt, 1))
    instr_a_ispm := Mux(pcReg(0) === 0.U, ispm_even, ispm_odd)
    instr_b_ispm := Mux(pcReg(0) === 0.U, ispm_odd, ispm_even)
  } /*else if (Driver.backend.isInstanceOf[CppBackend]) {
    // dummy blocks to keep the emulator happy
    val memEven = MemBlock(1, INSTR_WIDTH, bypass = false)
    val memOdd = MemBlock(1, INSTR_WIDTH, bypass = false)
  }*/

  val selSpm = RegInit(false.B)
  val selCache = RegInit(false.B)
  val selSpmNext = dontTouch(Wire(Bool())) // for emulator
  val selCacheNext = dontTouch(Wire(Bool())) // for emulator
  selSpmNext := selSpm
  selSpm := selSpmNext
  selCacheNext := selCache
  selCache := selCacheNext
  when (io.ena) {
    selSpmNext := io.icachefe.memSel(1)
    selCacheNext := io.icachefe.memSel(0)
  }

  //need to register these values to save them in  memory stage at call/return
  val baseReg = RegInit(0.U(ADDR_WIDTH.W))
  val relBaseReg = RegInit(1.U(MAX_OFF_WIDTH.W))
  val relocReg = RegInit(0.U(DATA_WIDTH.W))
  val relBaseNext = dontTouch(Wire(UInt(MAX_OFF_WIDTH.W))) // for emulator
  val relocNext = dontTouch(Wire(UInt(DATA_WIDTH.W))) // for emulator

  relBaseNext := relBaseReg
  relBaseReg := relBaseNext
  relocNext := relocReg
  relocReg := relocNext
  when(io.ena) {
    baseReg := io.icachefe.base
    when (io.memfe.doCallRet) {
      relBaseNext := io.icachefe.relBase
      relocNext := io.icachefe.reloc
    }
  }

  //select even/odd from rom
  // For some weird reason, Quartus infers the ROM as memory block
  // only if the output is registered
  // val data_even = RegNext(romEven(addrEven(romAddrUInt, 1)))
  // val data_odd = RegNext(romOdd(addrOdd(romAddrUInt, 1)))

  rom.io.addressEven := addrEven(romAddrUInt, 1)
  rom.io.addressOdd := addrOdd(romAddrUInt, 1)
  val data_even = RegNext(rom.io.instructionEven)
  val data_odd = RegNext(rom.io.instructionOdd)
  
  val instr_a_rom = Mux(pcReg(0) === 0.U, data_even, data_odd)
  val instr_b_rom = Mux(pcReg(0) === 0.U, data_odd, data_even)

  //select even/odd from method cache
  val instr_a_cache = Mux(pcReg(0) === 0.U, io.icachefe.instrEven, io.icachefe.instrOdd)
  val instr_b_cache = Mux(pcReg(0) === 0.U, io.icachefe.instrOdd, io.icachefe.instrEven)

  //Icache/ISPM/ROM Mux
  val instr_a = Mux(selSpm, instr_a_ispm,
                    Mux(selCache, instr_a_cache, instr_a_rom))
  val instr_b = Mux(selSpm, instr_b_ispm,
                    Mux(selCache, instr_b_cache, instr_b_rom))

  val b_valid = instr_a(31) === 1.U

  val pc_cont = Mux(b_valid, pcReg + 2.U, pcReg + 1.U)
  val pc_next =
    Mux(io.memfe.doCallRet, io.icachefe.relPc.asUInt,
            Mux(io.exfe.doBranch, io.exfe.branchPc,
                pc_cont))
  pcNext := pc_next
  val pc_cont2 = Mux(b_valid, pcReg + 4.U, pcReg + 3.U)
  val pc_next2 =
    Mux(io.memfe.doCallRet, io.icachefe.relPc.asUInt + 2.U,
        Mux(io.exfe.doBranch, io.exfe.branchPc + 2.U,
            pc_cont2))

  val pc_inc = Mux(pc_next(0), pc_next2, pc_next)
  addrEven := addrEvenReg
  addrOdd := addrOddReg
  when(io.ena && !reset) {
    addrEven := Cat((pc_inc)(PC_SIZE - 1, 1), 0.U).asUInt
    addrOdd := Cat((pc_next)(PC_SIZE - 1, 1), 1.U).asUInt
    pcReg := pcNext //is pc_next - needed for emulator
  }

  val relPc = pcReg - relBaseReg

  io.fedec.pc := pcReg
  io.fedec.base := baseReg
  io.fedec.reloc := relocReg
  io.fedec.relPc := relPc
  io.fedec.instr_a := instr_a
  io.fedec.instr_b := instr_b

  io.feex.pc := Mux(b_valid, relPc + 2.U, relPc + 1.U)

  //outputs to icache
  io.feicache.addrEven := addrEven
  io.feicache.addrOdd := addrOdd

}
