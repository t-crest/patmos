/*
 Instruction Cache - Direct Mapping, only one instruction per cycle (instr_a)
 Author: Philipp Degasperi (philipp.degasperi@gmail.com)
 */

package patmos

import Chisel._
import Node._
import ExtMemROM._
import IConstants._
import Constants._

import scala.collection.mutable.HashMap
import scala.util.Random
import scala.math


object IConstants {
  //on chip 4KB icache
  val ICACHE_SIZE = 4096 //* 8 //4KB = 2^12*2^3 = 32*1024 = 32768Bit

  //some of these should be generic
  val WORD_COUNT = 4
  val WORD_SIZE = 32 //not needed in bits
  val BLOCK_SIZE = WORD_COUNT * WORD_SIZE //not needed in bits
  val BLOCK_COUNT = ICACHE_SIZE / WORD_COUNT //--BLOCK_SIZE
  val VALIDBIT_FIELD_SIZE = 1  //could be enlarged like in leon where blocks are devided in subblocks?!
  val TAG_FIELD_SIZE = (32 - (8+2)) //32 - ((log2(BLOCK_COUNT) + log2(WORD_COUNT)) //32bit address - index field (8=address bits for block, 2=address bits for words
  val TAG_FIELD_HIGH = 31 //using 32 bit field address
  val TAG_FIELD_LOW = TAG_FIELD_HIGH - TAG_FIELD_SIZE + 1
  val INDEX_FIELD_HIGH = TAG_FIELD_LOW - 1
  val INDEX_FIELD_LOW = OFFSET_SIZE
  val INDEX_FIELD_SIZE = INDEX_FIELD_HIGH - INDEX_FIELD_LOW + 1
  val OFFSET_SIZE = 0 //could be added in case to address some subbytes in the block
  val TAG_CACHE_SIZE =  BLOCK_COUNT  // * (TAG_FIELD_SIZE + VALIDBIT_FIELD_SIZE)
  //for test only onchip double size of icache
  val EXTMEM_SIZE = 2 * ICACHE_SIZE // =32*2048
  //DEBUG INFO
  println("BLOCK_COUNT=" + BLOCK_COUNT)
  println("WORD_COUNT" + WORD_COUNT)
  println("TAG_FIELD_HIGH=" + TAG_FIELD_HIGH)
  println("TAG_FIELD_LOW=" + TAG_FIELD_LOW)
  println("TAG_FIELD_SIZE=" + TAG_FIELD_SIZE)
  println("INDEX_FIELD_LOW" + INDEX_FIELD_HIGH)
  println("INDEX_FIELD_HIGH" + INDEX_FIELD_LOW)
  println("INDEX_FIELD_SIZE" + INDEX_FIELD_SIZE)
  println("OFFSET_SIZE" + OFFSET_SIZE)
  println("TAG_CACHE_SIZE=" + TAG_CACHE_SIZE)
  println("EXT_MEM_SIZE=" + EXTMEM_SIZE)

}

class ICacheIn extends Bundle() {
  val address = Bits(width = 32) //=pc
  val request = Bits(width = 1) //the moment only needed for start
}
class ICacheOut extends Bundle() {
  val data_even = Bits(width = 32)
  val data_odd = Bits(width = 32)
  val hit = Bits(width = 1)
}
class ICacheIO extends Bundle() {
  val icachemem_in = new ICacheMemIn().asOutput
  val icachemem_out = new ICacheMemOut().asInput
  val icache_in = new ICacheIn().asInput
  val icache_out = new ICacheOut().asOutput
  val extmem_in = new ExtMemIn().asOutput
  val extmem_out = new ExtMemOut().asInput
}
class ICacheMemIn extends Bundle() {
  val w_enable = Bits(width = 1)
  val w_data = Bits(width = 32)
  val address = Bits(width = INDEX_FIELD_SIZE + OFFSET_SIZE)
  val tag = Bits(width = TAG_FIELD_SIZE)
}
class ICacheMemOut extends Bundle() {
  val data_even = Bits(width = 32)
  val data_odd = Bits(width = 32)
  val tag = Bits(width = TAG_FIELD_SIZE)
  val valid = Bits(width = 1)
}
class ICacheMemIO extends Bundle() {
  val icachemem_in = new ICacheMemIn().asInput
  val icachemem_out = new ICacheMemOut().asOutput
}

/*
 TODO:
 -add even odd memory same as in method cache...
 -check if we have to fetch twice since we support 64bit...
 */
class ICacheMem extends Component {
  val io = new ICacheMemIO()
  //reserve memory for the instruction cache data
  val ram_icache = Mem(ICACHE_SIZE, seqRead = true) {Bits(width = 32)}
  val dout = Reg() {Bits(width = 64)}
  when (io.icachemem_in.w_enable) { ram_icache(io.icachemem_in.address) := io.icachemem_in.w_data }
  .otherwise { 
    dout := Cat(ram_icache(io.icachemem_in.address + Bits(1)), ram_icache(io.icachemem_in.address))
  }
  io.icachemem_out.data_even := dout(31,0)
  io.icachemem_out.data_odd := dout(63,32)

