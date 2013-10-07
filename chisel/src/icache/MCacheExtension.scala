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

class MCacheReplFifo2() extends Component {
  val io = new MCacheReplIO()

  val mcache_addr_vec = { Vec(METHOD_COUNT) { Reg(resetVal = Bits(0, width = ADDR_WIDTH)) } }
  val mcache_valid_vec = { Vec(METHOD_COUNT) { Reg(resetVal = Bits(0, width = 1)) } }
  val next_index_tag = Reg(resetVal = Bits(0, width = log2Up(METHOD_COUNT)))
  val next_replace_tag = Reg(resetVal = Bits(0, width = log2Up(METHOD_COUNT)))
  val posReg = Reg(resetVal = Bits(0, width = MCACHE_SIZE_WIDTH))
  val hitReg = Reg(resetVal = Bits(1, width = 1))
  val wrPosReg = Reg(resetVal = Bits(1, width = MCACHE_SIZE_WIDTH)) //could may dropped
  val callRetBaseReg = Reg(resetVal = UFix(1, DATA_WIDTH))
  val callAddrReg = Reg(resetVal = UFix(1, DATA_WIDTH))
  val selIspmReg = Reg(resetVal = Bits(0, width = 1))
  val selMCacheReg = Reg(resetVal = Bits(0, width = 1))

  when (io.exmcache.doCallRet && io.ena_in) {
    callRetBaseReg := io.exmcache.callRetBase
    callAddrReg := io.exmcache.callRetAddr
    selIspmReg := io.exmcache.callRetBase(DATA_WIDTH - 1,ISPM_ONE_BIT - 2) === Bits(0x1)
    selMCacheReg := io.exmcache.callRetBase(DATA_WIDTH - 1,15) >= Bits(0x1)
    when (io.exmcache.callRetBase(DATA_WIDTH-1,15) >= Bits(0x1)) {
      hitReg := Bits(0)
      posReg := (next_index_tag << Bits(log2Up(METHOD_BLOCK_SIZE)))
      for (i <- 0 until METHOD_COUNT) {
        when (io.exmcache.callRetBase === mcache_addr_vec(i) && mcache_valid_vec(i) === Bits(1)) {
          hitReg := Bits(1)
          posReg := Bits(i << log2Up(METHOD_BLOCK_SIZE)) //makes no sence to start writing at odd position
        }
      }
    }
  }

  //should do this only on call/return!
  val relBase = Mux(selMCacheReg,
                    posReg.toUFix,
                    callRetBaseReg(ISPM_ONE_BIT-3, 0))
  val relPc = callAddrReg + relBase

  val reloc = Mux(selMCacheReg,
                  callRetBaseReg - posReg.toUFix,
                  UFix(1 << (ISPM_ONE_BIT - 2)))

  //insert new tags
  when (io.mcache_ctrlrepl.w_tag) {
    hitReg := Bits(1)
    wrPosReg := posReg //could use only posReg
    mcache_addr_vec(next_index_tag) := io.mcache_ctrlrepl.w_addr
    mcache_valid_vec(next_index_tag) := Bits(1)
    next_index_tag := (next_index_tag + io.mcache_ctrlrepl.w_data(31,log2Up(METHOD_BLOCK_SIZE)) + Bits(1)) % Bits(METHOD_COUNT)
    next_replace_tag := (next_replace_tag + Bits(1)) % Bits(METHOD_COUNT)
  }

  //invalidate next methods
  when (next_replace_tag != next_index_tag) {
    next_replace_tag := (next_replace_tag + Bits(1)) % Bits(METHOD_COUNT)
    mcache_valid_vec(next_replace_tag) := Bits(0)
  }

  val wr_parity = io.mcache_ctrlrepl.w_addr(0)
  val mcachemem_w_address = (wrPosReg + io.mcache_ctrlrepl.w_addr)(MCACHE_SIZE_WIDTH-1,1)
  val rd_parity = io.mcache_ctrlrepl.address(0)
  val mcachemem_in_address = (io.mcache_ctrlrepl.address)(MCACHE_SIZE_WIDTH-1,1)
  val addr_parity_reg = Reg(rd_parity)

