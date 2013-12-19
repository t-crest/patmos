/*
 Instruction Cache - Direct Mapped
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

object IConstants {

  val ICACHE_SIZE = 4096 //in bytes
  val ICACHE_WORD_SIZE = ICACHE_SIZE / 4
  val ICACHE_SIZE_WIDTH = log2Up(ICACHE_WORD_SIZE)
  val WORD_COUNT = 16
  val BLOCK_COUNT = ICACHE_WORD_SIZE / WORD_COUNT
  val WORD_COUNT_WIDTH = log2Up(WORD_COUNT)
  val BLOCK_COUNT_WIDTH = log2Up(BLOCK_COUNT)
  val VALIDBIT_FIELD_SIZE = 1  //could be enlarged like in leon where blocks are devided in subblocks?!
  val TAG_FIELD_SIZE = (32 - BLOCK_COUNT_WIDTH - WORD_COUNT_WIDTH)
  val TAG_FIELD_HIGH = 31
  val TAG_FIELD_LOW = TAG_FIELD_HIGH - TAG_FIELD_SIZE + 1
  val INDEX_FIELD_HIGH = TAG_FIELD_LOW - 1
  val INDEX_FIELD_LOW = WORD_COUNT_WIDTH
  val INDEX_FIELD_SIZE = INDEX_FIELD_HIGH - INDEX_FIELD_LOW + 1
  val OFFSET_SIZE = 0 //could be added in case to address some subbytes in the block

}

class ICacheIO extends Bundle() {
  val ena_out = Bool(OUTPUT)
  val ena_in = Bool(INPUT)
  val femcache = new FeMCache().asInput
  val exmcache = new ExMCache().asInput
  val mcachefe = new MCacheFe().asOutput
  val ocp_port = new OcpBurstMasterPort(EXTMEM_ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
}
class ICacheCtrlIO extends Bundle() {
  val ena_in = Bool(INPUT)
  val fetch_ena = Bool(OUTPUT)
  val icache_ctrlrepl = new ICacheCtrlRepl().asOutput
  val icache_replctrl = new ICacheReplCtrl().asInput
  val feicache = new FeMCache().asInput
  val exicache = new ExMCache().asInput
  val ocp_port = new OcpBurstMasterPort(EXTMEM_ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
}
class ICacheReplIO extends Bundle() {
  val ena_in = Bool(INPUT)
  val hit_ena = Bool(OUTPUT)
  val exicache = new ExMCache().asInput
  val icachefe = new MCacheFe().asOutput
  val icache_ctrlrepl = new ICacheCtrlRepl().asInput
  val icache_replctrl = new ICacheReplCtrl().asOutput
  val icachemem_in = new ICacheMemIn().asOutput
  val icachemem_out = new ICacheMemOut().asInput
}
class ICacheMemIO extends Bundle() {
  val icachemem_in = new ICacheMemIn().asInput
  val icachemem_out = new ICacheMemOut().asOutput
}
class ICacheCtrlRepl extends Bundle() {
  val w_enable = Bool()
  val w_data = Bits(width = INSTR_WIDTH)
  val w_addr = Bits(width = ADDR_WIDTH)
  val w_tag = Bool()
  val address_even = Bits(width = 32) //ICACHE_SIZE_WIDTH
  val address_odd = Bits(width = 32)
  val instr_stall = Bool()
}
class ICacheReplCtrl extends Bundle() {
  val hit_instr_a = Bool()
  val hit_instr_b = Bool()
}
class ICacheMemIn extends Bundle() {
  val w_ena = Bool()
  val w_data = Bits(width = DATA_WIDTH)
  val w_addr = Bits(width = INDEX_FIELD_SIZE + WORD_COUNT_WIDTH)
  val address_odd = Bits(width = INDEX_FIELD_SIZE + WORD_COUNT_WIDTH)
}
class ICacheMemOut extends Bundle() {
  val instr_a = Bits(width = INSTR_WIDTH)
  val instr_b = Bits(width = INSTR_WIDTH)
}


/*
 ICache: Top Level Class for the Instruction Cache
 */
