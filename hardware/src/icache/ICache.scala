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
  Instruction Cache for Patmos
  Author: Philipp Degasperi (philipp.degasperi@gmail.com)
 */

package patmos

import Chisel._
import Node._
import IConstants._
import Constants._
import ocp._

import scala.collection.mutable.HashMap
import scala.util.Random
import scala.math

/*
  Instruction cache constants only used internally
 */
object IConstants {

  val ICACHE_WORD_SIZE = ICACHE_SIZE / 4
  val ICACHE_SIZE_WIDTH = log2Up(ICACHE_WORD_SIZE)

  val LINE_WORD_SIZE = BURST_LENGTH
  val LINE_WORD_SIZE_WIDTH = log2Up(LINE_WORD_SIZE)

  val LINE_COUNT = ICACHE_WORD_SIZE / LINE_WORD_SIZE
  val LINE_COUNT_WIDTH = log2Up(LINE_COUNT)

  val TAG_HIGH = EXTMEM_ADDR_WIDTH - 1
  val TAG_LOW = LINE_COUNT_WIDTH + LINE_WORD_SIZE_WIDTH
  val TAG_SIZE = TAG_HIGH - TAG_LOW + 1

  val INDEX_HIGH = TAG_LOW - 1
  val INDEX_LOW = LINE_WORD_SIZE_WIDTH
  val INDEX_SIZE = INDEX_HIGH - INDEX_LOW + 1
}

/*
  Internal connections for the instruction cache
 */
class ICacheCtrlIO extends Bundle() {
  val ena_in = Bool(INPUT)
  val fetch_ena = Bool(OUTPUT)
  val ctrlrepl = new ICacheCtrlRepl().asOutput
  val replctrl = new ICacheReplCtrl().asInput
  val feicache = new FeICache().asInput
  val exicache = new ExICache().asInput
  val ocp_port = new OcpBurstMasterPort(EXTMEM_ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
}
class ICacheCtrlRepl extends Bundle() {
  val wEna = Bool()
  val wData = Bits(width = INSTR_WIDTH)
  val wAddr = Bits(width = ADDR_WIDTH)
  val wTag = Bool()
}
class ICacheReplCtrl extends Bundle() {
  val hitEna = Bool()
  val fetchAddr = Bits(width = EXTMEM_ADDR_WIDTH)
  val selCache = Bool()
}
class ICacheReplIO extends Bundle() {
  val ena_in = Bool(INPUT)
  val invalidate = Bool(INPUT)
  val exicache = new ExICache().asInput
  val feicache = new FeICache().asInput
  val icachefe = new ICacheFe().asOutput
  val ctrlrepl = new ICacheCtrlRepl().asInput
  val replctrl = new ICacheReplCtrl().asOutput
  val memIn = new ICacheMemIn().asOutput
  val memOut = new ICacheMemOut().asInput
  val perf = new InstructionCachePerf()
}

class ICacheMemIn extends Bundle() {
  val wEven = Bool()
  val wOdd = Bool()
  val wData = Bits(width = DATA_WIDTH)
  val wAddr = Bits(width = INDEX_SIZE + LINE_WORD_SIZE_WIDTH)
  val addrOdd = Bits(width = INDEX_SIZE + LINE_WORD_SIZE_WIDTH)
  val addrEven = Bits(width = INDEX_SIZE + LINE_WORD_SIZE_WIDTH)
}
class ICacheMemOut extends Bundle() {
  val instrEven = Bits(width = INSTR_WIDTH)
  val instrOdd = Bits(width = INSTR_WIDTH)
}
class ICacheMemIO extends Bundle() {
  val memIn = new ICacheMemIn().asInput
  val memOut = new ICacheMemOut().asOutput
}


/*
 ICache: Top Level Class for the Instruction Cache
 */
class ICache() extends Module {
  val io = new ICacheIO()
  //generate submodules of instruction cache
  val ctrl = Module(new ICacheCtrl())
  val repl = Module(new ICacheReplDm())
  val mem = Module(new ICacheMem())
  //connect submodules of instruction cache
  ctrl.io.ctrlrepl <> repl.io.ctrlrepl
  ctrl.io.feicache <> io.feicache
  ctrl.io.exicache <> io.exicache
  ctrl.io.ocp_port <> io.ocp_port
  //connect inputs to instruction cache repl unit
  repl.io.exicache <> io.exicache
  repl.io.feicache <> io.feicache
  repl.io.icachefe <> io.icachefe
  repl.io.replctrl <> ctrl.io.replctrl
  //connect repl unit to on chip memory
  repl.io.memIn <> mem.io.memIn
  repl.io.memOut <> mem.io.memOut
  //connect enables
  ctrl.io.ena_in <> io.ena_in
  repl.io.ena_in <> io.ena_in
  //output enable depending on hit/miss/fetch
  io.ena_out := ctrl.io.fetch_ena
  //connect invalidate signal
  repl.io.invalidate := io.invalidate
}

/*
 ICacheMem Class: On-Chip Instruction Cache Memory
 */
class ICacheMem extends Module {
  val io = new ICacheMemIO()

