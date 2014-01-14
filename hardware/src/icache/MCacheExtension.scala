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
 Different Replacement Strategies for Method Cache as a possible extension instead of MCacheRepl class
 Author: Philipp Degasperi (philipp.degasperi@gmail.com)
 */

package patmos

import Chisel._
import Node._
import MConstants._
import Constants._
import ocp._

import scala.collection.mutable.HashMap
import scala.util.Random
import scala.math

/*
 Fifo Replacement with blocks of a fixed size.
 Similar as it was orginially proposed and used in JOP
 */
class MCacheReplFifo2() extends Module {
  val io = new MCacheReplIO()

  val mcacheAddrVec = { Vec.fill(METHOD_COUNT) { Reg(init = Bits(0, width = ADDR_WIDTH)) }}
  val mcacheValidVec = { Vec.fill(METHOD_COUNT) { Reg(init = Bool(false)) }}
  val nextIndexReg = Reg(init = Bits(0, width = log2Up(METHOD_COUNT)))
  val nextReplReg = Reg(init = Bits(0, width = log2Up(METHOD_COUNT)))
  val posReg = Reg(init = Bits(0, width = MCACHE_SIZE_WIDTH))
  val hitReg = Reg(init = Bool(true))
  val wrPosReg = Reg(init = Bits(1, width = MCACHE_SIZE_WIDTH)) //could may dropped
  val callRetBaseReg = Reg(init = UInt(1, DATA_WIDTH))
  val callAddrReg = Reg(init = UInt(1, DATA_WIDTH))
  val selIspmReg = Reg(init = Bool(false))
  val selMCacheReg = Reg(init = Bool(false))

  when (io.exmcache.doCallRet && io.ena_in) {
    callRetBaseReg := io.exmcache.callRetBase
    callAddrReg := io.exmcache.callRetAddr
    selIspmReg := io.exmcache.callRetBase(EXTMEM_ADDR_WIDTH - 1,ISPM_ONE_BIT - 2) === Bits(0x1)
    selMCacheReg := io.exmcache.callRetBase(EXTMEM_ADDR_WIDTH - 1,15) >= Bits(0x1)
    when (io.exmcache.callRetBase(EXTMEM_ADDR_WIDTH-1,15) >= Bits(0x1)) {
      hitReg := Bool(false)
      posReg := (nextIndexReg << Bits(log2Up(METHOD_BLOCK_SIZE)))
      for (i <- 0 until METHOD_COUNT) {
        when (io.exmcache.callRetBase === mcacheAddrVec(i) && mcacheValidVec(i)) {
          hitReg := Bool(true)
          posReg := Bits(i << log2Up(METHOD_BLOCK_SIZE))
        }
      }
    }
  }

  //should do this only on call/return!
  val relBase = Mux(selMCacheReg,
                    posReg.toUInt,
                    callRetBaseReg(ISPM_ONE_BIT-3, 0))
  val relPc = callAddrReg + relBase

  val reloc = Mux(selMCacheReg,
                  callRetBaseReg - posReg.toUInt,
                  UInt(1 << (ISPM_ONE_BIT - 2)))

  //insert new tags
  when (io.mcache_ctrlrepl.wTag) {
    hitReg := Bool(true)
    wrPosReg := posReg //could use only posReg
    mcacheAddrVec(nextIndexReg) := io.mcache_ctrlrepl.wAddr
    mcacheValidVec(nextIndexReg) := Bool(true)
    nextIndexReg := (nextIndexReg + io.mcache_ctrlrepl.wData(31,log2Up(METHOD_BLOCK_SIZE)) + Bits(1)) % Bits(METHOD_COUNT)
    nextReplReg := (nextReplReg + Bits(1)) % Bits(METHOD_COUNT)
  }

  //invalidate next methods
  when (nextReplReg != nextIndexReg) {
    nextReplReg := (nextReplReg + Bits(1)) % Bits(METHOD_COUNT)
    mcacheValidVec(nextReplReg) := Bool(false)
  }