class ICache() extends Module {
  val io = new ICacheIO()
  //generate submodules of instruction cache
  val icachectrl = Module(new ICacheCtrl())
  val icacherepl = Module(new ICacheReplDm())
  val icachemem = Module(new ICacheMem())
  //connect submodules of instruction cache
  icachectrl.io.icache_ctrlrepl <> icacherepl.io.icache_ctrlrepl
  icachectrl.io.feicache <> io.femcache
  icachectrl.io.exicache <> io.exmcache
  icachectrl.io.ocp_port <> io.ocp_port
  //connect inputs to instruction cache repl unit
  icacherepl.io.exicache <> io.exmcache
  icacherepl.io.icachefe <> io.mcachefe
  icacherepl.io.icache_replctrl <> icachectrl.io.icache_replctrl
  //connect repl unit to on chip memory
  icacherepl.io.icachemem_in <> icachemem.io.icachemem_in
  icacherepl.io.icachemem_out <> icachemem.io.icachemem_out
  //connect enables
  icachectrl.io.ena_in <> io.ena_in
  icacherepl.io.ena_in <> io.ena_in
  //output enable depending on hit/miss/fetch
  io.ena_out := icachectrl.io.fetch_ena & icacherepl.io.hit_ena
}

/*
 ICacheMem Class: On-Chip Instruction Cache Memory
 */
class ICacheMem extends Module {
  val io = new ICacheMemIO()
 
  val ram_icache = Mem(Bits(width = INSTR_WIDTH), ICACHE_WORD_SIZE)

  when (io.icachemem_in.w_ena) { 
    ram_icache(io.icachemem_in.w_addr) := io.icachemem_in.w_data 
  }

  val addrReg = Reg(next = io.icachemem_in.address_odd)
  io.icachemem_out.instr_a := ram_icache(addrReg)
  io.icachemem_out.instr_b := ram_icache(addrReg + Bits(1))

}

/*
 Least Recently Used Replacement Class
 */
class ICacheReplLru extends Module {
  val io = new ICacheReplIO()

  /*
   add LRU replacement policy here 
   */

}

/*
 Direct Mapped Replacement Class
 */
class ICacheReplDm() extends Module {
  val io = new ICacheReplIO()

  //reserve memory for the instruction cache tag field containing valid bit and address tag
  val icache_tag_mem = Mem(Bits(width = TAG_FIELD_SIZE + VALIDBIT_FIELD_SIZE), BLOCK_COUNT)
  val tout = Reg(init = Bits(0, width = TAG_FIELD_SIZE + VALIDBIT_FIELD_SIZE))
  val tout2 = Reg(init = Bits(0, width = TAG_FIELD_SIZE + VALIDBIT_FIELD_SIZE))

  //variables when call/return occurs
  val hit_instr_a = Bool()
  val hit_instr_b = Bool()
  val callRetBaseReg = Reg(init = UInt(1, DATA_WIDTH))
  val callAddrReg = Reg(init = UInt(1, DATA_WIDTH))
  val selIspmReg = Reg(init = Bool(false))
  val selICacheReg = Reg(init = Bool(false))

  val addrIndex = io.icache_ctrlrepl.address_odd(INDEX_FIELD_HIGH, INDEX_FIELD_LOW)
  val addrIndex2 = (io.icache_ctrlrepl.address_odd + Bits(1))(INDEX_FIELD_HIGH, INDEX_FIELD_LOW)
  val addrTag = io.icache_ctrlrepl.address_odd(TAG_FIELD_HIGH, TAG_FIELD_LOW)
  val addrTag2 = (io.icache_ctrlrepl.address_odd + Bits(1))(TAG_FIELD_HIGH, TAG_FIELD_LOW)

  //we need to operate with absolute addresses in a conventional i-cache
  val relBase = Mux(selICacheReg,
                    UInt(0),
                    callRetBaseReg(ISPM_ONE_BIT-3, 0))
  val relPc = callAddrReg + relBase

  val reloc = Mux(selICacheReg,
                  callRetBaseReg,
                  UInt(1 << (ISPM_ONE_BIT - 2)))

