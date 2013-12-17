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
 Method Cache for Patmos
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
  Method Cache Constants only used internally in MCache.scala
 */
object MConstants {
 
  val MCACHE_WORD_SIZE = MCACHE_SIZE / 4
  val METHOD_BLOCK_SIZE = MCACHE_WORD_SIZE / METHOD_COUNT
  val METHOD_COUNT_WIDTH = log2Up(METHOD_COUNT)
  val METHOD_BLOCK_SIZE_WIDTH = log2Up(METHOD_BLOCK_SIZE)
  val MCACHE_SIZE_WIDTH = log2Up(MCACHE_WORD_SIZE)

}

/*
  Internal and external connections for the Method Cache
 */
class FeMCache extends Bundle() {
  val address_even = Bits(width = EXTMEM_ADDR_WIDTH) 
  val address_odd = Bits(width = EXTMEM_ADDR_WIDTH) 
  val request = Bool()
  val doCallRet = Bool()
  val callRetBase = UInt(width = EXTMEM_ADDR_WIDTH)
}
class ExMCache() extends Bundle() {
  val doCallRet = Bool()
  val callRetBase = UInt(width = EXTMEM_ADDR_WIDTH)
  val callRetAddr = UInt(width = EXTMEM_ADDR_WIDTH)
}
class MCacheFe extends Bundle() {
  val instr_a = Bits(width = INSTR_WIDTH)
  val instr_b = Bits(width = INSTR_WIDTH)
  // relative base address
  val relBase = UInt(width = MAX_OFF_WIDTH)
  // relative program counter
  val relPc = UInt(width = MAX_OFF_WIDTH+1)
  // offset between relative and absolute program counter
  val reloc = UInt(width = DATA_WIDTH)
  val mem_sel = Bits(width = 2)
}
class MCacheIO extends Bundle() {
  val ena_out = Bool(OUTPUT)
  val ena_in = Bool(INPUT)
  val femcache = new FeMCache().asInput
  val exmcache = new ExMCache().asInput
  val mcachefe = new MCacheFe().asOutput
  val ocp_port = new OcpBurstMasterPort(EXTMEM_ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
}
class MCacheCtrlIO extends Bundle() {
  val ena_in = Bool(INPUT)
  val fetch_ena = Bool(OUTPUT)
  val mcache_ctrlrepl = new MCacheCtrlRepl().asOutput
  val mcache_replctrl = new MCacheReplCtrl().asInput
  val femcache = new FeMCache().asInput
  val exmcache = new ExMCache().asInput
  val ocp_port = new OcpBurstMasterPort(EXTMEM_ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
}
class MCacheCtrlRepl extends Bundle() {
  val w_enable = Bool()
  val w_data = Bits(width = INSTR_WIDTH)
  val w_addr = Bits(width = EXTMEM_ADDR_WIDTH)
  val w_tag = Bool()
  val address_even = Bits(width = MCACHE_SIZE_WIDTH)
  val address_odd = Bits(width = MCACHE_SIZE_WIDTH)
  val instr_stall = Bool()
}
class MCacheReplCtrl extends Bundle() {
  val hit = Bool()
  val pos_offset = Bits(width = MAX_OFF_WIDTH)
}
class MCacheReplIO extends Bundle() {
  val ena_in = Bool(INPUT)
  val hit_ena = Bool(OUTPUT)
  val exmcache = new ExMCache().asInput
  val mcachefe = new MCacheFe().asOutput
  val mcache_ctrlrepl = new MCacheCtrlRepl().asInput
  val mcache_replctrl = new MCacheReplCtrl().asOutput
  val mcachemem_in = new MCacheMemIn().asOutput
  val mcachemem_out = new MCacheMemOut().asInput
}
class MCacheMemIn extends Bundle() {
  val w_even = Bool()
  val w_odd = Bool()
  val w_data = Bits(width = DATA_WIDTH)
  val w_addr = Bits(width = (log2Up(MCACHE_WORD_SIZE / 2)))
  val addr_even = Bits(width = (log2Up(MCACHE_WORD_SIZE / 2)))
  val addr_odd = Bits(width = (log2Up(MCACHE_WORD_SIZE / 2)))
}
class MCacheMemOut extends Bundle() {
  val instr_even = Bits(width = INSTR_WIDTH)
  val instr_odd = Bits(width = INSTR_WIDTH)
}
class MCacheMemIO extends Bundle() {
  val mcachemem_in = new MCacheMemIn().asInput
  val mcachemem_out = new MCacheMemOut().asOutput
}

/*
 MCache: Top Level Class for the Method Cache
 */
class MCache() extends Module {
  val io = new MCacheIO()
  val mcachectrl = Module(new MCacheCtrl())
  val mcacherepl = Module(new MCacheReplFifo())
  val mcachemem = Module(new MCacheMem())
  //connect inputs to method cache ctrl unit
  mcachectrl.io.mcache_ctrlrepl <> mcacherepl.io.mcache_ctrlrepl
  mcachectrl.io.femcache <> io.femcache
  mcachectrl.io.exmcache <> io.exmcache
  mcachectrl.io.ocp_port <> io.ocp_port
  //connect inputs to method cache repl unit
  mcacherepl.io.exmcache <> io.exmcache
  mcacherepl.io.mcachefe <> io.mcachefe
  mcacherepl.io.mcache_replctrl <> mcachectrl.io.mcache_replctrl
  //connect repl to on chip memory
  mcacherepl.io.mcachemem_in <> mcachemem.io.mcachemem_in
  mcacherepl.io.mcachemem_out <> mcachemem.io.mcachemem_out
  //connect enables
  mcachectrl.io.ena_in <> io.ena_in
  mcacherepl.io.ena_in <> io.ena_in
  //output enable depending on hit/miss/fetch
  io.ena_out := mcachectrl.io.fetch_ena & mcacherepl.io.hit_ena
}

/*
 MCacheMem: On-Chip Even/Odd Memory
 */
class MCacheMem() extends Module {
  val io = new MCacheMemIO()