  val icacheEven = MemBlock(ICACHE_WORD_SIZE / 2, INSTR_WIDTH)
  val icacheOdd = MemBlock(ICACHE_WORD_SIZE / 2, INSTR_WIDTH)

  icacheEven.io <= (io.memIn.wEven, io.memIn.wAddr, io.memIn.wData)
  icacheOdd.io <= (io.memIn.wOdd, io.memIn.wAddr, io.memIn.wData)

  io.memOut.instrEven := icacheEven.io(io.memIn.addrEven)
  io.memOut.instrOdd := icacheOdd.io(io.memIn.addrOdd)
}

/*
 Direct Mapped Replacement Class
 */
class ICacheReplDm() extends Module {
  val io = new ICacheReplIO()

  //reserve memory for the instruction cache tag field containing valid bit and address tag
  val tagMemEven = MemBlock(LINE_COUNT / 2, TAG_SIZE)
  val tagMemOdd = MemBlock(LINE_COUNT / 2, TAG_SIZE)
  val validVec = Vec.fill(LINE_COUNT) { Reg(init = Bool(false)) }

  //variables when call/return occurs
  val callRetBaseReg = Reg(init = UInt(1, DATA_WIDTH))
  val callAddrReg = Reg(init = UInt(1, DATA_WIDTH))
  val selSpmReg = Reg(init = Bool(false))
  val selCacheReg = Reg(init = Bool(false))

  val fetchAddr = Bits(width = EXTMEM_ADDR_WIDTH)
  val hitInstrEven = Bool()
  val hitInstrOdd = Bool()

  val relBase = Mux(selCacheReg,
                    callRetBaseReg(EXTMEM_ADDR_WIDTH-1, 0),
                    callRetBaseReg(ISPM_ONE_BIT-3, 0))
  val relPc = callAddrReg + relBase

  val reloc = Mux(selCacheReg,
                  UInt(0),
                  Mux(selSpmReg,
                      UInt(1 << (ISPM_ONE_BIT - 2)),
                      UInt(0)))

  when (io.exicache.doCallRet && io.ena_in) {
    callRetBaseReg := io.exicache.callRetBase
    callAddrReg := io.exicache.callRetAddr
    selSpmReg := io.exicache.callRetBase(EXTMEM_ADDR_WIDTH-1, ISPM_ONE_BIT-2) === Bits(0x1)
    selCacheReg := io.exicache.callRetBase(EXTMEM_ADDR_WIDTH-1, ISPM_ONE_BIT-1) >= Bits(0x1)
  }

  val addrIndexEven = (io.feicache.addrEven)(INDEX_HIGH, INDEX_LOW+1)
  val addrIndexOdd = io.feicache.addrOdd(INDEX_HIGH, INDEX_LOW+1)
  val addrTagEven = (io.feicache.addrEven)(TAG_HIGH, TAG_LOW)
  val addrTagOdd = io.feicache.addrOdd(TAG_HIGH, TAG_LOW)
  val blockParityEven = io.feicache.addrEven(INDEX_LOW)
  val blockParityOdd = io.feicache.addrOdd(INDEX_LOW)

  // Mux of tag memory input
  val addrBlockEven = Mux(blockParityEven, addrIndexOdd, addrIndexEven)
  val addrBlockOdd = Mux(blockParityEven, addrIndexEven, addrIndexOdd)
  val addrEvenReg = Reg(next = io.feicache.addrEven)
  val addrOddReg = Reg(next = io.feicache.addrOdd)
  val blockParityEvenReg = addrEvenReg(INDEX_LOW)
  val blockParityOddReg = addrOddReg(INDEX_LOW)
  val addrTagEvenReg = addrEvenReg(TAG_HIGH, TAG_LOW)
  val addrTagOddReg = addrOddReg(TAG_HIGH, TAG_LOW)
  val addrIndexEvenReg = addrEvenReg(INDEX_HIGH, INDEX_LOW)
  val addrIndexOddReg = addrOddReg(INDEX_HIGH, INDEX_LOW)
  val addrValidEven = io.feicache.addrEven(INDEX_HIGH, INDEX_LOW)
  val addrValidOdd = io.feicache.addrOdd(INDEX_HIGH, INDEX_LOW)