  when (io.exicache.doCallRet && io.ena_in) {
    callRetBaseReg := io.exicache.callRetBase
    callAddrReg := io.exicache.callRetAddr
    selIspmReg := io.exicache.callRetBase(DATA_WIDTH - 1,ISPM_ONE_BIT - 2) === Bits(0x1)
    selICacheReg := io.exicache.callRetBase(DATA_WIDTH - 1,15) >= Bits(0x1)
  }

  //check for a hit of both instructions of the address bundle
  hit_instr_a := Bool(true)
  hit_instr_b := Bool(true)
  val addrTagReg = Reg(next = addrTag)
  val addrTagReg2 = Reg(next = addrTag2)
  when (tout(TAG_FIELD_SIZE,1) != addrTagReg || tout(0) != Bits(1)) {
    hit_instr_a := Bool(false)
  }
  when (tout2(TAG_FIELD_SIZE,1) != addrTagReg2 || tout2(0) != Bits(1)) {
    hit_instr_b := Bool(false)
  }

  val wrAddrTag = io.icache_ctrlrepl.w_addr(TAG_FIELD_HIGH,TAG_FIELD_LOW)
  val wrAddrIndex = io.icache_ctrlrepl.w_addr(INDEX_FIELD_HIGH, INDEX_FIELD_LOW)
  //update tag field when new write occurs
  when (io.icache_ctrlrepl.w_tag) {
    icache_tag_mem(wrAddrIndex) := Cat(wrAddrTag, Bits(1))
  }
  .otherwise {
    tout := icache_tag_mem(addrIndex)
    tout2 := icache_tag_mem(addrIndex2)
  }

  //outputs to icache memory
  io.icachemem_in.w_ena := io.icache_ctrlrepl.w_enable
  io.icachemem_in.w_data := io.icache_ctrlrepl.w_data
  io.icachemem_in.w_addr := (io.icache_ctrlrepl.w_addr)(ICACHE_SIZE_WIDTH-1,0) //??? INDEX_FIELD_HIGH?
  io.icachemem_in.address_odd := (io.icache_ctrlrepl.address_odd)(ICACHE_SIZE_WIDTH-1,0) //??? INDEX_FIELD_HIGH?
  io.icachefe.instr_a := io.icachemem_out.instr_a
  io.icachefe.instr_b := io.icachemem_out.instr_b
  io.icachefe.relBase := relBase
  io.icachefe.relPc := relPc
  io.icachefe.reloc := reloc
  io.icachefe.mem_sel := Cat(selIspmReg, selICacheReg)
  //hit/miss return
  io.icache_replctrl.hit_instr_a := hit_instr_a
  io.icache_replctrl.hit_instr_b := hit_instr_b
  io.hit_ena := (hit_instr_a && hit_instr_b)

}

/*
 Instruction Cache Control Class: handles block transfer from external Memory to the I-Cache
 */
class ICacheCtrl() extends Module {
  val io = new ICacheCtrlIO()

  //fsm state variables
  val init_state :: idle_state :: fetch_state :: transfer_state :: Nil = Enum(UInt(), 5)
  val icache_state = Reg(init = init_state)

  //signal for replacement unit
  val icachemem_address = Bits(width = ADDR_WIDTH)
  val icachemem_w_data = Bits(width = DATA_WIDTH)
  val icachemem_w_tag = Bool() //signalizes the transfer of begin of a write
  val icachemem_w_addr = Bits(width = ADDR_WIDTH)
  val icachemem_w_enable = Bool()
  //signals for external memory
  val ext_mem_cmd = Bits(width = 3)
  val ext_mem_addr = Bits(width = EXTMEM_ADDR_WIDTH)
  val ext_mem_fcounter = Reg(init = Bits(0, width = ICACHE_SIZE_WIDTH))
  val ext_mem_burst_cnt = Reg(init = UInt(0, width = log2Up(BURST_LENGTH)))

  //input output registers
  val addrReg = Reg(init = Bits(0, width = 32))
  val wenaReg = Reg(init = Bool(false))
  val callRetBaseReg = Reg(init = Bits(0, width = ADDR_WIDTH))
  val ocpSlaveReg = Reg(next = io.ocp_port.S)