  val wParity = io.mcache_ctrlrepl.wAddr(0)
  val wAddr = (wrPosReg + io.mcache_ctrlrepl.wAddr)(MCACHE_SIZE_WIDTH-1,1)
  val addrEven = (io.mcache_ctrlrepl.addrEven)(MCACHE_SIZE_WIDTH-1,1)
  val addrOdd = (io.mcache_ctrlrepl.addrOdd)(MCACHE_SIZE_WIDTH-1,1)

  io.mcachemem_in.wEven := Mux(wParity, Bool(false), io.mcache_ctrlrepl.wEna)
  io.mcachemem_in.wOdd := Mux(wParity, io.mcache_ctrlrepl.wEna, Bool(false))
  io.mcachemem_in.wData := io.mcache_ctrlrepl.wData
  io.mcachemem_in.wAddr := wAddr
  io.mcachemem_in.addrEven := addrEven
  io.mcachemem_in.addrOdd := addrOdd

  val instrEvenReg = Reg(init = Bits(0, width = INSTR_WIDTH))
  val instrOddReg = Reg(init = Bits(0, width = INSTR_WIDTH))
  val instrEven = io.mcachemem_out.instrEven
  val instrOdd = io.mcachemem_out.instrOdd
  //save instr. ouput since method block at the given address could be overwritten during fetch
  when (!io.mcache_ctrlrepl.instrStall) {
    instrEvenReg := io.mcachefe.instrEven
    instrOddReg := io.mcachefe.instrOdd
  }
  io.mcachefe.instrEven := Mux(io.mcache_ctrlrepl.instrStall, instrEvenReg, instrEven)
  io.mcachefe.instrOdd := Mux(io.mcache_ctrlrepl.instrStall, instrOddReg, instrOdd)

  io.mcachefe.relBase := relBase
  io.mcachefe.relPc := relPc
  io.mcachefe.reloc := reloc
  io.mcachefe.memSel := Cat(selIspmReg, selMCacheReg)

  io.mcache_replctrl.hit := hitReg

  io.hitEna := hitReg
}

/*
 MCacheReplLru: LRU replacement strategy for the method cache
 */
class MCacheReplLru() extends Module {
  val io = new MCacheReplIO()

  //tag field and address translation table
  val mcacheAddrVec = { Vec.fill(METHOD_COUNT) { Reg(init = Bits(0, width = ADDR_WIDTH)) } }

  val mcacheMmuVec = { Vec.fill(METHOD_COUNT*METHOD_COUNT) { Reg(init = Bits(0, width = log2Up(METHOD_COUNT))) } }
  val mcacheMmuSize = { Vec.fill(METHOD_COUNT) { Reg(init = Bits(0, width = log2Up(METHOD_COUNT))) } }
  //linked list for lru replacement
  val initPrevReg = Array(Reg(init = Bits(1)), Reg(init = Bits(2)), Reg(init = Bits(3)), Reg(init = Bits(0)))
  val initNextReg = Array(Reg(init = Bits(3)), Reg(init = Bits(0)), Reg(init = Bits(1)), Reg(init = Bits(2)))
  val lruListPrev = Vec(initPrevReg)
  val lruListNext = Vec(initNextReg)
  val lruTagReg = Reg(init = Bits(0, width = log2Up(METHOD_COUNT)))
  val mruTagReg = Reg(init = Bits(METHOD_COUNT - 1, width = log2Up(METHOD_COUNT)))
  //val lru_pos = Reg(init = Bits(0, width = log2Up(method_count)))
  //registers for splitting up
  val splitCntReg = Reg(init = Bits(0, width = MCACHE_SIZE_WIDTH))
  val offsetCntReg = Reg(init = Bits(0, width = log2Up(METHOD_COUNT)))
  val updateCntReg = Reg(init = Bits(0, width = log2Up(METHOD_COUNT)))
  //variables when call/return occurs to check and set tag fields
  val posReg = Reg(init = Bits(0, width = MCACHE_SIZE_WIDTH))
  val hitReg = Reg(init = Bool(true))
  val wrPosReg = Reg(init = Bits(0, width = MCACHE_SIZE_WIDTH))
  val currPosReg = Reg(init = Bits(0, width = METHOD_COUNT))
  val callRetBaseReg = Reg(init = UInt(1, DATA_WIDTH))
  val callAddrReg = Reg(init = UInt(1, DATA_WIDTH))
  val selIspmReg = Reg(init = Bool(false))
  val selMCacheReg = Reg(init = Bool(false))

