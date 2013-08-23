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

/*
 Here should be listed a summary of the costs of the various replacement strategies in future

 LRU Replacement with fixed block size (MAYBE USE THE LRU DESIGN with shift registers to save list with pointers
 --> cost: addr_tag , size_tag, list(prev/next), search logic: all depending on method count in cache, LRU/MRU reg
 
 LRU Replacement with variable size
 VARIABLE BLOCK SIZE its getting more complicated we have to track the position of each
 ...but we run into problems when replacing there is needed an adress translation

 FIFO with variable block size
 --> cost: addr_tag, size_tag, pos_tag, search logic, next_index reg, next_replace reg, free_space reg
 
 FIFO with fixed block size could be implemented for comparison, this is straight forward just use
 a next_tag instead of the lru_tag at replacement and no lru table is needed than...
 --> cost: addr_tag, size_tag, search logic: all depending on method count in cache, NEXT reg

*/

package patmos

import Chisel._
import Node._
import MConstants._
import Constants._
import MCacheRepl._
import ocp._

import scala.collection.mutable.HashMap
import scala.util.Random
import scala.math

//move to Constants.scala
object MConstants {
  val MCACHE_SIZE = 4096
  val METHOD_COUNT = 4
  val METHOD_BLOCK_SIZE = MCACHE_SIZE / METHOD_COUNT
  val METHOD_SIZETAG_WIDTH = log2Up(MCACHE_SIZE)
  val METHOD_COUNT_WIDTH = log2Up(METHOD_COUNT)
  val WORD_COUNT = 4
  val BURST_LENGHT = 4 //for ssram max. 4
  val LRU_REPL = 1
  val FIFO_REPL = 2
  val FIXED_SIZE = 1
  val VARIABLE_SIZE = 2
}

class MCacheIn extends Bundle() {
  val address = Bits(width = ADDR_WIDTH) 
  val doCallRet = Bits(width = 1) //need to know when a method return is executed
  val callRetBase = Bits(width = ADDR_WIDTH) //to check if method is still in cache
  val request = Bits(width = 1)
}
class MCacheOut extends Bundle() {
  val instr_a = Bits(width = 32) //lower 32 bits
  val instr_b = Bits(width = 32) //higher 32 bits
}
class MCacheIO extends Bundle() {
  val ena = Bool(OUTPUT)
  val mcache_in = new MCacheIn().asInput
  val mcache_out = new MCacheOut().asOutput
  val ocp_port = new OcpBurstMasterPort(19, DATA_WIDTH, BURST_LENGHT)
}
class MCacheCtrlIO extends Bundle() {
  val ena = Bits(OUTPUT, width = 1)
  val mcache_repl_in = new MCacheReplIn().asOutput
  val mcache_repl_out = new MCacheReplOut().asInput
  val mcache_in = new MCacheIn().asInput
  val mcache_out = new MCacheOut().asOutput
  val ocp_port = new OcpBurstMasterPort(19, DATA_WIDTH, BURST_LENGHT)
}
class MCacheReplIn extends Bundle() {
  val w_enable = Bits(width = 1)
  val w_data = Bits(width = 32)
  val address = Bits(width = 32)
  val w_tag = Bits(width = 1)
  val doCallRet = Bits(width = 1)
  val callRetBase = Bits(width = 32)
}
class MCacheReplOut extends Bundle() {
  val instr_a = Bits(width = INSTR_WIDTH)
  val instr_b = Bits(width = INSTR_WIDTH)
  val hit = Bits(width = 1)
}
class MCacheReplIO extends Bundle() {
  val mcache_repl_in = new MCacheReplIn().asInput
  val mcache_repl_out = new MCacheReplOut().asOutput
  val mcachemem_in = new MCacheMemIn().asOutput
  val mcachemem_out = new MCacheMemOut().asInput
}
class MCacheMemIn extends Bundle() {
  val w_even = Bits(width = 1)
  val w_odd = Bits(width = 1)
  val w_data = Bits(width = DATA_WIDTH)
  val addr_even = Bits(width = (log2Up(MCACHE_SIZE / 2)))
  val addr_odd = Bits(width = (log2Up(MCACHE_SIZE / 2)))
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
class MCache() extends Component {
  val io = new MCacheIO()
  val mcachectrl = new MCacheCtrl()
  val mcacherepl = new MCacheReplFifo(method_count = METHOD_COUNT)
  val mcachemem = new MCacheMem()
  //connect ctrl to replacmenet block and method cache on chip memory
  mcachectrl.io.mcache_repl_in <> mcacherepl.io.mcache_repl_in
  mcachectrl.io.mcache_repl_out <> mcacherepl.io.mcache_repl_out
  mcachectrl.io.mcache_in <> io.mcache_in 
  mcachectrl.io.mcache_out <> io.mcache_out
  mcachectrl.io.ocp_port <> io.ocp_port
  mcachectrl.io.ena <> io.ena
  //connect repl to on chip memory
  mcacherepl.io.mcachemem_in <> mcachemem.io.mcachemem_in
  mcacherepl.io.mcachemem_out <> mcachemem.io.mcachemem_out
}

/*
 MCacheMem: On-Chip Memory
 */
class MCacheMem() extends Component {
  val io = new MCacheMemIO()

