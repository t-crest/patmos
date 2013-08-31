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

class FeMCache extends Bundle() {
  val address = Bits(width = ADDR_WIDTH) 
  val request = Bits(width = 1)
}
class ExMCache() extends Bundle() {
  val doCallRet = Bool()
  val callRetBase = UFix(width = DATA_WIDTH)
  val callRetAddr = UFix(width = DATA_WIDTH)
}
class MCacheFe extends Bundle() {
  val instr_a = Bits(width = 32)
  val instr_b = Bits(width = 32)
  val pos_offset = Bits(width = log2Up(MCACHE_SIZE))
  val mem_sel = Bits(width = 2)
}
class MCacheIO extends Bundle() {
  val ena = Bool(OUTPUT)
  val femcache = new FeMCache().asInput
  val exmcache = new ExMCache().asInput
  val mcachefe = new MCacheFe().asOutput
  val ocp_port = new OcpBurstMasterPort(19, DATA_WIDTH, BURST_LENGHT)
}
class MCacheCtrlIO extends Bundle() {
  val ena = Bits(OUTPUT, width = 1)
  val mcache_ctrlrepl = new MCacheCtrlRepl().asOutput
  val mcache_replctrl = new MCacheReplCtrl().asInput
  val femcache = new FeMCache().asInput
  val exmcache = new ExMCache().asInput
  val ocp_port = new OcpBurstMasterPort(19, DATA_WIDTH, BURST_LENGHT)
}
class MCacheCtrlRepl extends Bundle() {
  val w_enable = Bits(width = 1)
  val w_data = Bits(width = 32)
  val w_addr = Bits(width = 32)
  val w_tag = Bits(width = 1)
  val address = Bits(width = 32)
}
class MCacheReplCtrl extends Bundle() {
  val hit = Bits(width = 1)
  val pos_offset = Bits(width = log2Up(MCACHE_SIZE))
}
class MCacheReplIO extends Bundle() {
  val exmcache = new ExMCache().asInput
  val mcachefe = new MCacheFe().asOutput
  val mcache_ctrlrepl = new MCacheCtrlRepl().asInput
  val mcache_replctrl = new MCacheReplCtrl().asOutput
  val mcachemem_in = new MCacheMemIn().asOutput
  val mcachemem_out = new MCacheMemOut().asInput
}
class MCacheMemIn extends Bundle() {
  val w_even = Bits(width = 1)
  val w_odd = Bits(width = 1)
  val w_data = Bits(width = DATA_WIDTH)
  val w_addr = Bits(width = (log2Up(MCACHE_SIZE / 2)))
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
  //connect inputs to method cache ctrl unit
  mcachectrl.io.ena <> io.ena
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

  when (io.mcachemem_in.w_even) {ram_mcache_even(io.mcachemem_in.w_addr) := io.mcachemem_in.w_data}
  .otherwise {(dout_even := ram_mcache_even(io.mcachemem_in.addr_even))}

  when (io.mcachemem_in.w_odd) {ram_mcache_odd(io.mcachemem_in.w_addr) := io.mcachemem_in.w_data}
  .otherwise {(dout_odd := ram_mcache_odd(io.mcachemem_in.addr_odd))}

  io.mcachemem_out.instr_even := dout_even
  io.mcachemem_out.instr_odd := dout_odd
}

/*
 MCacheReplFifo: Class controlls a FIFO replacement strategie including tag-memory to keep history of methods in cache
 */
class MCacheReplFifo(method_count : Int = METHOD_COUNT) extends Component {
  val io = new MCacheReplIO()

  //tag field tables  for reading tag memory
  val mcache_addr_vec = { Vec(method_count) { Reg(resetVal = Bits(0, width = ADDR_WIDTH)) } }
  val mcache_size_vec = { Vec(method_count) { Reg(resetVal = Bits(0, width = METHOD_SIZETAG_WIDTH)) } }
  val mcache_pos_vec = { Vec(method_count) { Reg(resetVal = Bits(0, width = METHOD_SIZETAG_WIDTH)) } }
  //registers to save current replacement status
  val next_index_tag = Reg(resetVal = Bits(0, width = log2Up(method_count)))
  val next_replace_tag = Reg(resetVal = Bits(0, width = log2Up(method_count)))
  val next_replace_pos = Reg(resetVal = Bits(0, width = METHOD_SIZETAG_WIDTH+1))
  val free_space = Reg(resetVal = Fix(MCACHE_SIZE))
  //variables when call/return occurs to check tag field
  val posReg = Reg(resetVal = Bits(0))
  val hitReg = Reg(resetVal = Bits(0))
  val tag_field_size = Reg(resetVal = Bits(0))