  def updateTag(tag : UInt) = {
    when (tag === lruTagReg) {
      lruTagReg := lruListPrev(tag)
      mruTagReg := tag
      lruListNext(tag) := mruTagReg
      lruListPrev(mruTagReg) := tag
      lruListPrev(tag) := tag //no previous any more because mru
    }
    .elsewhen (tag != mruTagReg) {
      lruListNext(lruListPrev(tag)) := lruListNext(tag)
      lruListPrev(lruListNext(tag)) := lruListPrev(tag)
      lruListNext(tag) := mruTagReg
      lruListPrev(mruTagReg) := tag
      mruTagReg := tag
    }
  }

  when (io.exmcache.doCallRet && io.ena_in) {

    callRetBaseReg := io.exmcache.callRetBase
    callAddrReg := io.exmcache.callRetAddr
    selIspmReg := io.exmcache.callRetBase(DATA_WIDTH - 1,ISPM_ONE_BIT - 2) === Bits(0x1)
    selMCacheReg := io.exmcache.callRetBase(DATA_WIDTH - 1,15) >= Bits(0x1)

    when (io.exmcache.callRetBase(DATA_WIDTH - 1,15) >= Bits(0x1)) {
      hitReg := Bool(false)
      posReg := (lruTagReg << Bits(log2Up(METHOD_BLOCK_SIZE)))
      for (i <- 0 until METHOD_COUNT) {
        when (io.exmcache.callRetBase === mcacheAddrVec(i)) {
          hitReg := Bool(true)
          currPosReg := Bits(i << log2Up(METHOD_COUNT)) //pos in mmu
          posReg := Bits(i << log2Up(METHOD_BLOCK_SIZE)) //pos in cache
        }
      }
    }
  }

  val relBase = Mux(selMCacheReg,
    posReg.toUInt,
    callRetBaseReg(ISPM_ONE_BIT-3, 0))
  val relPc = callAddrReg + relBase

  val reloc = Mux(selMCacheReg,
                  callRetBaseReg - posReg.toUInt,
                  UInt(1 << (ISPM_ONE_BIT - 2)))

  //sequentially update of all connected blocks (maybe stall here, what happens when there is always a call/hit?!)
  val doCallRetReg = Reg(next = io.exmcache.doCallRet)
  when (doCallRetReg && hitReg) {
    updateTag((currPosReg/Bits(4))(log2Up(METHOD_COUNT)-1,0))
    updateCntReg := mcacheMmuSize(currPosReg/Bits(4))
  }
  when (updateCntReg > Bits(0)) {
    updateCntReg := updateCntReg - Bits(1)
    updateTag(mcacheMmuVec(currPosReg + updateCntReg))
  }

  // val address_in_pos = io.mcache_ctrlrepl.address(METHOD_BLOCK_SIZE_WIDTH*2+log2Up(METHOD_COUNT)-1,METHOD_BLOCK_SIZE_WIDTH)
  // val address_in_offset = io.mcache_ctrlrepl.address(METHOD_BLOCK_SIZE_WIDTH-1,0)
  // val w_address_pos = io.mcache_ctrlrepl.w_addr(METHOD_BLOCK_SIZE_WIDTH+log2Up(METHOD_COUNT),METHOD_BLOCK_SIZE_WIDTH)
  // val w_address_offset = io.mcache_ctrlrepl.w_addr(METHOD_BLOCK_SIZE_WIDTH-1,0)