  val ram_mcache_even = { Mem(MCACHE_SIZE / 2, seqRead = true) {Bits(width = INSTR_WIDTH)} }
  val ram_mcache_odd = { Mem(MCACHE_SIZE / 2, seqRead = true) {Bits(width = INSTR_WIDTH)} }
  val dout_even = Reg() {Bits(width = INSTR_WIDTH)}
  val dout_odd = Reg() {Bits(width = INSTR_WIDTH)}

  when (io.mcachemem_in.w_even) {ram_mcache_even(io.mcachemem_in.addr_even) := io.mcachemem_in.w_data}
  .otherwise {(dout_even := ram_mcache_even(io.mcachemem_in.addr_even))}

  when (io.mcachemem_in.w_odd) {ram_mcache_odd(io.mcachemem_in.addr_odd) := io.mcachemem_in.w_data}
  .otherwise {(dout_odd := ram_mcache_odd(io.mcachemem_in.addr_odd))}

  io.mcachemem_out.instr_even := dout_even
  io.mcachemem_out.instr_odd := dout_odd
}

object MCacheRepl {
  //move all general functions for replacement class here!!!

  def get_address(pos : Bits,  offset : Bits) : Bits = {
    (((pos + offset) / UFix(2)) % Bits(MCACHE_SIZE / 2)) //modulo if you write over the cache size (variable blocks)
  }
  //TODO: should integrate the check for variable blocks too...
  def check_block_size(size : Bits) : Bits = {
    ((size - Bits(1)) / Bits(METHOD_BLOCK_SIZE))
  }

  //update tag field... not needed any more
  // def update_tag(tag : Bits) = {
  //   tag := (tag + Bits(1)) % Bits(METHOD_COUNT)
  // }
 
}

/*
 MCacheReplFifo: Class controlls a FIFO replacement strategie including tag-memory to keep history of methods in cache
 */
class MCacheReplFifo(method_count : Int = METHOD_COUNT) extends Component {
  val io = new MCacheReplIO()

  //vector of registers for reading tag memory
  val mcache_addr_vec = { Vec(method_count) { Reg(resetVal = Bits(0, width = 32)) } }
  val mcache_size_vec = { Vec(method_count) { Reg(resetVal = Bits(0, width = METHOD_SIZETAG_WIDTH)) } }
  val mcache_pos_vec = { Vec(method_count) { Reg(resetVal = Bits(0, width = METHOD_SIZETAG_WIDTH)) } }

  val next_index_tag = Reg(resetVal = Bits(0, width = log2Up(method_count)))
  val next_replace_tag = Reg(resetVal = Bits(0, width = log2Up(method_count)))
  val next_replace_pos = Reg(resetVal = Bits(0, width = 13))
  val free_space = Reg(resetVal = Fix(MCACHE_SIZE))

  //signal of the current tagfield with position and hit result
  class TagField extends Bundle {
    val pos = Bits(width = 13)
    val hit = Bits(width = 1)
    val tag = Bits(width = 32)
  }
  val tag_field = new TagField()
  tag_field.pos := Bits(0)
  tag_field.hit := Bits(0)
  tag_field.tag := Bits(0)
  //saves the current tagfield position since fetch can only occur on call/return
  val pos = Reg(resetVal = Bits(0))
  val hit = Reg(resetVal = Bits(0))
  val tag = Reg(resetVal = Bits(0))
  val tag_field_size = Reg(resetVal = Bits(0))
  //for writing
  val tag_rd_ena = Reg(resetVal = Bits(0, width = 1))
  val tag_wr_ena = Reg(resetVal = Bits(0, width = 1))
  val tag_wr_size = Reg(resetVal = Bits(0, width = 13))
  val addr_reg = Reg(resetVal = Bits(0, width = 32))
  tag_rd_ena := io.mcache_repl_in.doCallRet
  tag_wr_ena := io.mcache_repl_in.w_tag
  addr_reg := io.mcache_repl_in.address

  //need to save size to write and start reading new size from tag memory when starting new transfer
  when(io.mcache_repl_in.w_tag) {
    tag_wr_size := io.mcache_repl_in.w_data(12,0) //size of the new method
  }