  // Mux at tag memory input
  val toutEven = tagMemEven.io(addrBlockEven)
  val toutOdd = tagMemOdd.io(addrBlockOdd)
  // Mux of tag memory output
  val tagEven = Mux(blockParityEvenReg, toutOdd, toutEven)
  val tagOdd = Mux(blockParityOddReg, toutOdd, toutEven)
  // valid tag
  val validTag = validVec(addrValidEven) && validVec(addrValidOdd)
  val validTagReg = Reg(next = validTag)

  //check for a hit of both instructions of the address bundle
  hitInstrEven := Bool(true)
  hitInstrOdd := Bool(true)
  when (tagEven != addrTagEvenReg) {
    hitInstrEven := Bool(false)
  }
  fetchAddr := addrEvenReg
  when (tagOdd != addrTagOddReg) {
    hitInstrOdd := Bool(false)
    fetchAddr := addrOddReg
  }
  //debug signals for emulator
  debug(hitInstrEven)
  debug(hitInstrOdd)

  val wrAddrTag = io.ctrlrepl.wAddr(TAG_HIGH,TAG_LOW)
  //index for valid field
  val wrValidIndex = io.ctrlrepl.wAddr(INDEX_HIGH, INDEX_LOW)
  //index for tag field even/odd
  val wrAddrIndex = io.ctrlrepl.wAddr(INDEX_HIGH, INDEX_LOW+1)
  val wrAddrParity = io.ctrlrepl.wAddr(INDEX_LOW)
  //update tag field when new write occurs
  tagMemEven.io <= (io.ctrlrepl.wTag && !wrAddrParity, wrAddrIndex, wrAddrTag)
  tagMemOdd.io <= (io.ctrlrepl.wTag && wrAddrParity, wrAddrIndex, wrAddrTag)
  when (io.ctrlrepl.wTag) {
    validVec(wrValidIndex) := Bool(true)
  }

  val wrParity = io.ctrlrepl.wAddr(0)

  //outputs to icache memory
  io.memIn.wEven := Mux(wrParity, Bool(false), io.ctrlrepl.wEna)
  io.memIn.wOdd := Mux(wrParity, io.ctrlrepl.wEna, Bool(false))
  io.memIn.wData := io.ctrlrepl.wData
  io.memIn.wAddr := (io.ctrlrepl.wAddr)(INDEX_HIGH,1)
  io.memIn.addrOdd := (io.feicache.addrOdd)(INDEX_HIGH,1)
  io.memIn.addrEven := (io.feicache.addrEven)(INDEX_HIGH,1)

  io.icachefe.instrEven := io.memOut.instrEven
  io.icachefe.instrOdd := io.memOut.instrOdd

  io.icachefe.relBase := relBase
  io.icachefe.relPc := relPc
  io.icachefe.reloc := reloc
  io.icachefe.memSel := Cat(selSpmReg, selCacheReg)
  //hit/miss return
  io.replctrl.fetchAddr := fetchAddr
  io.replctrl.hitEna := (hitInstrEven && hitInstrOdd && validTagReg)
  io.replctrl.selCache := selCacheReg

  when (io.invalidate) {
    validVec.map(_ := Bool(false))
  }
}

/*
 Instruction Cache Control Class: handles block transfer from external Memory to the I-Cache
 */
class ICacheCtrl() extends Module {
  val io = new ICacheCtrlIO()

  //fsm state variables
  val initState :: idleState :: transferState :: waitState :: Nil = Enum(UInt(), 4)
  val stateReg = Reg(init = initState)
  //signal for replacement unit
  val wData = Bits(width = DATA_WIDTH)
  val wTag = Bool()
  val wAddr = Bits(width = ADDR_WIDTH)
  val wEna = Bool()
  //signals for external memory
  val ocpCmd = Bits(width = 3)
  val ocpAddr = Bits(width = EXTMEM_ADDR_WIDTH)
  val fetchCnt = Reg(init = Bits(0, width = ICACHE_SIZE_WIDTH))
  val burstCnt = Reg(init = UInt(0, width = log2Up(BURST_LENGTH)))
  val fetchEna = Bool()
  //input output registers
  val addrReg = Reg(init = Bits(0, width = 32))
  val ocpSlaveReg = Reg(next = io.ocp_port.S)
  //address for the entire block
  val absFetchAddr = Cat(addrReg(EXTMEM_ADDR_WIDTH,LINE_WORD_SIZE_WIDTH), Bits(0)(LINE_WORD_SIZE_WIDTH-1,0))

