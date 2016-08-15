/*
   Copyright 2015 Technical University of Denmark, DTU Compute.
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
  Authors: Philipp Degasperi (philipp.degasperi@gmail.com)
           Wolfgang Puffitsch (wpuffitsch@gmail.com)
           Bekim Cilku (bcilku@gmail.com)
 */

package patmos

import Chisel._
import Node._
import PIConstants2LRU._
import Constants._
import ocp._

import scala.collection.mutable.HashMap
import scala.util.Random
import scala.math

/*
  Instruction cache constants only used internally
 */
object PIConstants2LRU {

  val ICACHE_WORD_SIZE = ICACHE_SIZE / 4
  val ICACHE_SIZE_WIDTH = log2Up(ICACHE_WORD_SIZE)

  val LINE_WORD_SIZE = BURST_LENGTH
  val LINE_WORD_SIZE_WIDTH = log2Up(LINE_WORD_SIZE)

  val LINE_COUNT = ICACHE_WORD_SIZE / (LINE_WORD_SIZE * 2)
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
class PICache2LRUCtrlIO extends Bundle() {
  val ena_in = Bool(INPUT)
  val fetch_ena = Bool(OUTPUT)
  val ctrlrepl = new PICache2LRUCtrlRepl().asOutput
  val replctrl = new PICache2LRUReplCtrl().asInput
  val ctrlpref = new PICache2LRUCtrlPref().asOutput
  val ocp_port = new OcpBurstMasterPort(EXTMEM_ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
  val perf = new InstructionCachePerf()
}
class PICache2LRUCtrlRepl extends Bundle() {
  val wEna = Bool()
  val wData = Bits(width = INSTR_WIDTH)
  val wAddr = Bits(width = ADDR_WIDTH)
  val wTag = Bool()
}
class PICache2LRUPrefRepl extends Bundle() {
  val prefAddr = Bits(width = EXTMEM_ADDR_WIDTH)
}
class PICache2LRUCtrlPref extends Bundle() {
  val prefTrig = Bool()
}
class PICache2LRUReplCtrl extends Bundle() {
  val hit = Bool()
  val hitPref = Bool()
  val fetchAddr = Bits(width = EXTMEM_ADDR_WIDTH)
  val selCache = Bool()
}
class PICache2LRUReplIO extends Bundle() {
  val ena_in = Bool(INPUT)
  val invalidate = Bool(INPUT)
  val exicache = new ExICache().asInput
  val feicache = new FeICache().asInput
  val icachefe = new ICacheFe().asOutput
  val ctrlrepl = new PICache2LRUCtrlRepl().asInput
  val replctrl = new PICache2LRUReplCtrl().asOutput
  val prefrepl = new PICache2LRUPrefRepl().asInput
  val memIn = new PICache2LRUMemIn().asOutput
  val memOut = new PICache2LRUMemOut().asInput
}

class PICache2LRUMemIn extends Bundle() {
  val wEvenFirst = Bool()
  val wOddFirst = Bool()
  val wEvenSecond = Bool()
  val wOddSecond = Bool()
  val wData = Bits(width = DATA_WIDTH)
  val wAddr = Bits(width = INDEX_SIZE + LINE_WORD_SIZE_WIDTH)
  val addrOdd = Bits(width = INDEX_SIZE + LINE_WORD_SIZE_WIDTH)
  val addrEven = Bits(width = INDEX_SIZE + LINE_WORD_SIZE_WIDTH)
}   
class PICache2LRUMemOut extends Bundle() {
  val instrEvenFirst = Bits(width = INSTR_WIDTH)
  val instrOddFirst = Bits(width = INSTR_WIDTH)
  val instrEvenSecond = Bits(width = INSTR_WIDTH)
  val instrOddSecond = Bits(width = INSTR_WIDTH)
}
class PICache2LRUMemIO extends Bundle() {
  val memIn = new PICache2LRUMemIn().asInput
  val memOut = new PICache2LRUMemOut().asOutput
}
class Prefetcher2LRUIO extends Bundle() {
  val invalidate = Bool(INPUT)
  val feicache = new FeICache().asInput
  val ctrlpref = new PICache2LRUCtrlPref().asInput
  val prefrepl = new PICache2LRUPrefRepl().asOutput
}

/*
 ICache: Top Level Class for the Instruction Cache
 */
class PICache2LRU() extends Module {
  val io = new ICacheIO()
  // Generate submodules of instruction cache
  val ctrl = Module(new PICache2LRUCtrl())
  val repl = Module(new PICache2LRURepl())
  val mem = Module(new PICache2LRUMem())
  val pref = Module(new PFSMLRU())
  // Connect control unit
  ctrl.io.ctrlrepl <> repl.io.ctrlrepl
  ctrl.io.ocp_port <> io.ocp_port
  ctrl.io.perf <> io.perf
  ctrl.io.ctrlpref <> pref.io.ctrlpref
  // Connect replacement unit
  repl.io.exicache <> io.exicache
  repl.io.feicache <> io.feicache
  repl.io.icachefe <> io.icachefe
  repl.io.replctrl <> ctrl.io.replctrl
  repl.io.prefrepl <> pref.io.prefrepl
  // Connect prefetch unit
  pref.io.feicache <> io.feicache
  // Connect replacement unit to on-chip memory
  repl.io.memIn <> mem.io.memIn
  repl.io.memOut <> mem.io.memOut
  // Connect enable signal
  ctrl.io.ena_in <> io.ena_in
  repl.io.ena_in <> io.ena_in
  // Output enable depending on hit/miss/fetch
  io.ena_out := ctrl.io.fetch_ena
  // Connect invalidate signal
  repl.io.invalidate := io.invalidate
  pref.io.invalidate := io.invalidate
}

/*
 ICacheMem Class: On-Chip Instruction Cache Memory
 */
class PICache2LRUMem extends Module {
  val io = new PICache2LRUMemIO()