  val ram_mcache_even = Mem(Bits(width = INSTR_WIDTH), MCACHE_WORD_SIZE / 2)
  val ram_mcache_odd = Mem(Bits(width = INSTR_WIDTH), MCACHE_WORD_SIZE / 2)

  when (io.mcachemem_in.w_even) {
	ram_mcache_even(io.mcachemem_in.w_addr) := io.mcachemem_in.w_data
  }
  when (io.mcachemem_in.w_odd) {
	ram_mcache_odd(io.mcachemem_in.w_addr) := io.mcachemem_in.w_data
  }

  val addrEvenReg = Reg(next = io.mcachemem_in.addr_even)
  val addrOddReg = Reg(next = io.mcachemem_in.addr_odd)
  io.mcachemem_out.instr_even := ram_mcache_even(addrEvenReg)
  io.mcachemem_out.instr_odd := ram_mcache_odd(addrOddReg)

}


/*
 MCacheReplFifo: Class controlls a FIFO replacement strategie including tag-memory to keep history of methods in cache. 
 Note: A variable block size is used in this replacement
 */
class MCacheReplFifo() extends Module {
  val io = new MCacheReplIO()

  //tag field tables  for reading tag memory
  val mcache_addr_vec = { Vec.fill(METHOD_COUNT) { Reg(init = Bits(0, width = EXTMEM_ADDR_WIDTH)) } }
  val mcache_size_vec = { Vec.fill(METHOD_COUNT) { Reg(init = Bits(0, width = MCACHE_SIZE_WIDTH+1)) } }
  val mcache_valid_vec = { Vec.fill(METHOD_COUNT) { Reg(init = Bool(false)) } }
  val mcache_pos_vec = { Vec.fill(METHOD_COUNT) { Reg(init = Bits(0, width = MCACHE_SIZE_WIDTH)) } }
  //registers to save current replacement status
  val next_index_tag = Reg(init = Bits(0, width = log2Up(METHOD_COUNT)))
  val next_replace_tag = Reg(init = Bits(0, width = log2Up(METHOD_COUNT)))
  val next_replace_pos = Reg(init = Bits(0, width = MCACHE_SIZE_WIDTH))
  val free_space = Reg(init = SInt(MCACHE_WORD_SIZE, width = MCACHE_SIZE_WIDTH+2))
  //variables when call/return occurs to check tag field
  val posReg = Reg(init = Bits(0, width = MCACHE_SIZE_WIDTH))
  val hitReg = Reg(init = Bool(true))
  val wrPosReg = Reg(init = Bits(0, width = MCACHE_SIZE_WIDTH))
  val callRetBaseReg = Reg(init = UInt(1, DATA_WIDTH))
  val callAddrReg = Reg(init = UInt(1, DATA_WIDTH))
  val selIspmReg = Reg(init = Bool(false))
  val selMCacheReg = Reg(init = Bool(false))