  //init signals
  wData := Bits(0)
  wTag := Bool(false)
  wEna := Bool(false)
  wAddr := Bits(0)
  ocpCmd := OcpCmd.IDLE
  ocpAddr := Bits(0)
  fetchEna := Bool(true)

  // wait till ICache is the selected source
  when (stateReg === initState) {
    when (io.replctrl.selCache) {
      stateReg := idleState
    }
  }
  when (stateReg === idleState) {
    when (!io.replctrl.selCache) {
      stateReg := initState
    } .otherwise {
      when (!io.replctrl.hitEna) {
        fetchEna := Bool(false)
        addrReg := io.replctrl.fetchAddr
        burstCnt := UInt(0)
        fetchCnt := UInt(0)
        //write new tag field memory
        wTag := Bool(true)
        wAddr := Cat(io.replctrl.fetchAddr(EXTMEM_ADDR_WIDTH-1,LINE_WORD_SIZE_WIDTH), Bits(0)(LINE_WORD_SIZE_WIDTH-1,0))
        //check if command is accepted by the memory controller
        when (io.ocp_port.S.CmdAccept === Bits(1)) {
          ocpAddr := Cat(io.replctrl.fetchAddr(EXTMEM_ADDR_WIDTH-1,LINE_WORD_SIZE_WIDTH), Bits(0)(LINE_WORD_SIZE_WIDTH-1,0))
          ocpCmd := OcpCmd.RD
          stateReg := transferState
        }
        .otherwise {
          stateReg := waitState
        }
      }
    }
  }
  when (stateReg === waitState) {
    fetchEna := Bool(false)
    when (io.ocp_port.S.CmdAccept === Bits(1)) {
      ocpAddr := Cat(addrReg(EXTMEM_ADDR_WIDTH-1,LINE_WORD_SIZE_WIDTH), Bits(0)(LINE_WORD_SIZE_WIDTH-1,0))
      ocpCmd := OcpCmd.RD
    }
  }
  //transfer/fetch cache block
  when (stateReg === transferState) {
    fetchEna := Bool(false)
    when (fetchCnt < UInt(LINE_WORD_SIZE)) {
      when (ocpSlaveReg.Resp === OcpResp.DVA) {
        fetchCnt := fetchCnt + Bits(1)
        burstCnt := burstCnt + Bits(1)
        when(fetchCnt < UInt(LINE_WORD_SIZE-1)) {
          //fetch next address from external memory
          when (burstCnt >= UInt(BURST_LENGTH - 1)) {
            ocpCmd := OcpCmd.RD
            ocpAddr := Cat(addrReg(EXTMEM_ADDR_WIDTH,LINE_WORD_SIZE_WIDTH), Bits(0)(LINE_WORD_SIZE_WIDTH-1,0)) + fetchCnt + Bits(1)
            burstCnt := UInt(0)
          }
        }
        //write current address to icache memory
        wData := ocpSlaveReg.Data
        wEna := Bool(true)
      }
      wAddr := Cat(addrReg(EXTMEM_ADDR_WIDTH,LINE_WORD_SIZE_WIDTH), Bits(0)(LINE_WORD_SIZE_WIDTH-1,0)) + fetchCnt
    }
    //restart to idle state
    .otherwise {
      stateReg := idleState
    }
  }

  //outputs to instruction cache memory
  io.ctrlrepl.wEna := wEna
  io.ctrlrepl.wData := wData
  io.ctrlrepl.wAddr := wAddr
  io.ctrlrepl.wTag := wTag

  io.fetch_ena := fetchEna

  //output to external memory
  io.ocp_port.M.Addr := Cat(ocpAddr, Bits("b00"))
  io.ocp_port.M.Cmd := ocpCmd
  io.ocp_port.M.Data := Bits(0) //read-only
  io.ocp_port.M.DataByteEn := Bits("b1111") //read-only
  io.ocp_port.M.DataValid := Bits(0) //read-only
}