  io.mcachemem_in.w_even := Mux(wr_parity, Bits(0), io.mcache_ctrlrepl.w_enable)
  io.mcachemem_in.w_odd := Mux(wr_parity, io.mcache_ctrlrepl.w_enable, Bits(0))
  io.mcachemem_in.w_data := io.mcache_ctrlrepl.w_data
  io.mcachemem_in.w_addr := mcachemem_w_address
  io.mcachemem_in.addr_even := Mux(rd_parity, mcachemem_in_address + Bits(1), mcachemem_in_address)
  io.mcachemem_in.addr_odd := mcachemem_in_address

  val instr_aReg = Reg(resetVal = Bits(0, width = INSTR_WIDTH))
  val instr_bReg = Reg(resetVal = Bits(0, width = INSTR_WIDTH))
  val instr_a = Mux(addr_parity_reg, io.mcachemem_out.instr_odd, io.mcachemem_out.instr_even)
  val instr_b = Mux(addr_parity_reg, io.mcachemem_out.instr_even, io.mcachemem_out.instr_odd)
  //save instr. ouput since method block at the given address could be overwritten during fetch
  when (io.mcache_ctrlrepl.instr_stall === Bits(0)) {
    instr_aReg := io.mcachefe.instr_a
    instr_bReg := io.mcachefe.instr_b
  }
  io.mcachefe.instr_a := Mux(io.mcache_ctrlrepl.instr_stall, instr_aReg, instr_a)
  io.mcachefe.instr_b := Mux(io.mcache_ctrlrepl.instr_stall, instr_bReg, instr_b)

  io.mcachefe.relBase := relBase
  io.mcachefe.relPc := relPc
  io.mcachefe.reloc := reloc
  io.mcachefe.mem_sel := Cat(selIspmReg, selMCacheReg)

  io.mcache_replctrl.hit := hitReg
  io.mcache_replctrl.pos_offset := wrPosReg

  io.hit_ena := hitReg
}

/*
 MCacheReplLru: LRU replacement strategy for the method cache
 */
class MCacheReplLru(method_count : Int = METHOD_COUNT) extends Component {
  val io = new MCacheReplIO()

  //tag field and address translation table
  val mcache_addr_vec = { Vec(method_count) { Reg(resetVal = Bits(0, width = ADDR_WIDTH)) } }
  val mcache_mmu_vec = { Vec(method_count * method_count) { Reg(resetVal = Bits(0, width = log2Up(method_count))) } }
  val mcache_mmu_size = { Vec(method_count) { Reg(resetVal = Bits(0, width = log2Up(method_count))) } }
  //linked list for lru replacement
  val lru_list_prev = Vec(method_count) { Reg(resetVal = Bits(0, width = log2Up(method_count))) }
  val lru_list_next = Vec(method_count) { Reg(resetVal = Bits(0, width = log2Up(method_count))) }
  val lru_tag = Reg(resetVal = Bits(0, width = log2Up(method_count)))
  val mru_tag = Reg(resetVal = Bits(method_count - 1, width = log2Up(method_count)))
  //val lru_pos = Reg(resetVal = Bits(0, width = log2Up(method_count)))
  //registers for splitting up
  val split_msize_cnt = Reg(resetVal = Bits(0, width = MCACHE_SIZE_WIDTH))
  val mmu_offset_cnt = Reg(resetVal = Bits(0, width = log2Up(method_count)))
  val update_cnt = Reg(resetVal = Bits(0, width = log2Up(method_count)))
  //variables when call/return occurs to check and set tag fields
  val hitReg = Reg(resetVal = Bits(1, width = 1))
  val currPosReg = Reg(resetVal = Bits(0, width = method_count))
  val posReg = Reg(resetVal = Bits(0, width = 32))
  val callRetBaseReg = Reg(resetVal = UFix(1, DATA_WIDTH))
  val callAddrReg = Reg(resetVal = UFix(1, DATA_WIDTH))
  val selIspmReg = Reg(resetVal = Bits(0))
  val selMCacheReg = Reg(resetVal = Bits(0))
  // val rdPosReg = Reg(resetVal = Bits(0)) 
  // val wrPosReg = Reg(resetVal = Bits(0))

