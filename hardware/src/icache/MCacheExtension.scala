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
 Different Replacement Strategies for Method Cache as a possible extension instead of
 MCacheRepl class
 Author: Philipp Degasperi (philipp.degasperi@gmail.com)
 */

package patmos

import Chisel._
import Node._
import MConstants._
import Constants._
import ocp._
import MCacheReplLru._

import scala.collection.mutable.HashMap
import scala.util.Random
import scala.math

/*
 FIFO Replacement with blocks of a fixed size.  Similar as it was orginially proposed and
 used in JOP This FIFO replacement uses fixed-block replacement and allows functions to
 span over several blocks
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

  // reset valid bits
  when (io.invalidate) {
    mcacheValidVec.map(_ := Bool(false))
  }
}

/*
  Tag field bundle for LRU replacement contains:
  address tag, valid tag and position tag
*/
//PD: maybe there is a nicer way to implement such a tag field to implement it into a vector
class TagField() extends Module {
  val io = new Bundle {
    val addrIn = Bits(INPUT, width = ADDR_WIDTH)
    val addrOut = Bits(OUTPUT, width = ADDR_WIDTH)
    val validIn = Bool(INPUT)
    val validOut = Bool(OUTPUT)
    val posIn = Bits(INPUT, width = log2Up(METHOD_COUNT))
    val posOut = Bits(OUTPUT, width = log2Up(METHOD_COUNT))
    val shiftEna = Bool(INPUT)
  }
  i+=1
  val addr = Reg(init = Bits(0, width = ADDR_WIDTH))
  val valid = Reg(init = Bool(false))
  val blockPos = Reg(init = Bits(METHOD_COUNT-i, width = log2Up(METHOD_COUNT)))
  when (io.shiftEna) {
    addr := io.addrIn
    blockPos := io.posIn
    valid := io.validIn
  }
  io.addrOut := addr
  io.posOut := blockPos
  io.validOut := valid
}

object MCacheReplLru {
  //variable for the initialization of lru tag field
  var i = 0
}

/*
  MCacheReplLru: LRU replacement strategy for the method cache This class implements a LRU
  replacement and restrict functions to have at most the size of a cache block and does
  not allow to span over several blocks
  Use Function Splitter to cut down the maximal fucntion size to cache block size
  LRU replacement is implemented with a shift registers
 */
class MCacheReplLru() extends Module {
  val io = new MCacheReplIO()

  //a vector with n tag fields
  val mcacheTagVec = { Vec.fill(METHOD_COUNT) { Module(new TagField()).io }}

  val posReg = Reg(init = Bits(0, width = MCACHE_SIZE_WIDTH))
  val currPosReg = Reg(init = Bits(0, width = log2Up(METHOD_COUNT)))
  val hitReg = Reg(init = Bool(true))
  val wrPosReg = Reg(init = Bits(0, width = MCACHE_SIZE_WIDTH))
  val callRetBaseReg = Reg(init = UInt(1, DATA_WIDTH))
  val doCallRetReg = Reg(init = Bool(false))
  val callAddrReg = Reg(init = UInt(1, DATA_WIDTH))
  val selIspmReg = Reg(init = Bool(false))
  val selMCacheReg = Reg(init = Bool(false))

  when (io.exmcache.doCallRet && io.ena_in) {
    //catch new call address
    callRetBaseReg := io.exmcache.callRetBase
    callAddrReg := io.exmcache.callRetAddr
    doCallRetReg := io.exmcache.doCallRet
    selIspmReg := io.exmcache.callRetBase(EXTMEM_ADDR_WIDTH - 1,ISPM_ONE_BIT - 2) === Bits(0x1)
    selMCacheReg := io.exmcache.callRetBase(EXTMEM_ADDR_WIDTH - 1,15) >= Bits(0x1)
    //check for a hit
    when (io.exmcache.callRetBase(EXTMEM_ADDR_WIDTH - 1,15) >= Bits(0x1)) {
      hitReg := Bool(false)
      //position of LRU block in the cache
      posReg := (mcacheTagVec(METHOD_COUNT-1).posOut << Bits(log2Up(METHOD_BLOCK_SIZE)))
      for (i <- 0 until METHOD_COUNT) {
        when (io.exmcache.callRetBase === mcacheTagVec(i).addrOut && mcacheTagVec(i).validOut) {
          hitReg := Bool(true)
          currPosReg := Bits(i) //save position for shift
          posReg := mcacheTagVec(i).posOut << Bits(log2Up(METHOD_BLOCK_SIZE)) //save position for new fetch
        }
      }
    }
  }