  //reserve memory for the instruction cache tag field containing valid bit and address tag
  val ram_icachetag = Mem(TAG_CACHE_SIZE, seqRead = true) {Bits(width = TAG_FIELD_SIZE + VALIDBIT_FIELD_SIZE)}
  val tout = Reg() { Bits() }
  val tag_field = Bits(width = TAG_FIELD_SIZE + VALIDBIT_FIELD_SIZE)
  tag_field := Bits(0)
  when (io.icachemem_in.w_enable) {
    tag_field := Cat(io.icachemem_in.tag, Bits(1))
    ram_icachetag(io.icachemem_in.address / UFix(4)) := (tag_field)
  }
  .otherwise { tout := ram_icachetag(io.icachemem_in.address / UFix(4)) }
  io.icachemem_out.tag := tout(TAG_FIELD_SIZE, VALIDBIT_FIELD_SIZE)
  io.icachemem_out.valid := tout(0)
}

class ICache extends Component {
  val io = new ICacheIO()

  //fsm variables
  val init_state :: check_hit_state :: fetch_state :: restart_state :: Nil = Enum(4){ UFix() }
  val icache_state = Reg(resetVal = init_state)

  //signal variables
  val icachemem_address = Bits(width = 12)
  val icache_w_enable = Bits(width = 1)
  val icache_fetched_data = Bits(width = 32)
  val icache_w_tag = Bits(width = TAG_FIELD_SIZE)
  val icache_data_even = Bits(width = 32)
  val icache_data_odd = Bits(width = 32)
  val icache_hit = Bits(width = 1)
  val extmem_fetch = Bits(width = 1)
  val extmem_fetch_address = Bits(width = 32)

  //regs
  val icache_address = Reg(resetVal = Bits(0, width = 32))
  val icache_address_next = Reg(resetVal = Bits(0, width = 32))
  val fword_counter = Reg(resetVal = UFix(0, width = 32))

  //init signals
  icachemem_address := io.icache_in.address(11,0)
  icache_fetched_data := Bits(0)
  icache_w_enable := Bits(0)
  icache_w_tag := Bits(0)
  icache_hit := Bits(0)
  icache_data_even := Bits(0)
  icache_data_odd := Bits(0)
  extmem_fetch := Bits(0)
  extmem_fetch_address := Bits(0)

  //check for a hit
  when ((io.icachemem_out.tag === icache_address(TAG_FIELD_HIGH, TAG_FIELD_LOW)) && (io.icachemem_out.valid === Bits(1))) {
      icache_data_even := io.icachemem_out.data_even
      icache_data_odd := io.icachemem_out.data_odd
      icache_hit := Bits(1)
  }

  //init sets the first address
  when (icache_state === init_state) {
    when (io.icache_in.request === Bits(1)) {
      icache_address := io.icache_in.address
      icache_state := check_hit_state
    }
  }
  when (icache_state === check_hit_state) {
    when (icache_hit) {
      icache_address := io.icache_in.address
    }
    .otherwise {
       icache_address_next := icachemem_address
       icache_state := fetch_state
       extmem_fetch := Bits(1)
       extmem_fetch_address := Cat(icache_address(31,2), Bits("b00"))
    }
  }
  when (icache_state === fetch_state) {
    when (io.extmem_out.ready === Bits(1)) {
      when (fword_counter < UFix(WORD_COUNT)) {
        icache_fetched_data := io.extmem_out.data
        icache_w_tag := icache_address(TAG_FIELD_HIGH, TAG_FIELD_LOW)
        icache_w_enable := Bits(1)
        fword_counter := fword_counter + UFix(1)
        icachemem_address := Cat(icache_address(31,2), Bits("b00")) + fword_counter
      }
      .otherwise {
        fword_counter := UFix(0)
        icache_state := restart_state
        icache_w_enable := Bits(0)
        icachemem_address := icache_address
      }
    }
  }
  when (icache_state === restart_state) {
    icache_state := check_hit_state
    icachemem_address := icache_address_next
    icache_address := icache_address_next
  }
  
  io.icachemem_in.address := icachemem_address
  io.icachemem_in.w_enable := icache_w_enable
  io.icachemem_in.w_data := icache_fetched_data
  io.icachemem_in.tag := icache_w_tag

  io.icache_out.data_even := icache_data_even
  io.icache_out.data_odd := icache_data_odd
  io.icache_out.hit := icache_hit

  io.extmem_in.address := extmem_fetch_address
  io.extmem_in.fetch := extmem_fetch

}