  val hit = Bool()
  hit := Bool(false)

  val mergePosVec = { Vec.fill(METHOD_COUNT) { Bits(width = MCACHE_SIZE_WIDTH) } }
  for (i <- 0 until METHOD_COUNT) {
    mergePosVec(i) := Bits(0)
  }

  //read from tag memory on call/return to check if method is in the cache
  when (io.exmcache.doCallRet && io.ena_in) {

    callRetBaseReg := io.exmcache.callRetBase
    callAddrReg := io.exmcache.callRetAddr
    selIspmReg := io.exmcache.callRetBase(EXTMEM_ADDR_WIDTH - 1,ISPM_ONE_BIT - 2) === Bits(0x1)
    selMCacheReg := io.exmcache.callRetBase(EXTMEM_ADDR_WIDTH - 1,15) >= Bits(0x1)

    when (io.exmcache.callRetBase(EXTMEM_ADDR_WIDTH-1,15) >= Bits(0x1)) {
      hitReg := Bool(false)
      for (i <- 0 until METHOD_COUNT) {
        when (io.exmcache.callRetBase === mcache_addr_vec(i)
              && mcache_valid_vec(i)) {
                hitReg := Bool(true)
                hit := Bool(true)
                mergePosVec(i) := mcache_pos_vec(i)
              }
      }
      posReg := next_replace_pos
      when (hit) {
        posReg := mergePosVec.fold(Bits(0))(_|_)
      }
    }
  }

  val relBase = Mux(selMCacheReg,
                    posReg.toUInt,
                    callRetBaseReg(ISPM_ONE_BIT-3, 0))
  val relPc = callAddrReg + relBase

  val reloc = Mux(selMCacheReg,
                  callRetBaseReg - posReg.toUInt,
                  Mux(selIspmReg,
                      UInt(1 << (ISPM_ONE_BIT - 2)),
                      UInt(0)))

  //insert new tags when control unit requests
  when (io.mcache_ctrlrepl.w_tag) {
    hitReg := Bool(true) //start fetch, we have again a hit!
    wrPosReg := posReg
    //update free space
    free_space := free_space - io.mcache_ctrlrepl.w_data(MCACHE_SIZE_WIDTH,0) + mcache_size_vec(next_index_tag)
    //update tag fields
    mcache_pos_vec(next_index_tag) := next_replace_pos
    mcache_size_vec(next_index_tag) := io.mcache_ctrlrepl.w_data(MCACHE_SIZE_WIDTH, 0)
    mcache_addr_vec(next_index_tag) := io.mcache_ctrlrepl.w_addr
    mcache_valid_vec(next_index_tag) := Bool(true)
    //update pointers
    next_replace_pos := next_replace_pos + io.mcache_ctrlrepl.w_data(MCACHE_SIZE_WIDTH-1,0)
    val next_tag = Mux(next_index_tag === Bits(METHOD_COUNT - 1), Bits(0), next_index_tag + Bits(1))
    next_index_tag := next_tag
    when (next_replace_tag === next_index_tag) {
      next_replace_tag := next_tag
    }
  }
  //free new space if still needed -> invalidate next method
  when (free_space < SInt(0)) {
    free_space := free_space + mcache_size_vec(next_replace_tag)
    mcache_size_vec(next_replace_tag) := Bits(0)
    //mcache_addr_vec(next_replace_tag) := Bits(0)
    mcache_valid_vec(next_replace_tag) := Bool(false)
    next_replace_tag := Mux(next_replace_tag === Bits(METHOD_COUNT - 1), Bits(0), next_replace_tag + Bits(1))
  }