  val icacheEvenFirst = MemBlock(ICACHE_WORD_SIZE / 4, INSTR_WIDTH)
  val icacheOddFirst = MemBlock(ICACHE_WORD_SIZE / 4, INSTR_WIDTH)

  val icacheEvenSecond = MemBlock(ICACHE_WORD_SIZE / 4, INSTR_WIDTH)
  val icacheOddSecond = MemBlock(ICACHE_WORD_SIZE / 4, INSTR_WIDTH)

  icacheEvenFirst.io <= (io.memIn.wEvenFirst, io.memIn.wAddr, io.memIn.wData)
  icacheOddFirst.io <= (io.memIn.wOddFirst, io.memIn.wAddr, io.memIn.wData)
  icacheEvenSecond.io <= (io.memIn.wEvenSecond, io.memIn.wAddr, io.memIn.wData)
  icacheOddSecond.io <= (io.memIn.wOddSecond, io.memIn.wAddr, io.memIn.wData)


  io.memOut.instrEvenFirst := icacheEvenFirst.io(io.memIn.addrEven)
  io.memOut.instrOddFirst := icacheOddFirst.io(io.memIn.addrOdd)
  io.memOut.instrEvenSecond := icacheEvenSecond.io(io.memIn.addrEven)
  io.memOut.instrOddSecond := icacheOddSecond.io(io.memIn.addrOdd)
}

/*
 2-Way Replacement Class
 */
class PICache2LRURepl() extends Module {
  val io = new PICache2LRUReplIO()

  // Tag memory and vector for valid bits
  val tagMemEvenFirst = MemBlock(LINE_COUNT / 2, TAG_SIZE)
  val tagMemOddFirst = MemBlock(LINE_COUNT / 2, TAG_SIZE)
  val validVecFirst = Vec.fill(LINE_COUNT) { Reg(init = Bool(false)) }
  val tagMemEvenSecond = MemBlock(LINE_COUNT / 2, TAG_SIZE)
  val tagMemOddSecond = MemBlock(LINE_COUNT / 2, TAG_SIZE)
  val validVecSecond = Vec.fill(LINE_COUNT) { Reg(init = Bool(false)) }
  val replVec = Vec.fill(LINE_COUNT) { Reg(init = Bool(false)) }