  //search tag in list
  when (tag_rd_ena) {
    for (i <- 0 until method_count) {
      when ((addr_reg >= mcache_addr_vec(i)) && (addr_reg < (mcache_addr_vec(i) + mcache_size_vec(i)))) {
        tag_field.pos := mcache_pos_vec(i)
        tag_field.hit := Bits(1)
        tag_field.tag := mcache_addr_vec(i)
        pos := mcache_pos_vec(i)
        hit := Bits(1)
        tag := mcache_addr_vec(i)
      }
    }
  }
  .otherwise {
    tag_field.pos := pos
    tag_field.tag := tag
    tag_field.hit := hit
  }

  //read from tag memory to check if method is in the cache
  when(io.mcache_repl_in.doCallRet === Bits(1)) {
    // for (i <- 0 until method_count) {
    //   mcache_addr_vec(i) := mcache_tag.addr(Bits(i))
    //   mcache_size_vec(i) := mcache_tag.size(Bits(i))
    //   mcache_pos_vec(i) := mcache_tag.pos(Bits(i))
    // }
    hit := Bits(0)
  }

  //write to tag memory
  when (tag_wr_ena) {
    //update free space and tags
    free_space := free_space - tag_wr_size + tag_field_size
    when (free_space < tag_wr_size) {
      tag_field_size := mcache_size_vec(next_replace_tag)
    }
    //add new tags to memory
    mcache_pos_vec(next_index_tag) := next_replace_pos(11,0) //current position before update
    mcache_size_vec(next_index_tag) := tag_wr_size(11,0) //size of method
    mcache_addr_vec(next_index_tag) := addr_reg //address of method
    //update tags and fields
    next_replace_pos := ((next_replace_pos + tag_wr_size) % Bits(MCACHE_SIZE))
    pos := next_replace_pos //set new position for next replacement
    hit := Bits(1) //we have a hit till next call/return because tag is now in the field
    tag := addr_reg //set new address as current tag address
    tag_field.pos := next_replace_pos
    tag_field.hit := Bits(1)
    tag_field.tag := addr_reg
    next_index_tag := Mux(next_index_tag === Bits(3), Bits(0), next_index_tag + Bits(1))
    //update also replace tag since it is covered by index tag at  the next replacement
    //when((next_index_tag + Bits(1)) % Bits(METHOD_COUNT) === next_replace_tag) {
    when((next_index_tag + Bits(1)) === next_replace_tag) {
      next_replace_tag := Mux(next_replace_tag === Bits(3), Bits(0), next_replace_tag + Bits(1))
    }
  }

  //still more space is needed remove another tag from cache
  when (free_space < Fix(0)) {
    free_space := free_space + tag_field_size
    tag_field_size := mcache_size_vec(next_replace_tag)
    mcache_size_vec(next_replace_tag) := Bits(0)
    next_replace_tag := Mux(next_replace_tag === Bits(3), Bits(0), next_replace_tag + Bits(1))
  }

  val addr_offset = Mux(tag_rd_ena, (addr_reg - tag_field.tag), (io.mcache_repl_in.address - tag_field.tag)) //offset between incoming address and base address
  val addr_parity = (addr_offset(0) ^ tag_field.pos(0))
  val addr_parity_reg = Reg(addr_parity)

  io.mcachemem_in.w_even := Mux(addr_parity, Bits(0), io.mcache_repl_in.w_enable)
  io.mcachemem_in.w_odd := Mux(addr_parity, io.mcache_repl_in.w_enable, Bits(0))
  io.mcachemem_in.w_data := io.mcache_repl_in.w_data
  io.mcachemem_in.addr_even := Mux(addr_parity, get_address(tag_field.pos(11,0), addr_offset(11,0) + Bits(1)), get_address(tag_field.pos(11,0), addr_offset(11,0)))
  io.mcachemem_in.addr_odd := get_address(tag_field.pos(11,0), addr_offset(11,0))
  io.mcache_repl_out.instr_a := Mux(addr_parity_reg, io.mcachemem_out.instr_odd, io.mcachemem_out.instr_even)
  io.mcache_repl_out.instr_b := Mux(addr_parity_reg, io.mcachemem_out.instr_even, io.mcachemem_out.instr_odd)
  io.mcache_repl_out.hit := Mux(io.mcache_repl_in.w_enable, Bits(0), tag_field.hit)

}


/*
 MCacheCtrl Class: Main Class of Method Cache, implements the State Machine and handles the R/W/Fetch of Cache and External Memory
 */
class MCacheCtrl() extends Component {
  val io = new MCacheCtrlIO()

  //fsm state variables
  val init_state :: idle_state :: size_state :: transfer_state :: restart_state :: Nil = Enum(5){ UFix() }
  val mcache_state = Reg(resetVal = init_state)