  //calculate relative address signals
  val relBase = Mux(selMCacheReg,
    posReg.toUInt,
    callRetBaseReg(ISPM_ONE_BIT-3, 0))
  val relPc = callAddrReg + relBase

  val reloc = Mux(selMCacheReg,
                  callRetBaseReg - posReg.toUInt,
                  UInt(1 << (ISPM_ONE_BIT - 2)))

  //signals for a new tag field
  val lruAddr = Bits()
  val lruValid = Bool()
  val lruPos = Bits()
  val insertPos = Bits() //shift position
  lruValid := Bool(false)
  lruAddr := Bits(0)
  lruPos := Bits(0)
  insertPos := Bits(0)
  //update the LRU tag field after a hit
  when (doCallRetReg && hitReg) {
    insertPos := currPosReg + Bits(1)
    lruAddr :=  mcacheTagVec(currPosReg).addrOut
    lruValid := mcacheTagVec(currPosReg).validOut
    lruPos := mcacheTagVec(currPosReg).posOut
  }
  //insert new tags after a miss
  when (io.mcache_ctrlrepl.wTag) {
    hitReg := Bool(true) //we have a hit again
    wrPosReg := posReg
    lruAddr := io.mcache_ctrlrepl.wAddr
    lruValid := Bool(true)
    lruPos := mcacheTagVec(METHOD_COUNT-1).posOut
    insertPos := Bits(METHOD_COUNT) //update all registers
  }
  //shift LRU tag registers
  //PD: maybe there is a more efficient way to implement a shift register in Chisel
  //instead of this solution
  for (i <- 0 until METHOD_COUNT) {
    if (i != 0) {
      mcacheTagVec(i).addrIn := mcacheTagVec(i-1).addrOut
      mcacheTagVec(i).posIn := mcacheTagVec(i-1).posOut
      mcacheTagVec(i).validIn := mcacheTagVec(i-1).validOut
    }
    //input for first register (MRU)
    else {
      mcacheTagVec(i).addrIn := lruAddr
      mcacheTagVec(i).posIn := lruPos
      mcacheTagVec(i).validIn := lruValid
    }
    //shift enable for all registers till current position
    when (Bits(i) < insertPos) {
      mcacheTagVec(i).shiftEna := Bool(true)
    }
    .otherwise{
      mcacheTagVec(i).shiftEna := Bool(false)
    }
  }

  val wParity = io.mcache_ctrlrepl.wAddr(0)
  val wAddr = (wrPosReg + io.mcache_ctrlrepl.wAddr)(MCACHE_SIZE_WIDTH-1,1)
  val addrEven = (io.mcache_ctrlrepl.addrEven)(MCACHE_SIZE_WIDTH-1,1)
  val addrOdd = (io.mcache_ctrlrepl.addrOdd)(MCACHE_SIZE_WIDTH-1,1)
  //output to on-chip memory
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
  //save instrucitons in cache of a stall
  when (!io.mcache_ctrlrepl.instrStall) {
    instrEvenReg := io.mcachefe.instrEven
    instrOddReg := io.mcachefe.instrOdd
  }
  //output to fetch stage
  io.mcachefe.instrEven := Mux(io.mcache_ctrlrepl.instrStall, instrEvenReg, instrEven)
  io.mcachefe.instrOdd := Mux(io.mcache_ctrlrepl.instrStall, instrOddReg, instrOdd)
  io.mcachefe.relBase := relBase
  io.mcachefe.relPc := relPc
  io.mcachefe.reloc := reloc
  io.mcachefe.memSel := Cat(selIspmReg, selMCacheReg)
  //output to control module
  io.mcache_replctrl.hit := hitReg
  //output to mcache
  io.hitEna := hitReg

}