  val tagMemEvenPrefFirst = MemBlock(LINE_COUNT / 2, TAG_SIZE)
  val tagMemOddPrefFirst = MemBlock(LINE_COUNT / 2, TAG_SIZE)
  val tagMemEvenPrefSecond = MemBlock(LINE_COUNT / 2, TAG_SIZE)
  val tagMemOddPrefSecond = MemBlock(LINE_COUNT / 2, TAG_SIZE)
  val validVecPrefFirst = Vec.fill(LINE_COUNT) { Reg(init = Bool(false)) }
  val validVecPrefSecond = Vec.fill(LINE_COUNT) { Reg(init = Bool(false)) }
  

  // Variables for call/return
  val callRetBaseReg = Reg(init = UInt(1, DATA_WIDTH))
  val callAddrReg = Reg(init = UInt(1, DATA_WIDTH))
  val selSpmReg = Reg(init = Bool(false))
  val selCacheReg = Reg(init = Bool(false))

  val fetchAddr = Bits(width = EXTMEM_ADDR_WIDTH)
  val hitEven = Bool()
  val hitOdd = Bool()
  val hitPref = Bool()

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

  // Register addresses
  val addrEvenReg = Reg(next = io.feicache.addrEven)
  val addrOddReg = Reg(next = io.feicache.addrOdd)
  val addrPrefReg = Reg(next = io.prefrepl.prefAddr)
  
  // Addresses for tag memory
  val indexEven = io.feicache.addrEven(INDEX_HIGH, INDEX_LOW+1)
  val indexOdd = io.feicache.addrOdd(INDEX_HIGH, INDEX_LOW+1)
  val indexPref = io.prefrepl.prefAddr(INDEX_HIGH, INDEX_LOW+1) 
  val parityEven = io.feicache.addrEven(INDEX_LOW)
  val parityPref = io.prefrepl.prefAddr(INDEX_LOW)  
  val tagAddrEven = Mux(parityEven, indexOdd, indexEven)
  val tagAddrOdd = Mux(parityEven, indexEven, indexOdd)
  val tagAddrPref = indexPref

  // Read from tag memory
  val toutEvenFirst = tagMemEvenFirst.io(tagAddrEven)
  val toutOddFirst = tagMemOddFirst.io(tagAddrOdd)
  val toutEvenSecond = tagMemEvenSecond.io(tagAddrEven)
  val toutOddSecond = tagMemOddSecond.io(tagAddrOdd)

  val toutEvenPrefFirst = tagMemEvenPrefFirst.io(tagAddrPref)
  val toutOddPrefFirst = tagMemOddPrefFirst.io(tagAddrPref)
  val toutEvenPrefSecond = tagMemEvenPrefSecond.io(tagAddrPref)
  val toutOddPrefSecond = tagMemOddPrefSecond.io(tagAddrPref)
 
  // Multiplex tag memory output
  val tagEvenFirst = Mux(addrEvenReg(INDEX_LOW), toutOddFirst, toutEvenFirst)
  val tagOddFirst = Mux(addrOddReg(INDEX_LOW), toutOddFirst, toutEvenFirst)
  val tagEvenSecond = Mux(addrEvenReg(INDEX_LOW), toutOddSecond, toutEvenSecond)
  val tagOddSecond = Mux(addrOddReg(INDEX_LOW), toutOddSecond, toutEvenSecond)
  val tagPrefFirst = Mux(addrPrefReg(INDEX_LOW), toutOddPrefFirst, toutEvenPrefFirst)
  val tagPrefSecond = Mux(addrPrefReg(INDEX_LOW), toutOddPrefSecond, toutEvenPrefSecond)
 

  // Check if line is valid
  val validEvenFirst = validVecFirst(addrEvenReg(INDEX_HIGH, INDEX_LOW))
  val validOddFirst = validVecFirst(addrOddReg(INDEX_HIGH, INDEX_LOW))
  val validEvenSecond = validVecSecond(addrEvenReg(INDEX_HIGH, INDEX_LOW))
  val validOddSecond = validVecSecond(addrOddReg(INDEX_HIGH, INDEX_LOW))
  val validPrefFirst = validVecPrefFirst(addrPrefReg(INDEX_HIGH, INDEX_LOW))
  val validPrefSecond = validVecPrefSecond(addrPrefReg(INDEX_HIGH, INDEX_LOW))
  