  //signals for method cache memory
  val mcachemem_address = Bits(width = 32) //has to be the full address to save the exact tag
  val mcachemem_w_data = Bits(width = DATA_WIDTH) //instruction to be written
  val mcachemem_wtag = Bits(width = 1) //signalizes the transfer of begin of a write
  val mcachemem_doCallRet = Bits(width = 1)
  val mcachemem_w_enable = Bits(width = 1)
  //signals for fetch stage from mcachemem
  val mcache_instr_a = Bits(width = DATA_WIDTH)
  val mcache_instr_b = Bits(width = DATA_WIDTH)
  val mcache_hit = Bits(width = 1)
  //signals for external memory
  val ext_mem_cmd = Bits(width = 3)
  val ext_mem_addr = Bits(width = 23)
  //signals for external memory (old)
  val extmem_fetch = Bits(width = 1)
  val extmem_fetch_address = Bits(width = 32)
  val extmem_msize = Bits(width = METHOD_SIZETAG_WIDTH)
  //regs for external meomory
  val ext_mem_tsize = Reg(resetVal = Bits(0, width = 32))
  val ext_mem_fcounter = Reg(resetVal = Bits(0, width = 32))
  val ext_mem_burst_cnt = Reg(resetVal = UFix(0, width = log2Up(BURST_LENGHT)))
  //save address in case no hit occours
  val mcache_address = Reg(resetVal = Bits(0, width = 32))

  val doCallRet_reg = Reg(resetVal = Bits(0, width = 1))
  doCallRet_reg := io.mcache_in.doCallRet

  //init signals
  mcachemem_address := io.mcache_in.address
  mcachemem_w_data := Bits(0)
  mcachemem_wtag := Bits(0)
  mcachemem_doCallRet := io.mcache_in.doCallRet
  mcachemem_w_enable := Bits(0)
  mcache_hit := Bits(0)
  mcache_instr_a := io.mcache_repl_out.instr_a
  mcache_instr_b := io.mcache_repl_out.instr_b
  ext_mem_cmd := OcpCmd.IDLE
  ext_mem_addr := Bits(0)
  extmem_fetch := Bits(0)
  extmem_fetch_address := Bits(0)
  extmem_msize := Bits(0)

  //init state needs to fetch at program counter - 1 the first size of method block
  when (mcache_state === init_state) {
    mcache_hit := Bits(1)
    when(io.mcache_in.request) {
      mcache_address := io.mcache_in.address - Bits(1)
      mcachemem_address := io.mcache_in.address - Bits(1)
      mcache_state := idle_state
      mcache_hit := Bits(0)
    }
  }
  //check if instruction is available
  when (mcache_state === idle_state) {
    mcache_hit := io.mcache_repl_out.hit
    when(io.mcache_repl_out.hit === Bits(1)) {
      mcache_address := io.mcache_in.callRetBase // use callret to save base address for next cycle
      //short workaround we have one wait cycle between call and method is found in cache -> not needed in future
      when (doCallRet_reg) {
        mcache_instr_a := Bits(0)
        mcache_instr_b := Bits(0)
        mcache_hit := Bits(0)
      }
    }
    //no hit... fetch from external memory
    .otherwise {
      ext_mem_addr := mcache_address - Bits(1)
      ext_mem_cmd := OcpCmd.RD
      ext_mem_burst_cnt := UFix(0)
      mcache_state := size_state
    }
  }
  //fetch size of the required method from external memory address - 1
  when (mcache_state === size_state) {
    when (io.ocp_port.S.Resp === OcpResp.DVA) {
      //init transfer from external memory
      ext_mem_tsize := io.ocp_port.S.Data / Bits(WORD_COUNT)
      ext_mem_fcounter := io.ocp_port.S.Data / Bits(WORD_COUNT)
      ext_mem_burst_cnt := ext_mem_burst_cnt + Bits(1)
      when (ext_mem_burst_cnt >= UFix(BURST_LENGHT - 1)) {
        ext_mem_addr := mcache_address
        ext_mem_cmd := OcpCmd.RD
        ext_mem_burst_cnt := UFix(0)
      }
      //init transfer to on-chip method cache memory
      mcachemem_wtag := Bits(1)
      mcachemem_w_data := io.ocp_port.S.Data / Bits(WORD_COUNT) //write size to mcachemem for LRU tagfield
      mcachemem_address := mcache_address //write base address to mcachemem for tagfield
      mcache_state := transfer_state
    }
  }

  //transfer/fetch method to the cache
  when (mcache_state === transfer_state) {
    when (ext_mem_fcounter > Bits(0)) {
      when (io.ocp_port.S.Resp === OcpResp.DVA) {
        ext_mem_fcounter := ext_mem_fcounter - Bits(1)
        ext_mem_burst_cnt := ext_mem_burst_cnt + Bits(1)
        when (ext_mem_fcounter > Bits(1)) {
          //fetch next address from external memory
          when (ext_mem_burst_cnt >= UFix(BURST_LENGHT - 1)) {
            ext_mem_cmd := OcpCmd.RD
            ext_mem_addr := mcache_address + (ext_mem_tsize - ext_mem_fcounter) + Bits(1)
            ext_mem_burst_cnt := UFix(0)
          }
        }
        //write current address to mcache memory
        mcachemem_w_data := io.ocp_port.S.Data
        mcachemem_w_enable := Bits(1)
      }
      mcachemem_address := mcache_address + (ext_mem_tsize - ext_mem_fcounter) //adress = base address + offset
    }
    //restart to idle state
    .otherwise {
      mcachemem_address := mcache_address
      mcache_state := idle_state
    }
  }