  val pos_offset = Bits()
  pos_offset := Bits(0)

  val callRetBaseReg = Reg(resetVal = UFix(1, DATA_WIDTH))
  val callAddrReg = Reg(resetVal = UFix(1, DATA_WIDTH))
  val selBase = Mux(io.exmcache.doCallRet, io.exmcache.callRetBase, callRetBaseReg)
  val selIspm = selBase(DATA_WIDTH - 1,ISPM_ONE_BIT - 2) === Bits(0x1)
  val selMCache = selBase(DATA_WIDTH - 1,15) >= Bits(0x1)

  // val pos_tmp = { Vec(method_count) {Bits(width = METHOD_SIZETAG_WIDTH)} }
  // for (i <- 0 until method_count) {
  //   pos_tmp(i) := Bits(0)
  // }
  /*how is this done time effective, is the for loop building parallel elements right???*/
  //read from tag memory on call/return to check if method is in the cache
  when (io.exmcache.doCallRet) {

    callRetBaseReg := io.exmcache.callRetBase
    callAddrReg := io.exmcache.callRetAddr

    when (selMCache) {
      hitReg := Bits(0)
      for (i <- 0 until method_count) {
        when (io.exmcache.callRetBase === mcache_addr_vec(i)) {
          hitReg := Bits(1)
          posReg := mcache_pos_vec(i)
        }
      }
    }
    //why the following does not work to OR all pos_tmp(i)???
    //for (i <- 0 until method_count) {
      //posReg := (posReg | pos_tmp(i))
    //}
  }

  when (hitReg === Bits(0)) {
    posReg := Cat(next_replace_pos(11,1), Bits("b1"))
    pos_offset := Mux(selMCache, 
      callAddrReg - callRetBaseReg + Cat(next_replace_pos(11,1), Bits("b1")), 
      callAddrReg(ISPM_ONE_BIT - 3,0))
  }
  .otherwise {
    pos_offset := Mux(selMCache, 
      callAddrReg - callRetBaseReg + posReg, 
      callAddrReg(ISPM_ONE_BIT - 3,0))
  }

  //insert new tags
  when (io.mcache_ctrlrepl.w_tag) {

    hitReg := Bits(1)

    //update free space
    free_space := free_space - io.mcache_ctrlrepl.w_data + tag_field_size
    when (free_space < io.mcache_ctrlrepl.w_data) {
      tag_field_size := mcache_size_vec(next_replace_tag)
    }
    //update tag fields
    mcache_pos_vec(next_index_tag) := Cat(next_replace_pos(11,1), Bits("b1"))
    mcache_size_vec(next_index_tag) := io.mcache_ctrlrepl.w_data
    mcache_addr_vec(next_index_tag) := io.mcache_ctrlrepl.w_addr
    //update pointers
    next_replace_pos := (next_replace_pos + io.mcache_ctrlrepl.w_data)(11,0) //lenght cast needed?
    next_index_tag := Mux(next_index_tag === Bits(method_count - 1), Bits(0), next_index_tag + Bits(1))
    //update replace tag since it is covered by index tag at  the next replacement
    when((next_index_tag + Bits(1)) === next_replace_tag) {
      next_replace_tag := Mux(next_replace_tag === Bits(method_count - 1), Bits(0), next_replace_tag + Bits(1))
    }
  }

  //free new space if still needed
  when (free_space < Fix(0)) {
    free_space := free_space + tag_field_size
    tag_field_size := mcache_size_vec(next_replace_tag)
    mcache_size_vec(next_replace_tag) := Bits(0)
    next_replace_tag := Mux(next_replace_tag === Bits(3), Bits(0), next_replace_tag + Bits(1))
  }

  val wr_parity = ~io.mcache_ctrlrepl.w_addr(0)
  val mcachemem_w_address = (posReg + io.mcache_ctrlrepl.w_addr)(11,1)

  val rd_parity = io.mcache_ctrlrepl.address(0)
  val addr_parity_reg = Reg(rd_parity)
  val mcachemem_in_address = (io.mcache_ctrlrepl.address)(11,1)

  io.mcachemem_in.w_even := Mux(wr_parity, Bits(0), io.mcache_ctrlrepl.w_enable)
  io.mcachemem_in.w_odd := Mux(wr_parity, io.mcache_ctrlrepl.w_enable, Bits(0))
  io.mcachemem_in.w_data := io.mcache_ctrlrepl.w_data
  io.mcachemem_in.w_addr := mcachemem_w_address
  io.mcachemem_in.addr_even := Mux(rd_parity, mcachemem_in_address + Bits(1), mcachemem_in_address)
  io.mcachemem_in.addr_odd := mcachemem_in_address