  // Check for a hit of both instructions in the address bundle
  hitEven := Bool(true)
  hitOdd := Bool(true)
  hitPref := Bool(true)

  fetchAddr := addrEvenReg

  when (((tagEvenFirst != addrEvenReg(TAG_HIGH, TAG_LOW)) || (!validEvenFirst)) && ((tagEvenSecond != addrEvenReg(TAG_HIGH, TAG_LOW)) || (!validEvenSecond))) {
    hitEven := Bool(false)
  }
  .elsewhen (((tagOddFirst != addrOddReg(TAG_HIGH, TAG_LOW)) || (!validOddFirst)) && ((tagOddSecond != addrOddReg(TAG_HIGH, TAG_LOW)) || (!validOddSecond))) {
    hitOdd := Bool(false)
    fetchAddr := addrOddReg
  }

 .elsewhen  (((tagPrefFirst != addrPrefReg(TAG_HIGH, TAG_LOW)) || (!validPrefFirst)) && ((tagPrefSecond != addrPrefReg(TAG_HIGH, TAG_LOW)) || (!validPrefSecond))) {
    hitPref := Bool(false)
    fetchAddr := addrPrefReg
  }

// LRU replacement 
  val LRUIndexEven = addrEvenReg(INDEX_HIGH, INDEX_LOW)
  
  when (((tagEvenFirst === addrEvenReg(TAG_HIGH, TAG_LOW)) && (validEvenFirst)) || ((tagEvenSecond === addrEvenReg(TAG_HIGH, TAG_LOW)) && (validEvenSecond))) {
    replVec(LRUIndexEven) := Mux((tagEvenFirst === addrEvenReg(TAG_HIGH, TAG_LOW)), Bool(true), Bool(false))
  }


  // Keep signals alive for emulator
  debug(hitEven)
  debug(hitOdd)

  val wrAddrTag = io.ctrlrepl.wAddr(TAG_HIGH,TAG_LOW)
  // Index for vector of valid bits
  val wrValidIndex = io.ctrlrepl.wAddr(INDEX_HIGH, INDEX_LOW)
  // Index for tag memory even/odd
  val wrAddrIndex = io.ctrlrepl.wAddr(INDEX_HIGH, INDEX_LOW+1)
  val wrAddrParity = io.ctrlrepl.wAddr(INDEX_LOW)
  // Update tag field when new write occurs
  tagMemEvenFirst.io <= (io.ctrlrepl.wTag && (!wrAddrParity) && (!replVec(wrValidIndex)), wrAddrIndex, wrAddrTag)
  tagMemOddFirst.io <= (io.ctrlrepl.wTag && wrAddrParity && (!replVec(wrValidIndex)), wrAddrIndex, wrAddrTag)
  tagMemEvenPrefFirst.io <= (io.ctrlrepl.wTag && (!wrAddrParity) && (!replVec(wrValidIndex)), wrAddrIndex, wrAddrTag)
  tagMemOddPrefFirst.io <= (io.ctrlrepl.wTag && wrAddrParity && (!replVec(wrValidIndex)), wrAddrIndex, wrAddrTag) 
  when (io.ctrlrepl.wTag && (!replVec(wrValidIndex))) {
    validVecFirst(wrValidIndex) := Bool(true)
    validVecPrefFirst(wrValidIndex) := Bool(true)
//    replVec(wrValidIndex) := Bool(true)
  }

  tagMemEvenSecond.io <= (io.ctrlrepl.wTag && (!wrAddrParity) && (replVec(wrValidIndex)), wrAddrIndex, wrAddrTag)
  tagMemOddSecond.io <= (io.ctrlrepl.wTag && (wrAddrParity) && (replVec(wrValidIndex)), wrAddrIndex, wrAddrTag)
  tagMemEvenPrefSecond.io <= (io.ctrlrepl.wTag && (!wrAddrParity) && (replVec(wrValidIndex)), wrAddrIndex, wrAddrTag)
  tagMemOddPrefSecond.io <= (io.ctrlrepl.wTag && (wrAddrParity) && (replVec(wrValidIndex)), wrAddrIndex, wrAddrTag) 
  when (io.ctrlrepl.wTag && (replVec(wrValidIndex))) {
    validVecSecond(wrValidIndex) := Bool(true)
    validVecPrefSecond(wrValidIndex) := Bool(true)
//    replVec(wrValidIndex) := Bool(false)
  }