  //outputs to mcache memory
  io.mcache_repl_in.address := mcachemem_address
  io.mcache_repl_in.w_enable := mcachemem_w_enable
  io.mcache_repl_in.w_data := mcachemem_w_data
  io.mcache_repl_in.w_tag := mcachemem_wtag
  io.mcache_repl_in.doCallRet := mcachemem_doCallRet //io.mcache_in.doCallRet
  io.mcache_repl_in.callRetBase := io.mcache_in.callRetBase //forwarding the base address... really needed?

  //outputs to fetch stage
  io.mcache_out.instr_a := mcache_instr_a
  io.mcache_out.instr_b := mcache_instr_b
  //io.mcache_out.hit := mcache_hit
  io.ena := mcache_hit

  //output to external memory
  io.ocp_port.M.Addr := ext_mem_addr
  io.ocp_port.M.Cmd := ext_mem_cmd
  io.ocp_port.M.Data := Bits(0)
  io.ocp_port.M.DataByteEn := Bits("b1111")
  io.ocp_port.M.DataValid := Bits(0)

}


// /*
//  memory logic of the method cache
// */
// class MCacheRepl( 
//   method_count : Int = METHOD_COUNT,
//   replacement : Int = FIFO_REPL,
//   block_arrangement : Int = FIXED_SIZE
// ) extends Component {
  
//   val io = new MCacheReplIO()
//   val ram_mcache_even = Mem(MCACHE_SIZE / 2, seqRead = true) {Bits(width = INSTR_WIDTH)}
//   val ram_mcache_odd = Mem(MCACHE_SIZE / 2, seqRead = true) {Bits(width = INSTR_WIDTH)}

//   val mcache_addr_tag = Mem(method_count) {Bits(width = 32)}
//   val mcache_size_tag = Mem(method_count) {Bits(width = METHOD_SIZETAG_WIDTH)}
//   val mcache_pos_tag = Mem(method_count) {Bits(width = METHOD_SIZETAG_WIDTH)}
//   //linked list for LRU replacement
//   val mcache_list_prev = Mem(method_count) {Bits(width = log2Up(method_count))}
//   val mcache_list_next = Mem(method_count) {Bits(width = log2Up(method_count))}
//   //save base block for FIXED block size arrangement
//   val mcache_base_block = Mem(method_count) {Bits(width = METHOD_SIZETAG_WIDTH)}
//   val mcache_base_addr = Mem(method_count) {Bits(width = 32)}
//   //regs should be moved to mcachemem
//   val dout_a = Reg() {Bits(width = INSTR_WIDTH)}
//   val dout_b = Reg() {Bits(width = INSTR_WIDTH)}
//   val dout_hit = Reg() {Bits(width = 1)}
//   //keep track of lru and mru of the list

// //TODO why this doesnt work???? is synthesized away...
// //if (replacement == LRU_REPL) {
//   val lru_tag = Reg(resetVal = Bits(0, width = log2Up(method_count)))
//   val mru_tag = Reg(resetVal = Bits(method_count - 1, width = log2Up(method_count)))
// //}

//   val next_index_tag = Reg(resetVal = Bits(0, width = log2Up(method_count)))
//   val next_replace_tag = Reg(resetVal = Bits(0, width = log2Up(method_count)))
//   val next_replace_pos = Reg(resetVal = Bits(0, width = METHOD_SIZETAG_WIDTH))
//   val free_space = Reg(resetVal = Fix(MCACHE_SIZE))


//   //for splitting up methods if needed
//   val split_mcounter = Reg(resetVal = Bits(0, width = log2Up(method_count)))
//   val split_maddress = Reg(resetVal = Bits(0, width = 32))
//   val split_msize = Reg(resetVal = Bits(0, width = METHOD_SIZETAG_WIDTH))
//   val split_base_tag = Reg(resetVal = Bits(0, width = METHOD_SIZETAG_WIDTH))
//   val split_base_addr = Reg(resetVal = Bits(0, width = 32))

//   //signals
//   val data_even = Bits(width = INSTR_WIDTH)
//   val data_odd = Bits(width = INSTR_WIDTH)

// //TODO:
// //should not need this, list of differences betweeen 3 operating modes!
//   if (replacement == FIFO_REPL) {
//     If (block_arrangement == FIXED_SIZE) {
//       next_index_tag := Bits(0)
//       next_replace_pos := Bits(0)
//       free_space := Fix(0)
//     }
//     else if (block_arrangement == VARIABLE_SIZE) {
//       split_mcounter := Bits(0)
//       split_maddress := Bits(0)
//       split_msize := Bits(0)
//     }
//     split_base_tag := Bits(0)
//     split_base_addr := Bits(0)
//     lru_tag := Bits(0)
//     mru_tag := Bits(0)
//   }
//   else if (replacement == LRU_REPL) {
//     next_index_tag := Bits(0)
//     next_replace_tag := Bits(0)
//     next_replace_pos := Bits(0)
//     free_space := Fix(0)
//   }

//   //init signals
//   data_even := Bits(0)
//   data_odd := Bits(0)

//   //signal of the current tagfield with position and hit result
//   class TagField extends Bundle {
//     val pos = Bits(width = METHOD_SIZETAG_WIDTH)
//     val hit = Bits(width = 1)
//     val tag = Bits(width = 32)
//   }
//   val tag_field = new TagField()
//   tag_field.pos := Bits(0)
//   tag_field.hit := Bits(0)
//   tag_field.tag := Bits(0)

//   //saves the current tagfield position since fetch can only occur on call/return
//   val pos = Reg(resetVal = Bits(0, width = METHOD_SIZETAG_WIDTH))
//   val hit = Reg(resetVal = Bits(0, width = 1))
//   val tag = Reg(resetVal = Bits(0, width = 32))

//   //search the given addr in the tag field
//   def search_tag_addr(addr : Bits, tagfield: TagField) = {
//     for (i <- 0 until method_count) {
//       when ((addr >= mcache_addr_tag(Bits(i))) && (addr < (mcache_addr_tag(Bits(i)) + mcache_size_tag(Bits(i))))) {
//         if (block_arrangement == FIXED_SIZE) {
//           tagfield.pos := Bits(i) * Bits(METHOD_BLOCK_SIZE / 2) // divided by two because even odd memory!
//         }
//         else if (block_arrangement == VARIABLE_SIZE) {
//           tagfield.pos := mcache_pos_tag(Bits(i))
//         }
//         tagfield.hit := Bits(1)
//         tagfield.tag := mcache_addr_tag(Bits(i))
//       }
//     }
//   }

//   //CHECK BLOCK SiZE only for FIXED SiZE when read/write at next block not do it with variable size!
//   //search needed or not
//   when(io.mcache_repl_in.doCallRet === Bits(1) || (check_block_size(io.mcache_repl_in.address - tag) != Bits(0))) {
//     when(io.mcache_repl_in.callRetBase < io.mcache_repl_in.address && io.mcache_repl_in.doCallRet === Bits(1)) {
//       search_tag_addr(io.mcache_repl_in.callRetBase, tag_field)
//     }
//     //basically at start but think we could drop this inserting a call/ret at start in mcache
//     .otherwise {
//       search_tag_addr(io.mcache_repl_in.address, tag_field)
//     }

//     if (replacement == LRU_REPL) {
//       when (tag_field.hit === Bits(1) && io.mcache_repl_in.w_enable === Bits(0)) {
//         update_tag(tag_field.pos / Bits(METHOD_BLOCK_SIZE / 2))
//       }
//     }
//     pos := tag_field.pos
//     hit := tag_field.hit
//     tag := tag_field.tag
//   }
//   .otherwise {
//     tag_field.pos := pos
//     tag_field.hit := hit
//     tag_field.tag := tag
//   }

//   val addr_offset = (io.mcache_repl_in.address - tag_field.tag) //offset between incoming address and base address

// //TODO only for LRU
// //TODO how we can init Memories??? programming init in C?! following only for simulation in chisel!
//   def init_tag_list() = {
//     when (list_init === Bits(0)) {
//       for (i <- 0 until 4) {
//         mcache_list_next(Bits(i)) := Bits(i) - Bits(1)
//         mcache_list_prev(Bits(i)) := Bits(i) + Bits(1)
//       }
//     }
//   }
//   val list_init = Reg(resetVal = Bits(0, width = 1))
//   when (list_init === Bits(0)) {
//     if (replacement == LRU_REPL) {
//       init_tag_list()
//     }
//   }
//   list_init := Bits(1)

//   //update tag field
//   def update_tag(tag : Bits) = {
//     if (replacement == FIFO_REPL) {
//       tag := (tag + Bits(1)) % Bits(method_count)
//     }
//     else if (replacement == LRU_REPL) {
//       when (tag === lru_tag) {
//         lru_tag := mcache_list_prev(tag)
//         mru_tag := tag
//         mcache_list_next(tag) := mru_tag
//         mcache_list_prev(mru_tag) := tag
//         mcache_list_prev(tag) := tag //no previous any more because mru
//       }
//       .elsewhen (tag != mru_tag) {
//         mcache_list_next(mcache_list_prev(tag)) := mcache_list_next(tag)
//         mcache_list_prev(mcache_list_next(tag)) := mcache_list_prev(tag)
//         mcache_list_next(tag) := mru_tag
//         mcache_list_prev(mru_tag) := tag
//         mru_tag := tag
//       }
//     }
//   }

//   //prepare replacement: index the size and the address tag and check the size fits into one block/free size otherwise split up
//   when (io.mcache_repl_in.w_tag === Bits(1)) {

//     if (replacement == FIFO_REPL && block_arrangement == VARIABLE_SIZE) {
//       //enough free space to fill up
//       when (free_space >= io.mcache_repl_in.w_data) {
//         free_space := free_space - io.mcache_repl_in.w_data + mcache_size_tag(next_index_tag)
//       }
//       .otherwise {
//         free_space := free_space + mcache_size_tag(next_replace_tag) - io.mcache_repl_in.w_data
//       }

//       next_replace_pos := (next_replace_pos + io.mcache_repl_in.w_data) % Bits(MCACHE_SIZE)
//       mcache_pos_tag(next_index_tag) := next_replace_pos
//       mcache_size_tag(next_index_tag) := io.mcache_repl_in.w_data
//       mcache_addr_tag(next_index_tag) := io.mcache_repl_in.address

//       pos := next_replace_pos
//       hit := Bits(1)
//       tag := io.mcache_repl_in.address

//       update_tag(next_index_tag)
//     }

//     if (block_arrangement == FIXED_SIZE) {
//       //everything fine method fits into one block
//       when (check_block_size(io.mcache_repl_in.w_data) === Bits(0)) {
//         if (replacement == FIFO_REPL) {
//           mcache_size_tag(next_replace_tag) := io.mcache_repl_in.w_data
//           mcache_addr_tag(next_replace_tag) := io.mcache_repl_in.address

//           pos := next_replace_tag * Bits(METHOD_BLOCK_SIZE / 2)
//           hit := Bits(1)
//           tag := io.mcache_repl_in.address

//           //when (mcache_addr_tag(mcache_base_block(next_replace_tag)) === mcache_base_addr(next_replace_tag)) {
//           //  mcache_size_tag(mcache_base_block(next_replace_tag)) := Bits(0)
//           //}

//           update_tag(next_replace_tag)

//         }
//         else if (replacement == LRU_REPL) {
//           mcache_size_tag(lru_tag) := io.mcache_repl_in.w_data
//           mcache_addr_tag(lru_tag) := io.mcache_repl_in.address

//           pos := lru_tag * Bits(METHOD_BLOCK_SIZE / 2)
//           hit := Bits(1)
//           tag := io.mcache_repl_in.address

//           when (mcache_addr_tag(mcache_base_block(lru_tag)) === mcache_base_addr(lru_tag)) {
//             mcache_size_tag(mcache_base_block(lru_tag)) := Bits(0)
//           }
//           update_tag(lru_tag)
//         }
//       }
//       //split up in more blocks and set counter
//       .otherwise {
//         split_mcounter := check_block_size(io.mcache_repl_in.w_data)
//         split_maddress := io.mcache_repl_in.address + Bits(METHOD_BLOCK_SIZE)
//         split_msize := io.mcache_repl_in.w_data - Bits(METHOD_BLOCK_SIZE)
//         if (replacement == FIFO_REPL) {
         
//           mcache_size_tag(next_replace_tag) := Bits(METHOD_BLOCK_SIZE)
//           mcache_addr_tag(next_replace_tag) := io.mcache_repl_in.address
//           update_tag(next_replace_tag)

//           //split_base_tag := next_replace_tag
//           //split_base_addr := io.mcachemem_in.address
//           //when (mcache_addr_tag(mcache_base_block(next_replace_tag)) === mcache_base_addr(next_replace_tag)) {
//           //  mcache_size_tag(mcache_base_block(next_replace_tag)) := Bits(0)
//           //}

//           pos := next_replace_tag * Bits(METHOD_BLOCK_SIZE / 2)
//           hit := Bits(1)
//           tag := io.mcache_repl_in.address

//         }
//         else if (replacement == LRU_REPL) {
//           split_base_tag := lru_tag
//           split_base_addr := io.mcache_repl_in.address
//           mcache_size_tag(lru_tag) := Bits(METHOD_BLOCK_SIZE)
//           mcache_addr_tag(lru_tag) := io.mcache_repl_in.address

//           pos := lru_tag * Bits(METHOD_BLOCK_SIZE / 2)
//           hit := Bits(1)
//           tag := io.mcache_repl_in.address

//           when (mcache_addr_tag(mcache_base_block(lru_tag)) === mcache_base_addr(lru_tag)) {
//             mcache_size_tag(mcache_base_block(lru_tag)) := Bits(0)
//           }

//           update_tag(lru_tag)
//         }
//       }
//     }

//   }

//   //more space is needed!
//   if (replacement == FIFO_REPL && block_arrangement == VARIABLE_SIZE) {
//     //the following handles a possible split up if more space is needed
//     when (free_space < Fix(0)) {
//       free_space := free_space + mcache_size_tag(next_replace_tag)
//       mcache_size_tag(next_replace_tag) := Bits(0)
//       update_tag(next_replace_tag)
//     }
//   }

  
//   if (block_arrangement == FIXED_SIZE) {
//     when (split_mcounter != Bits(0)) {
//       //one more block is needed
//       when (check_block_size(split_msize) === Bits(0)) {
//         if (replacement == FIFO_REPL) {
//           mcache_size_tag(next_replace_tag) := split_msize
//           mcache_addr_tag(next_replace_tag) := split_maddress

//           //mcache_base_block(next_replace_tag) := split_base_tag
//           //mcache_base_addr(next_replace_tag) := split_base_addr
//           //when (mcache_addr_tag(mcache_base_block(next_replace_tag)) === mcache_base_addr(next_replace_tag)) {
//           //  mcache_size_tag(mcache_base_block(next_replace_tag)) := Bits(0)
//           //}

//           update_tag(next_replace_tag)
//         }
//         else if (replacement == LRU_REPL) {
//           mcache_size_tag(lru_tag) := split_msize
//           mcache_addr_tag(lru_tag) := split_maddress
//           mcache_base_block(lru_tag) := split_base_tag
//           mcache_base_addr(lru_tag) := split_base_addr

//           when (mcache_addr_tag(mcache_base_block(lru_tag)) === mcache_base_addr(lru_tag)) {
//             mcache_size_tag(mcache_base_block(lru_tag)) := Bits(0)
//           }

//           update_tag(lru_tag)
//         }
//       }
//       //split up in even more blocks
//       .otherwise {
//         split_maddress := split_maddress + Bits(METHOD_BLOCK_SIZE)
//         split_msize := split_msize - Bits(METHOD_BLOCK_SIZE)
//         if (replacement == FIFO_REPL) {
//           mcache_size_tag(next_replace_tag) := Bits(METHOD_BLOCK_SIZE)
//           mcache_addr_tag(next_replace_tag) := split_maddress


//           //mcache_base_block(next_replace_tag) := split_base_tag
//           //mcache_base_addr(next_replace_tag) := split_base_addr
//           //when (mcache_addr_tag(mcache_base_block(next_replace_tag)) === mcache_base_addr(next_replace_tag)) {
//           //  mcache_size_tag(mcache_base_block(next_replace_tag)) := Bits(0)
//           //}

//           update_tag(next_replace_tag)
//         }
//         else if (replacement == LRU_REPL) {
//           mcache_size_tag(lru_tag) := Bits(METHOD_BLOCK_SIZE)
//           mcache_addr_tag(lru_tag) := split_maddress
//           mcache_base_block(lru_tag) := split_base_tag
//           mcache_base_addr(lru_tag) := split_base_addr

//           when (mcache_addr_tag(mcache_base_block(lru_tag)) === mcache_base_addr(lru_tag)) {
//             mcache_size_tag(mcache_base_block(lru_tag)) := Bits(0)
//           }

//           update_tag(lru_tag)
//         }
//       }
//       split_mcounter := split_mcounter - Bits(1)
//     }

//   }

//   //write
//   when (io.mcache_repl_in.w_enable) {
//     //TODO: should always be a hit... not used...
//     when (tag_field.hit === Bits(1)) {
//       when (addr_offset(0) === Bits(1)) {
//         ram_mcache_odd(get_address(tag_field.pos, addr_offset)) := io.mcache_repl_in.w_data
//       }
//       .otherwise {
//         ram_mcache_even(get_address(tag_field.pos, addr_offset)) := io.mcache_repl_in.w_data
//       }
//     }
//   }
//   //read
//   .otherwise {
//     when (tag_field.hit === Bits(1)) {
//       data_even := ram_mcache_even(Mux(addr_offset(0), (get_address(tag_field.pos, addr_offset) + Bits(1)), get_address(tag_field.pos, addr_offset)))
//       data_odd := ram_mcache_odd(get_address(tag_field.pos, addr_offset))
//     }
//     dout_hit := tag_field.hit
//     dout_a := Mux(addr_offset(0), data_odd, data_even) //instr_a must be set here because instr_a and b depend on offset
//     dout_b := Mux(addr_offset(0), data_even, data_odd) //instr_b
//   }

//   io.mcache_repl_out.instr_a := dout_a
//   io.mcache_repl_out.instr_b := dout_b 
//   io.mcache_repl_out.hit := dout_hit
// }