  //should not be needed instead a absolut pc should be used
  val absAddr = io.feicache.address_odd + callRetBaseReg
  val absCallAddr = io.feicache.address_odd + io.feicache.callRetBase
  //address for the entire block
  val absFetchAddr = Cat(addrReg(31,WORD_COUNT_WIDTH), Bits(0)(WORD_COUNT_WIDTH-1,0))

  //init signals
  icachemem_address := absAddr
  icachemem_w_data := Bits(0)
  icachemem_w_tag := Bool(false)
  icachemem_w_enable := Bool(false)
  icachemem_w_addr := Bits(0)
  ext_mem_cmd := OcpCmd.IDLE
  ext_mem_addr := Bits(0)

  when (icache_state === init_state) {
    when(io.feicache.request) {
      icache_state := idle_state
    }
  }
  //check hit/miss
  when (icache_state === idle_state) {
    when (io.icache_replctrl.hit_instr_a && io.icache_replctrl.hit_instr_b) {
      icachemem_address := Mux(io.feicache.doCallRet, absCallAddr, absAddr)
      //not used if pc is absolut
      when (io.feicache.doCallRet && io.ena_in) {
        callRetBaseReg := io.feicache.callRetBase
      }
    }
    .otherwise {
      wenaReg := Bool(true)
      addrReg := Mux(io.icache_replctrl.hit_instr_b, io.feicache.address_odd, io.feicache.address_odd + Bits(1)) + callRetBaseReg
      icache_state := fetch_state
    }
  }
  //prepare fetch from ext memory
  when (icache_state === fetch_state) {
    ext_mem_addr := absFetchAddr
    ext_mem_cmd := OcpCmd.RD
    ext_mem_burst_cnt := UInt(0)
    ext_mem_fcounter := UInt(0)
    icachemem_w_tag := Bool(true)
    icachemem_w_addr := absFetchAddr
    icache_state := transfer_state
  }
  //transfer/fetch cache block
  when (icache_state === transfer_state) {
    when (ext_mem_fcounter < UInt(WORD_COUNT)) {
      when (ocpSlaveReg.Resp === OcpResp.DVA) {
        ext_mem_fcounter := ext_mem_fcounter + Bits(1)
        ext_mem_burst_cnt := ext_mem_burst_cnt + Bits(1)
        when(ext_mem_fcounter < UInt(WORD_COUNT-1)) {
          //fetch next address from external memory
          when (ext_mem_burst_cnt >= UInt(BURST_LENGTH - 1)) {
            ext_mem_cmd := OcpCmd.RD
            ext_mem_addr := Cat(addrReg(31,WORD_COUNT_WIDTH), Bits(0)(WORD_COUNT_WIDTH-1,0)) + ext_mem_fcounter + Bits(1)
            ext_mem_burst_cnt := UInt(0)
          }
        }
        //write current address to icache memory
        icachemem_w_data := ocpSlaveReg.Data
        icachemem_w_enable := Bool(true)
      }
      icachemem_w_addr := Cat(addrReg(31,WORD_COUNT_WIDTH), Bits(0)(WORD_COUNT_WIDTH-1,0)) + ext_mem_fcounter
    }
    //restart to idle state
    .otherwise {
      icache_state := idle_state
      wenaReg := Bool(false)
    }
  }
  
  //outputs to instruction cache memory
  io.icache_ctrlrepl.address_odd := icachemem_address
  io.icache_ctrlrepl.w_enable := icachemem_w_enable
  io.icache_ctrlrepl.w_data := icachemem_w_data
  io.icache_ctrlrepl.w_addr := icachemem_w_addr
  io.icache_ctrlrepl.w_tag := icachemem_w_tag

  io.fetch_ena := !wenaReg

  //output to external memory
  io.ocp_port.M.Addr := Cat(ext_mem_addr, Bits("b00"))
  io.ocp_port.M.Cmd := ext_mem_cmd
  io.ocp_port.M.Data := Bits(0)
  io.ocp_port.M.DataByteEn := Bits("b1111")
  io.ocp_port.M.DataValid := Bits(0)

}