  io.mcachefe.instr_a := Mux(addr_parity_reg, io.mcachemem_out.instr_odd, io.mcachemem_out.instr_even)
  io.mcachefe.instr_b := Mux(addr_parity_reg, io.mcachemem_out.instr_even, io.mcachemem_out.instr_odd)
  io.mcachefe.pos_offset := pos_offset
  io.mcachefe.mem_sel := Cat(selIspm, selMCache)

  io.mcache_replctrl.hit := hitReg
  io.mcache_replctrl.pos_offset := pos_offset //Mux(hitReg, posReg, Cat(next_replace_pos(11,1), Bits("b1")))
}


/*
 MCacheCtrl Class: Main Class of Method Cache, implements the State Machine and handles the R/W/Fetch of Cache and External Memory
 */
class MCacheCtrl() extends Component {
  val io = new MCacheCtrlIO()

  //fsm state variables
  val init_state :: idle_state :: size_state :: transfer_state :: restart_state :: Nil = Enum(5){ UFix() }
  val mcache_state = Reg(resetVal = init_state)
  //signals for method cache memory (mcache_repl)
  val mcachemem_address = Bits(width = 32)
  val mcachemem_w_data = Bits(width = DATA_WIDTH)
  val mcachemem_w_tag = Bits(width = 1) //signalizes the transfer of begin of a write
  val mcachemem_w_addr = Bits(width = 32)
  val mcachemem_w_enable = Bits(width = 1)
  //signals for fetch stage
  val mcache_hit = Bits(width = 1)
  //signals for external memory
  val ext_mem_cmd = Bits(width = 3)
  val ext_mem_addr = Bits(width = 23)
  val ext_mem_tsize = Reg(resetVal = Bits(0, width = 32))
  val ext_mem_fcounter = Reg(resetVal = Bits(0, width = 32))
  val ext_mem_burst_cnt = Reg(resetVal = UFix(0, width = log2Up(BURST_LENGHT)))
  //input registers
  val callRetBaseReg = Reg(resetVal = Bits(0, width = 32))
  val msize_addr = callRetBaseReg - Bits(1)
  val addrReg = Reg(resetVal = Bits(1))

  //init signals
  mcachemem_address := addrReg //io.femcache.address
  mcachemem_w_data := Bits(0)
  mcachemem_w_tag := Bits(0)
  mcachemem_w_enable := Bits(0)
  mcachemem_w_addr := Bits(0)
  mcache_hit := Bits(0)
  ext_mem_cmd := OcpCmd.IDLE
  ext_mem_addr := Bits(0)

  when (io.exmcache.doCallRet) {
    callRetBaseReg := io.exmcache.callRetBase // use callret to save base address for next cycle
    addrReg := io.femcache.address
  }

  val hit_reg = Reg(io.mcache_replctrl.hit)

  //init state needs to fetch at program counter - 1 the first size of method block
  when (mcache_state === init_state) {
    mcache_hit := Bits(1) //do not stall for default till mcache is selected
    when(io.femcache.request) {
      mcache_state := idle_state
      mcache_hit := Bits(0) //stall to fetch the first method into the cache
    }
  }
  //check if instruction is available
  when (mcache_state === idle_state) {
    mcache_hit := io.mcache_replctrl.hit
    when(io.mcache_replctrl.hit === Bits(1)) {
      mcachemem_address := io.femcache.address
    }
    //no hit... fetch from external memory
    .otherwise {
      ext_mem_addr := Cat(msize_addr(31,2), Bits("b00"))
      //ext_mem_addr := msize_addr
      ext_mem_cmd := OcpCmd.RD
      ext_mem_burst_cnt := UFix(0)
      mcache_state := size_state
    }
  }
  //fetch size of the required method from external memory address - 1
  when (mcache_state === size_state) {
    when (io.ocp_port.S.Resp === OcpResp.DVA) {
      ext_mem_burst_cnt := ext_mem_burst_cnt + Bits(1)
      when (ext_mem_burst_cnt === msize_addr(1,0)) {
      //init transfer from external memory
        ext_mem_tsize := io.ocp_port.S.Data(31,2)
        ext_mem_fcounter := Bits(0) //start to write to cache with offset 0
        when (ext_mem_burst_cnt >= UFix(BURST_LENGHT - 1)) {
          ext_mem_addr := callRetBaseReg
          ext_mem_cmd := OcpCmd.RD
          ext_mem_burst_cnt := UFix(0)
        }
        //init transfer to on-chip method cache memory
        mcachemem_w_tag := Bits(1)
        mcachemem_w_data := io.ocp_port.S.Data(31,2) //write size to mcachemem for LRU tagfield
        mcachemem_w_addr := callRetBaseReg //write base address to mcachemem for tagfield
        mcache_state := transfer_state
      }
    }
  }