  def update_tag(tag : Bits) = {
    when (tag === lru_tag) {
      lru_tag := lru_list_prev(tag)
      mru_tag := tag
      lru_list_next(tag) := mru_tag
      lru_list_prev(mru_tag) := tag
      lru_list_prev(tag) := tag //no previous any more because mru
    }
    .elsewhen (tag != mru_tag) {
      lru_list_next(lru_list_prev(tag)) := lru_list_next(tag)
      lru_list_prev(lru_list_next(tag)) := lru_list_prev(tag)
      lru_list_next(tag) := mru_tag
      lru_list_prev(mru_tag) := tag
      mru_tag := tag
    }
  }

  // def search_tag(tag : Bits) = {
  //     for (i <- 0 until method_count) {
  //       when (mcache_shift_tag === tag) {

  //        }
  //     }
  // }
  //implementation with shift registers
  // val mcache_shift_tag = { Vec(method_count) {Reg(resetVal = Bits(0, width = log2Up(method_count)))} }
  // def update_shift_tag(tag : Bits) = {
  //   for (i <- 0 until method_count) {
  //     when (mcache_shift_tag(i) != tag) {
  //       mcache_shift_tag(i) := mcache_shift_tag(i)
  //     }
  //   }
  //   mcache_shift_tag(0) := tag
  // }

  when (io.exmcache.doCallRet) {

    callRetBaseReg := io.exmcache.callRetBase
    callAddrReg := io.exmcache.callRetAddr
    selIspmReg := io.exmcache.callRetBase(DATA_WIDTH - 1,ISPM_ONE_BIT - 2) === Bits(0x1)
    selMCacheReg := io.exmcache.callRetBase(DATA_WIDTH - 1,15) >= Bits(0x1)

    when (io.exmcache.callRetBase(DATA_WIDTH - 1,15) >= Bits(0x1)) {
      hitReg := Bits(0)
      for (i <- 0 until method_count) {
        when (io.exmcache.callRetBase === mcache_addr_vec(i)) {
          hitReg := Bits(1)
          currPosReg := Bits(i << log2Up(method_count))
          posReg:= Bits(i << (log2Up(method_count)+METHOD_BLOCK_SIZE_WIDTH))
        }
      }
    }
  }

  val relBase = Mux(selMCacheReg,
    posReg.toUFix,
    callRetBaseReg(ISPM_ONE_BIT-3, 0))
  val relPc = callAddrReg + relBase

  val reloc = Mux(selMCacheReg,
                  callRetBaseReg - posReg.toUFix,
                  UFix(1 << (ISPM_ONE_BIT - 2)))

  //sequentially update of all connected blocks (maybe stall here, what happens when there is always a call/hit?!)
  val doCallRetReg = Reg(io.exmcache.doCallRet)
  when (doCallRetReg && hitReg) {
    update_tag((currPosReg/Bits(4))(log2Up(method_count)-1,0))
    //update_shift_tag((currPosReg/Bits(4))(log2Up(method_count)-1,0))
    update_cnt := mcache_mmu_size(currPosReg/Bits(4))
  }
  when (update_cnt > Bits(0)) {
    update_cnt := update_cnt - Bits(1)
    update_tag(mcache_mmu_vec(currPosReg + update_cnt))
    //update_shift_tag(mcache_mmu_vec(update_cnt))
  }