  //val rdPos = Cat(mcache_mmu_vec(address_in_pos), address_in_offset)

  //val wrPos = Cat(mcache_mmu_vec(w_address_pos + currPosReg), w_address_offset)

  //insert new tags
  when (io.mcache_ctrlrepl.wTag) {
    hitReg := Bool(true)
    wrPosReg := posReg
    //start splitting into more blocks if current method size > method block size
    splitCntReg := io.mcache_ctrlrepl.wData(METHOD_BLOCK_SIZE_WIDTH+log2Up(METHOD_COUNT), METHOD_BLOCK_SIZE_WIDTH)
    offsetCntReg := Bits(1)
    currPosReg := (lruTagReg << Bits(log2Up(METHOD_COUNT)))
    mcacheAddrVec(lruTagReg) := io.mcache_ctrlrepl.wAddr
    mcacheMmuVec(lruTagReg * Bits(METHOD_COUNT)) := lruTagReg
    mcacheMmuSize(lruTagReg) := io.mcache_ctrlrepl.wData(METHOD_BLOCK_SIZE_WIDTH+log2Up(METHOD_COUNT)-1, METHOD_BLOCK_SIZE_WIDTH)
    updateTag(lruTagReg)
  }

  when (splitCntReg > Bits(0)) {
    splitCntReg := splitCntReg - Bits(1)
    offsetCntReg := offsetCntReg + Bits(1)
    mcacheAddrVec(lruTagReg) := Bits(0) //invalidate field
    mcacheMmuVec(currPosReg + offsetCntReg) := lruTagReg
    updateTag(lruTagReg)
  }

  val wParity = io.mcache_ctrlrepl.wAddr(0)
  val wAddr = (wrPosReg + io.mcache_ctrlrepl.wAddr)(MCACHE_SIZE_WIDTH-1,1)
  val addrEven = (io.mcache_ctrlrepl.addrEven)(MCACHE_SIZE_WIDTH-1,1)
  val addrOdd = (io.mcache_ctrlrepl.addrOdd)(MCACHE_SIZE_WIDTH-1,1)

  //read/write to mcachemem
  io.mcachemem_in.wEven := Mux(wParity, Bool(false), io.mcache_ctrlrepl.wEna)
  io.mcachemem_in.wOdd := Mux(wParity, io.mcache_ctrlrepl.wEna, Bool(false))
  io.mcachemem_in.wData := io.mcache_ctrlrepl.wData
  io.mcachemem_in.wAddr := wAddr
  io.mcachemem_in.addrEven := addrEven
  io.mcachemem_in.addrOdd := addrOdd

  val instrEvenReg = Reg(init = Bits(0, width = INSTR_WIDTH))
  val instrOddReg = Reg(init = Bits(0, width = INSTR_WIDTH))
  val instrEven = io.mcachemem_out.instrEven
  val instrOdd = io.mcachemem_out.instrOdd
  when (io.mcache_ctrlrepl.instrStall === Bits(0)) {
    instrEvenReg := io.mcachefe.instrEven
    instrOddReg := io.mcachefe.instrOdd
  }
  //signals to fetch stage
  io.mcachefe.instrEven := Mux(io.mcache_ctrlrepl.instrStall, instrEvenReg, instrEven)
  io.mcachefe.instrOdd := Mux(io.mcache_ctrlrepl.instrStall, instrOddReg, instrOdd)
  io.mcachefe.relBase := relBase
  io.mcachefe.relPc := relPc
  io.mcachefe.reloc := reloc
  io.mcachefe.memSel := Cat(selIspmReg, selMCacheReg)
  //signals to ctrl unit
  io.mcache_replctrl.hit := hitReg
  //hit/stall signal
  io.hitEna := hitReg

}