  //transfer/fetch method to the cache
  when (mcache_state === transfer_state) {
    when (ext_mem_fcounter < ext_mem_tsize) {
      when (io.ocp_port.S.Resp === OcpResp.DVA) {
        ext_mem_fcounter := ext_mem_fcounter + Bits(1)
        ext_mem_burst_cnt := ext_mem_burst_cnt + Bits(1)
        when(ext_mem_fcounter < ext_mem_tsize - Bits(1)) {
          //fetch next address from external memory
          when (ext_mem_burst_cnt >= UFix(BURST_LENGHT - 1)) {
            ext_mem_cmd := OcpCmd.RD
            ext_mem_addr := callRetBaseReg + ext_mem_fcounter + Bits(1) //need +1 because start fetching with the size of method
            ext_mem_burst_cnt := UFix(0)
          }
        }
        //write current address to mcache memory
        mcachemem_w_data := io.ocp_port.S.Data
        mcachemem_w_enable := Bits(1)
      }
      mcachemem_w_addr := ext_mem_fcounter
    }
    //restart to idle state
    .otherwise {
      mcache_state := idle_state
      //mcachemem_address := io.femcache.address //- Bits(1)
    }
  }

  //outputs to mcache memory
  io.mcache_ctrlrepl.address := mcachemem_address
  io.mcache_ctrlrepl.w_enable := mcachemem_w_enable
  io.mcache_ctrlrepl.w_data := mcachemem_w_data
  io.mcache_ctrlrepl.w_addr := mcachemem_w_addr
  io.mcache_ctrlrepl.w_tag := mcachemem_w_tag

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



//registers to write the next cycle after call/return
  //val tag_rd_ena = Reg(resetVal = Bits(0, width = 1))
  //val tag_wr_ena = Reg(resetVal = Bits(0, width = 1))
  //val tag_wr_size = Reg(resetVal = Bits(0, width = METHOD_SIZETAG_WIDTH+1))
  //val addr_reg = Reg(resetVal = Bits(0, width = ADDR_WIDTH))
  //init signals and registers
  //tag_wr_ena := io.mcache_repl_in.w_tag
  //addr_reg := io.mcache_repl_in.base_addr
  //tag_rd_ena := io.mcache_repl_in.doCallRet
  //search tag in list
  //when (tag_rd_ena) {
    // for (i <- 0 until method_count) {
    //   //when ((addr_reg >= mcache_addr_vec(i)) && (addr_reg < (mcache_addr_vec(i) + mcache_size_vec(i)))) {
    //   when (addr_reg === mcache_addr_vec(i)) {
    //     tag_field.pos := mcache_pos_vec(i)
    //     tag_field.hit := Bits(1)
    //     tag_field.tag := mcache_addr_vec(i)
    //     pos := mcache_pos_vec(i)
    //     hit := Bits(1)
    //     tag := mcache_addr_vec(i)
    //   }
    // }
  //}
  //need to save size to write and start reading new size from tag memory when starting new transfer
  // when(io.mcache_repl_in.w_tag) {
  //   tag_wr_size := io.mcache_repl_in.w_data //size of the new method
  //   tag_field_size := mcache_size_vec(next_index_tag)
  // }
  //write to tag memory
  // when (tag_wr_ena) {
  //   //update free space and tags
  //   free_space := free_space - tag_wr_size + tag_field_size
  //   when (free_space < tag_wr_size) {
  //     tag_field_size := mcache_size_vec(next_replace_tag)
  //   }
  //   //add new tags to memory
  //   mcache_pos_vec(next_index_tag) := next_replace_pos(11,0) //current position before update
  //   mcache_size_vec(next_index_tag) := tag_wr_size(11,0) //size of method
  //   mcache_addr_vec(next_index_tag) := addr_reg //address of method
  //   //update tags and fields
  //   next_replace_pos := ((next_replace_pos + tag_wr_size) % Bits(MCACHE_SIZE))
  //   //pos := next_replace_pos //set new position for next replacement
  //   hit := Bits(1) //we have a hit till next call/return because tag is now in the field
  //   //tag_field_pos := next_replace_pos //used!!!
  //   next_index_tag := Mux(next_index_tag === Bits(3), Bits(0), next_index_tag + Bits(1))
  //   //update also replace tag since it is covered by index tag at  the next replacement
  //   when((next_index_tag + Bits(1)) === next_replace_tag) {
  //     next_replace_tag := Mux(next_replace_tag === Bits(3), Bits(0), next_replace_tag + Bits(1))
  //   }
  // }



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