  val address_in_pos = io.mcache_ctrlrepl.address(METHOD_BLOCK_SIZE_WIDTH*2+log2Up(method_count)-1,METHOD_BLOCK_SIZE_WIDTH)
  val address_in_offset = io.mcache_ctrlrepl.address(METHOD_BLOCK_SIZE_WIDTH-1,0)
  val w_address_pos = io.mcache_ctrlrepl.w_addr(METHOD_BLOCK_SIZE_WIDTH+log2Up(method_count),METHOD_BLOCK_SIZE_WIDTH)
  val w_address_offset = io.mcache_ctrlrepl.w_addr(METHOD_BLOCK_SIZE_WIDTH-1,0)
  //hmmm this one should be done in a second pipeline stage!
  val rdPos = Cat(mcache_mmu_vec(address_in_pos), address_in_offset)
  //also the write should be moved to a fix input write address without adding currPosReg > change MCacheCtrl
  val wrPos = Cat(mcache_mmu_vec(w_address_pos + currPosReg), w_address_offset)

  //insert new tags
  when (io.mcache_ctrlrepl.w_tag) {
     //we have again a hit!
    hitReg := Bits(1)
    //start splitting into more blocks if current method size > method block size
    split_msize_cnt := io.mcache_ctrlrepl.w_data(METHOD_BLOCK_SIZE_WIDTH+log2Up(method_count), METHOD_BLOCK_SIZE_WIDTH)
    mmu_offset_cnt := Bits(1)
    //update lru tag field
    currPosReg := (lru_tag << Bits(log2Up(method_count)))
    posReg := (lru_tag << Bits(log2Up(method_count)+METHOD_BLOCK_SIZE_WIDTH))
    mcache_addr_vec(lru_tag) := io.mcache_ctrlrepl.w_addr
    mcache_mmu_vec(lru_tag * Bits(method_count)) := lru_tag
    mcache_mmu_size(lru_tag) := io.mcache_ctrlrepl.w_data(METHOD_BLOCK_SIZE_WIDTH+log2Up(method_count)-1, METHOD_BLOCK_SIZE_WIDTH)
    update_tag(lru_tag)
    //update_shift_tag(lru_tag)
  }

  when (split_msize_cnt > Bits(0)) {
    split_msize_cnt := split_msize_cnt - Bits(1)
    mmu_offset_cnt := mmu_offset_cnt + Bits(1)
    mcache_addr_vec(lru_tag) := Bits(0) //invalidate field
    mcache_mmu_vec(currPosReg + mmu_offset_cnt) := lru_tag
    update_tag(lru_tag)
    //update_shift_tag(lru_tag)
  }

  val wr_parity = wrPos(0)
  val mcachemem_w_address = (wrPos)(11,1)
  val rd_parity = rdPos(0)
  val mcachemem_in_address = (rdPos)(11,1)
  val addr_parity_reg = Reg(rd_parity)
  //save value till address translation found the right place where to write
  // val wr_enaReg = Reg(io.mcache_ctrlrepl.w_enable)
  // val wr_dataReg = Reg(io.mcache_ctrlrepl.w_data)
  val wr_ena = io.mcache_ctrlrepl.w_enable
  val wr_data = io.mcache_ctrlrepl.w_data

  //read/write to mcachemem
  io.mcachemem_in.w_even := Mux(wr_parity, Bits(0), wr_ena)
  io.mcachemem_in.w_odd := Mux(wr_parity, wr_ena, Bits(0))
  io.mcachemem_in.w_data := wr_data
  io.mcachemem_in.w_addr := mcachemem_w_address
  io.mcachemem_in.addr_even := Mux(rd_parity, mcachemem_in_address + Bits(1), mcachemem_in_address)
  io.mcachemem_in.addr_odd := mcachemem_in_address
  //signals to fetch stage
  io.mcachefe.instr_a := Mux(addr_parity_reg, io.mcachemem_out.instr_odd, io.mcachemem_out.instr_even)
  io.mcachefe.instr_b := Mux(addr_parity_reg, io.mcachemem_out.instr_even, io.mcachemem_out.instr_odd)
  io.mcachefe.relBase := relBase
  io.mcachefe.relPc := relPc
  io.mcachefe.reloc := reloc
  io.mcachefe.mem_sel := Cat(selIspmReg, selMCacheReg)
  //signals to ctrl unit
  io.mcache_replctrl.hit := hitReg
  io.mcache_replctrl.pos_offset := wrPos
  //hit/stall signal
  io.hit_ena := hitReg

}