  val wrParity = io.ctrlrepl.wAddr(0)

  // Outputs to cache memory
  io.memIn.wEvenFirst := Mux((!wrParity) && (!replVec(wrValidIndex)), io.ctrlrepl.wEna, Bool(false))
  io.memIn.wOddFirst := Mux(wrParity && (!replVec(wrValidIndex)), io.ctrlrepl.wEna, Bool(false))
  io.memIn.wEvenSecond := Mux((!wrParity) && (replVec(wrValidIndex)), io.ctrlrepl.wEna, Bool(false))
  io.memIn.wOddSecond := Mux(wrParity && (replVec(wrValidIndex)), io.ctrlrepl.wEna, Bool(false)) 
  io.memIn.wData := io.ctrlrepl.wData
  io.memIn.wAddr := io.ctrlrepl.wAddr(INDEX_HIGH,1)
  io.memIn.addrOdd := io.feicache.addrOdd(INDEX_HIGH,1)
  io.memIn.addrEven := io.feicache.addrEven(INDEX_HIGH,1)

  // Outputs to fetch stage
  io.icachefe.instrEven := Mux((tagEvenFirst === addrEvenReg(TAG_HIGH, TAG_LOW)), io.memOut.instrEvenFirst, io.memOut.instrEvenSecond)
  io.icachefe.instrOdd := Mux((tagOddFirst === addrOddReg(TAG_HIGH, TAG_LOW)), io.memOut.instrOddFirst, io.memOut.instrOddSecond)
  io.icachefe.relBase := relBase
  io.icachefe.relPc := relPc
  io.icachefe.reloc := reloc
  io.icachefe.memSel := Cat(selSpmReg, selCacheReg)

  // Hit/miss to control module
  io.replctrl.fetchAddr := fetchAddr
  io.replctrl.hitPref := hitPref
  io.replctrl.hit := hitEven && hitOdd 
  io.replctrl.selCache := selCacheReg

  when (io.invalidate) {
    validVecFirst.map(_ := Bool(false))
    validVecSecond.map(_ := Bool(false))
    validVecPrefFirst.map(_ := Bool(false))
    validVecPrefSecond.map(_ := Bool(false))
    replVec.map(_ := Bool(false))
  }
}

/*
 Instruction Cache Control Class: handles block transfer from external Memory to the I-Cache
 */
class PICache2LRUCtrl() extends Module {
  val io = new PICache2LRUCtrlIO()

  // States of the state machine
  val initState :: idleState :: transferState :: waitState :: Nil = Enum(UInt(), 4)
  val stateReg = Reg(init = initState)
  
  val fetch = (!io.replctrl.hit) || (!io.replctrl.hitPref)
  val prefTrig = Bool()
  // Signal for replacement unit
  val wData = Bits(width = DATA_WIDTH)
  val wTag = Bool()
  val wAddr = Bits(width = ADDR_WIDTH)
  val wEna = Bool()
  // Signals for external memory
  val ocpCmd = Bits(width = 3)
  val ocpAddr = Bits(width = EXTMEM_ADDR_WIDTH)
  val fetchCnt = Reg(init = Bits(0, width = ICACHE_SIZE_WIDTH))
  val burstCnt = Reg(init = UInt(0, width = log2Up(BURST_LENGTH)))
  val fetchEna = Bool()
  // Input/output registers
  val addrReg = Reg(init = Bits(0, width = EXTMEM_ADDR_WIDTH - LINE_WORD_SIZE_WIDTH))
  val ocpSlaveReg = Reg(next = io.ocp_port.S)