  val wr_parity = io.mcache_ctrlrepl.w_addr(0)
  //adder could be moved to ctrl. unit to operate with rel. addresses here
  val mcachemem_w_address = (wrPosReg + io.mcache_ctrlrepl.w_addr)(MCACHE_SIZE_WIDTH-1,1)
  val mcachemem_in_address_even = (io.mcache_ctrlrepl.address_even)(MCACHE_SIZE_WIDTH-1,1)
  val mcachemem_in_address_odd = (io.mcache_ctrlrepl.address_odd)(MCACHE_SIZE_WIDTH-1,1)
  //remember parity for the next cycle
  val addr_parity_reg = Reg(next = io.mcache_ctrlrepl.address_odd(0))

  io.mcachemem_in.w_even := Mux(wr_parity, Bool(false), io.mcache_ctrlrepl.w_enable)
  io.mcachemem_in.w_odd := Mux(wr_parity, io.mcache_ctrlrepl.w_enable, Bool(false))
  io.mcachemem_in.w_data := io.mcache_ctrlrepl.w_data
  io.mcachemem_in.w_addr := mcachemem_w_address
  io.mcachemem_in.addr_even := mcachemem_in_address_even
  io.mcachemem_in.addr_odd := mcachemem_in_address_odd

  val instr_aReg = Reg(init = Bits(0, width = INSTR_WIDTH))
  val instr_bReg = Reg(init = Bits(0, width = INSTR_WIDTH))
  val instr_a = Mux(addr_parity_reg, io.mcachemem_out.instr_odd, io.mcachemem_out.instr_even)
  val instr_b = Mux(addr_parity_reg, io.mcachemem_out.instr_even, io.mcachemem_out.instr_odd)
  when (!io.mcache_ctrlrepl.instr_stall) {
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
 MCacheCtrl Class: Main Class of Method Cache, implements the State Machine and handles the R/W/Fetch of Cache and External Memory
 */
class MCacheCtrl() extends Module {
  val io = new MCacheCtrlIO()

  //fsm state variables
  val init_state :: idle_state :: size_state :: transfer_state :: restart_state :: Nil = Enum(UInt(), 5)
  val mcache_state = Reg(init = init_state)
  //signals for method cache memory (mcache_repl)
  val mcachemem_address_even = Bits(width = EXTMEM_ADDR_WIDTH) //not needed here we are on relative addresses!
  val mcachemem_address_odd = Bits(width = EXTMEM_ADDR_WIDTH)
  val mcachemem_w_data = Bits(width = DATA_WIDTH)
  val mcachemem_w_tag = Bool() //signalizes the transfer of begin of a write
  val mcachemem_w_addr = Bits(width = EXTMEM_ADDR_WIDTH)
  val mcachemem_w_enable = Bool()
  //signals for external memory
  val ext_mem_cmd = Bits(width = 3)
  val ext_mem_addr = Bits(width = EXTMEM_ADDR_WIDTH)
  val ext_mem_tsize = Reg(init = Bits(0, width = MCACHE_SIZE_WIDTH))
  val ext_mem_fcounter = Reg(init = Bits(0, width = MCACHE_SIZE_WIDTH))
  val ext_mem_burst_cnt = Reg(init = UInt(0, width = log2Up(BURST_LENGTH)))
  //input/output registers
  val callRetBaseReg = Reg(init = Bits(0, width = EXTMEM_ADDR_WIDTH))
  val msize_addr = callRetBaseReg - Bits(1)
  val addrEvenReg = Reg(init = Bits(0))
  val addrOddReg = Reg(init = Bits(0))
  val wenaReg = Reg(init = Bool(false))

  val ocpSlaveReg = Reg(next = io.ocp_port.S)

  //init signals
  mcachemem_address_even := addrEvenReg
  mcachemem_address_odd := addrOddReg
  mcachemem_w_data := Bits(0)
  mcachemem_w_tag := Bool(false)
  mcachemem_w_enable := Bool(false)
  mcachemem_w_addr := Bits(0)
  ext_mem_cmd := OcpCmd.IDLE
  ext_mem_addr := Bits(0)

  when (io.exmcache.doCallRet) {
    callRetBaseReg := io.exmcache.callRetBase // use callret to save base address for next cycle
    addrEvenReg := io.femcache.address_even
    addrOddReg := io.femcache.address_odd
  }

  //init state needs to fetch at program counter - 1 the first size of method block
  when (mcache_state === init_state) {
    when(io.femcache.request) {
      mcache_state := idle_state
    }
  }
  //check if instruction is available
  when (mcache_state === idle_state) {
    when(io.mcache_replctrl.hit === Bits(1)) {
      mcachemem_address_even := io.femcache.address_even
      mcachemem_address_odd := io.femcache.address_odd
    }
    //no hit... fetch from external memory
    .otherwise {
      ext_mem_addr := Cat(msize_addr(EXTMEM_ADDR_WIDTH-1,2), Bits("b00")) //aligned read from ssram
      ext_mem_cmd := OcpCmd.RD
      ext_mem_burst_cnt := UInt(0)
      mcache_state := size_state
      wenaReg := Bool(true)
    }
  }
  //fetch size of the required method from external memory address - 1
  when (mcache_state === size_state) {
    when (ocpSlaveReg.Resp === OcpResp.DVA) {
      ext_mem_burst_cnt := ext_mem_burst_cnt + Bits(1)
      when (ext_mem_burst_cnt === msize_addr(1,0)) {
        val size = ocpSlaveReg.Data(MCACHE_SIZE_WIDTH+2,2)
        //init transfer from external memory
        ext_mem_tsize := size
        ext_mem_fcounter := Bits(0) //start to write to cache with offset 0
        when (ext_mem_burst_cnt >= UInt(BURST_LENGTH - 1)) {
          ext_mem_addr := callRetBaseReg
          ext_mem_cmd := OcpCmd.RD
          ext_mem_burst_cnt := UInt(0)
        }
        //init transfer to on-chip method cache memory
        mcachemem_w_tag := Bool(true)
        //size rounded to next double-word
        mcachemem_w_data := size+size(0)
        //write base address to mcachemem for tagfield
        mcachemem_w_addr := callRetBaseReg
        mcache_state := transfer_state
      }
    }
  }

  //transfer/fetch method to the cache
  when (mcache_state === transfer_state) {
    when (ext_mem_fcounter < ext_mem_tsize) {
      when (ocpSlaveReg.Resp === OcpResp.DVA) {
        ext_mem_fcounter := ext_mem_fcounter + Bits(1)
        ext_mem_burst_cnt := ext_mem_burst_cnt + Bits(1)
        when(ext_mem_fcounter < ext_mem_tsize - Bits(1)) {
          //fetch next address from external memory
          when (ext_mem_burst_cnt >= UInt(BURST_LENGTH - 1)) {
            ext_mem_cmd := OcpCmd.RD
            ext_mem_addr := callRetBaseReg + ext_mem_fcounter + Bits(1) //need +1 because start fetching with the size of method
            ext_mem_burst_cnt := UInt(0)
          }
        }
        //write current address to mcache memory
        mcachemem_w_data := ocpSlaveReg.Data
        mcachemem_w_enable := Bool(true)
      }
      mcachemem_w_addr := ext_mem_fcounter //+ io.mcache_replctrl.pos_offset
    }
    //restart to idle state
    .otherwise {
      mcache_state := restart_state //restart_state
      wenaReg := Bool(false)
    }
  }
  when (mcache_state === restart_state) {
    mcachemem_address_even := io.femcache.address_even
    mcachemem_address_odd := io.femcache.address_odd
    mcache_state := idle_state
  }

  //outputs to mcache memory
  io.mcache_ctrlrepl.address_even := mcachemem_address_even
  io.mcache_ctrlrepl.address_odd := mcachemem_address_odd
  io.mcache_ctrlrepl.w_enable := mcachemem_w_enable
  io.mcache_ctrlrepl.w_data := mcachemem_w_data
  io.mcache_ctrlrepl.w_addr := mcachemem_w_addr
  io.mcache_ctrlrepl.w_tag := mcachemem_w_tag
  io.mcache_ctrlrepl.instr_stall := mcache_state != idle_state

  io.fetch_ena := !wenaReg

  //output to external memory
  io.ocp_port.M.Addr := Cat(ext_mem_addr, Bits("b00"))
  io.ocp_port.M.Cmd := ext_mem_cmd
  io.ocp_port.M.Data := Bits(0)
  io.ocp_port.M.DataByteEn := Bits("b1111")
  io.ocp_port.M.DataValid := Bits(0)

}