  // Initialize signals
  wData := Bits(0)
  wTag := Bool(false)
  wEna := Bool(false)
  wAddr := Bits(0)
  ocpCmd := OcpCmd.IDLE
  ocpAddr := Bits(0)
  fetchEna := Bool(true)
  prefTrig := Bool(false)

  // Wait till ICache is the selected source of instructions
  when (stateReg === initState) {
    when (io.replctrl.selCache) {
      stateReg := idleState
    }
  }
  when (stateReg === idleState) {
    when (!io.replctrl.selCache) {
      stateReg := initState
    } .otherwise {
      when (fetch) {
	when (!io.replctrl.hit) {
          fetchEna := Bool(false)
	}
        val addr = io.replctrl.fetchAddr(EXTMEM_ADDR_WIDTH-1, LINE_WORD_SIZE_WIDTH)
        addrReg := addr
        burstCnt := UInt(0)
        fetchCnt := UInt(0)
	wAddr := Cat(addr, Bits(0, width = LINE_WORD_SIZE_WIDTH))
       // Check if command is accepted by the memory controller
        ocpAddr := Cat(addr, Bits(0, width =  LINE_WORD_SIZE_WIDTH))
        ocpCmd := OcpCmd.RD
        when (io.ocp_port.S.CmdAccept === Bits(1)) {
          stateReg := transferState
        } .otherwise {
          stateReg := waitState
        }
      }
    }
  }
  when (stateReg === waitState) {
    when (!io.replctrl.hit) {
          fetchEna := Bool(false)
    }
    ocpAddr := Cat(addrReg, Bits(0, width = LINE_WORD_SIZE_WIDTH))
    ocpCmd := OcpCmd.RD
    when (io.ocp_port.S.CmdAccept === Bits(1)) {
      stateReg := transferState
    }
  }
  // Transfer/fetch cache block
  when (stateReg === transferState) {
     when (!io.replctrl.hit) {
          fetchEna := Bool(false)
    }
    when (fetchCnt < UInt(LINE_WORD_SIZE)) {
      when (fetchCnt === UInt(LINE_WORD_SIZE - 1)) {
	wTag := Bool(true)
      }
      when (ocpSlaveReg.Resp === OcpResp.DVA) {
        fetchCnt := fetchCnt + Bits(1)
        burstCnt := burstCnt + Bits(1)
        when(fetchCnt < UInt(LINE_WORD_SIZE-1)) {
          // Fetch next address from external memory
          when (burstCnt >= UInt(BURST_LENGTH - 1)) {
            ocpCmd := OcpCmd.RD
            ocpAddr := Cat(addrReg, Bits(0, width = LINE_WORD_SIZE_WIDTH)) + fetchCnt + Bits(1)
            burstCnt := UInt(0)
          }
        }
        // Write current address to icache memory
        wData := ocpSlaveReg.Data
        wEna := Bool(true)
      }
      wAddr := Cat(addrReg, Bits(0, width = LINE_WORD_SIZE_WIDTH)) + fetchCnt
    }
    // Restart to idle state
    .otherwise {
      prefTrig := Bool(true)
      stateReg := idleState
    }
  }

  // Outputs to cache memory
  io.ctrlrepl.wEna := wEna
  io.ctrlrepl.wData := wData
  io.ctrlrepl.wAddr := wAddr
  io.ctrlrepl.wTag := wTag

  io.fetch_ena := fetchEna
  io.ctrlpref.prefTrig := prefTrig


  // Outputs to external memory
  io.ocp_port.M.Addr := Cat(ocpAddr, Bits("b00"))
  io.ocp_port.M.Cmd := ocpCmd
  io.ocp_port.M.Data := Bits(0) //read-only
  io.ocp_port.M.DataByteEn := Bits("b1111") //read-only
  io.ocp_port.M.DataValid := Bits(0) //read-only

  // Output to performance counters
  io.perf.hit := Bool(false)
  io.perf.miss := Bool(false)
  when (io.ena_in && io.replctrl.selCache && stateReg === idleState) {
    when (io.replctrl.hit) {
      io.perf.hit := Bool(true)
    } .otherwise {
      io.perf.miss := Bool(true)
    }
  }
}
