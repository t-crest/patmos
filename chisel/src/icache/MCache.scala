/*
 Method Cache, N Fixed Method Blocks, LRU-Replacement
 Author: Philipp Degasperi (philipp.degasperi@gmail.com)
 */


//merge package mcache and icache in future
package patmos

import Chisel._
import Node._
import ExtMemROM._
import MConstants._
import Constants._
import MCacheMem._

import scala.collection.mutable.HashMap
import scala.util.Random
import scala.math

object MConstants {
  //on chip 4KB icache
  val MCACHE_SIZE = 4096 / 4 //* 8 //4KB = 2^12*2^3 = 32*1024 = 32768Bit
  val EXTMEM_SIZE = 2 * MCACHE_SIZE // =32*2048
  val METHOD_COUNT = 4
  val METHOD_BLOCK_SIZE = MCACHE_SIZE / METHOD_COUNT
  val METHOD_SIZETAG_WIDTH = log2Up(MCACHE_SIZE)
  val METHOD_COUNT_WIDTH = log2Up(METHOD_COUNT)
  val WORD_COUNT = 4

  //DEBUG INFO
  println("MCACHE_SIZE=" + MCACHE_SIZE)
  println("EXTMEM_SIZE=" + EXTMEM_SIZE)
  println("METHOD_BLOCK_SIZE=" + METHOD_BLOCK_SIZE)
  println("METHOD_COUNT=" + METHOD_COUNT)
  println("METHOD_SIZETAG_WIDTH=" + METHOD_SIZETAG_WIDTH)
  println("METHOD_COUNT_WIDTH=" + METHOD_COUNT_WIDTH)
  println("WORD_COUNT=" + WORD_COUNT)
}

class MCacheIn extends Bundle() {
  val address = Bits(width = 32) //=pc_next
  val request = Bits(width = 1) //should be used in future maybe when data cache stalls?
}
class MCacheOut extends Bundle() {
  val data_even = Bits(width = 32) //lower 32 bits
  val data_odd = Bits(width = 32) //higher 32 bits
  val hit = Bits(width = 1) //hit/stall signal
}
class MCacheIO extends Bundle() {
  val mcachemem_in = new MCacheMemIn().asOutput
  val mcachemem_out = new MCacheMemOut().asInput
  val mcache_in = new MCacheIn().asInput
  val mcache_out = new MCacheOut().asOutput
  val extmem_in = new ExtMemIn().asOutput
  val extmem_out = new ExtMemOut().asInput
}

/* cache memory of icache and mcache should be integrated in one mem.scala class and one package: cache 
   ...at least external Memory since it is a simulation of a extmem and the same for icache and mcache
*/
class ExtMemIn extends Bundle() {
  val address = Bits(width = 32)
  val msize = Bits(width = METHOD_SIZETAG_WIDTH) //size or block count to fetch
  val fetch = Bits(width = 1)
}
class ExtMemOut extends Bundle() {
  val data = Bits(width = 32)
  val ready = Bits(width = 1)
}
class ExtMemIO extends Bundle() {
  val extmem_in = new ExtMemIn().asInput
  val extmem_out = new ExtMemOut().asOutput
}
class MCacheMemIn extends Bundle() {
  val w_enable = Bits(width = 1)
  val w_data = Bits(width = 32)
  val address = Bits(width = 32) //should be 32 because we need whole address for tag?!

  val w_tag = Bits(width = 1)
}
class MCacheMemOut extends Bundle() {
  val even_data = Bits(width = INSTR_WIDTH)
  val odd_data = Bits(width = INSTR_WIDTH)
  val hit = Bits(width = 1)
}
class MCacheMemIO extends Bundle() {
  val mcachemem_in = new MCacheMemIn().asInput
  val mcachemem_out = new MCacheMemOut().asOutput
}

object MCacheMem {

  def get_address(pos : Bits,  offset : Bits) : Bits = {
    ((pos * Bits(METHOD_BLOCK_SIZE)) + offset) / UFix(2)
  }

}

/*
 memory of the method cache
*/
class MCacheMem extends Component {
  val io = new MCacheMemIO()
  val ram_mcache_even = Mem(MCACHE_SIZE / 2, seqRead = true) {Bits(width = INSTR_WIDTH)}
  val ram_mcache_odd = Mem(MCACHE_SIZE / 2, seqRead = true) {Bits(width = INSTR_WIDTH)}

  //IS THERE a VALID TAG NEEDED?
  //TAG FIELDS THINK ABOUT HOW TO USE MEMORY HERE
  val mcache_addr_tag = Mem(METHOD_COUNT) {Bits(width = 32)}
  val mcache_size_tag = Mem(METHOD_COUNT) {Bits(width = METHOD_SIZETAG_WIDTH)}
  val mcache_list_prev = Mem(METHOD_COUNT) {Bits(width = METHOD_COUNT_WIDTH)}
  val mcache_list_next = Mem(METHOD_COUNT) {Bits(width = METHOD_COUNT_WIDTH)}

  //regs
  val dout_even = Reg() {Bits(width = INSTR_WIDTH)}
  val dout_odd = Reg() {Bits(width = INSTR_WIDTH)}
  val dout_hit = Reg() {Bits(width = 1)}

  val lru_tag = Reg(resetVal = Bits(0, width = METHOD_COUNT_WIDTH))
  val mru_tag = Reg(resetVal = Bits(METHOD_COUNT - 1, width = METHOD_COUNT_WIDTH))

  //signals
  //val address = io.mcachemem_in.address / UFix(2)
  val data_even = Bits(width = INSTR_WIDTH)
  val data_odd = Bits(width = INSTR_WIDTH)
  val addr_offset = Bits(width = METHOD_SIZETAG_WIDTH)

  //tag field type
  class TagField extends Bundle {
    val pos = Bits(width = METHOD_COUNT_WIDTH)
    val hit = Bits(width = 1)
    val tag = Bits(width = 32)
  }

  //new bundle
  val tag_field = new TagField()

  //init signals
  data_even := Bits(0)
  data_odd := Bits(0)
  tag_field := search_tag_addr(io.mcachemem_in.address)
  addr_offset := io.mcachemem_in.address - tag_field.tag //offset between incoming address and base address

  //init list
  def init_tag_list() = {
    for (i <- 0 until METHOD_COUNT - 1) {
      mcache_list_next(Bits(i)) := Bits(i) - Bits(1)
      mcache_list_prev(Bits(i)) := Bits(i) + Bits(1)
    }
  }

  //how we can init Memories???
  val list_init = Reg(resetVal = Bits(0, width = 1))
  when (list_init === Bits(0)) {
    init_tag_list()
  }
  list_init := Bits(1)

  //update list, input tag is the position which should be updated
  def update_tag_list(tag : Bits) = {
    //moving current tag to head
    mru_tag := tag
    mcache_list_next(tag) := mru_tag
    mcache_list_prev(mru_tag) := tag

    when (tag === lru_tag) {
      lru_tag := mcache_list_prev(tag)
    }
    .elsewhen (tag === mru_tag) {
      //nothing to do here?
    }
    .otherwise {
      mcache_list_next(mcache_list_prev(tag)) := mcache_list_next(tag)
      mcache_list_prev(mcache_list_next(tag)) := mcache_list_prev(tag)
    }    
  }

  def search_tag_addr (addr : Bits) : TagField = {
    val tagfield = new TagField()
    tagfield.tag := Bits(0)
    tagfield.hit := Bits(0)
    tagfield.pos := Bits(0)
    for (i <- 0 until METHOD_COUNT - 1) {
      when ((io.mcachemem_in.address >= mcache_addr_tag(Bits(i))) && (io.mcachemem_in.address <= (mcache_addr_tag(Bits(i)) + mcache_size_tag(Bits(i))))) {
        tagfield.pos := Bits(i)
        tagfield.hit := Bits(1)
        tagfield.tag := mcache_addr_tag(Bits(i))  //not really used!
      }
    }
    tagfield
  }

  //write at LRU index the sicze and the address tag
  when (io.mcachemem_in.w_tag === Bits(1)) {

    mcache_addr_tag(lru_tag) := io.mcachemem_in.address
    mcache_size_tag(lru_tag) := io.mcachemem_in.w_data
    update_tag_list(lru_tag) 

  }

  //write at lru place signed the round before
  when (io.mcachemem_in.w_enable) {
    when (tag_field.hit === Bits(1)) {

      when (addr_offset(0) === Bits(1)) {
        ram_mcache_odd((tag_field.pos * Bits(METHOD_BLOCK_SIZE)) + (addr_offset / UFix(2))) := io.mcachemem_in.w_data
      }
      .otherwise {
        ram_mcache_even((tag_field.pos * Bits(METHOD_BLOCK_SIZE)) + (addr_offset / UFix(2))) := io.mcachemem_in.w_data
      }

    }.otherwise {} //expection should be in the tag_field
  }
  //read
  .otherwise {
    
    when (tag_field.hit === Bits(1)) {
      data_even := ram_mcache_even(Mux(addr_offset(0), get_address(tag_field.pos, addr_offset) + Bits(1), get_address(tag_field.pos, addr_offset)))
      data_odd := ram_mcache_odd(get_address(tag_field.pos, addr_offset))
    }
    dout_hit := tag_field.hit
    dout_even := data_even
    dout_odd := data_odd
  }

  io.mcachemem_out.even_data := dout_even
  io.mcachemem_out.odd_data := dout_odd 
  io.mcachemem_out.hit := dout_hit

}

/*
 Object of external memory implemented in ROM
 */
object ExtMemROM {

  //external memory instance
  val rom_extmem = Vec(EXTMEM_SIZE) {Bits(width = 32)} //how is the bus width?

  //init the rom memory with dummy messages
  def initROM_random() {
    for (i <- 0 until (EXTMEM_SIZE)) {
      rom_extmem(i) = Bits(i)
    }
    rom_extmem(1) = Bits(10)
    rom_extmem(0) = Bits(10)
  }
  
  /**
   * Read a binary file into the ROM vector, from Utility.scala
     Author: Martin Schoeberl
   */
  def initROM_bin(fileName: String): Vec[Bits] = { 
    println("Reading " + fileName)
    // an encodig to read a binary file? Strange new world.
    val source = scala.io.Source.fromFile(fileName)(scala.io.Codec.ISO8859)
    val byteArray = source.map(_.toByte).toArray
    source.close()
    for (i <- 0 until byteArray.length / 4) {
      var word = 0
      for (j <- 0 until 4) {
        word <<= 8
        word += byteArray(i * 4 + j).toInt & 0xff
      }
      printf("%08x\n", word)
      // mmh, width is needed to keep bit 31
      rom_extmem(i) = Bits(word, width=32)
    }
    // generate some dummy data to fill the table and make Bit 31 test happy
    for (x <- byteArray.length / 4 until EXTMEM_SIZE)
      rom_extmem(x) = Bits("h8000000000000000")
    rom_extmem
  }
}

/*
 External memory implemented as ROM onchip in chisel...
 TODO: ADD a delay for simulation of a real external memory access penalty
 */
class ExtMemROM(filename: String) extends Component {
  val io = new ExtMemIO()
  val rom_init = Reg(resetVal = Bits(0, width = 1))
  val dout = Reg(resetVal = Bits(0, width = 32))
  val dout_ready = Reg(resetVal = Bits(0, width = 1))
  val burst_counter = Reg(resetVal = UFix(0, width = 32))
  val read_address = Reg(resetVal = UFix(0))

  //reading something into rom for debugging
  rom_init := Bits(1)
  when (rom_init === Bits(0)) {
    initROM_bin(filename)
    //initROM_random()
  }
  
  when (io.extmem_in.fetch) {
    dout := rom_extmem(io.extmem_in.address)
    dout_ready := Bits(1)
    read_address := io.extmem_in.address + UFix(1)
    burst_counter := io.extmem_in.msize - UFix(1)
  }
  .elsewhen (burst_counter != Bits(0)) {
    dout := rom_extmem(read_address)
    dout_ready := Bits(1)
    burst_counter := burst_counter - UFix(1)
    read_address := read_address + UFix(1)
  }
  .otherwise {
    dout := Bits(0)
    dout_ready := Bits(0)
  }

  io.extmem_out.data := dout
  io.extmem_out.ready := dout_ready

}

class MCache extends Component {
  val io = new MCacheIO()

  //fsm variables
  val init_state :: idle_state :: size_state :: transfer_state :: restart_state :: Nil = Enum(5){ UFix() }
  val mcache_state = Reg(resetVal = init_state)

  //signals
  val mcache_data_even = Bits(width = DATA_WIDTH)
  val mcache_data_odd = Bits(width = DATA_WIDTH)
  val extmem_fetch = Bits(width = 1)
  val extmem_fetch_address = Bits(width = 32)
  val extmem_msize = Bits(width = METHOD_SIZETAG_WIDTH)
  val mcachemem_address = Bits(width = 32) //???
  val mcache_w_enable = Bits(width = 1)
  val mcachemem_w_data = Bits(width = DATA_WIDTH)
  val mcachemem_wtag = Bits(width = 1)

  //regs
  val transfer_size = Reg(resetVal = Bits(0, width = METHOD_SIZETAG_WIDTH))
  val fword_counter = Reg(resetVal = Bits(0, width = 32))
  val mcache_address = Reg(resetVal = Bits(0, width = 32)) //save address in case no hit occours
  val mcache_address_next = Reg(resetVal = Bits(0, width = 32)) //save next address in case of fetch to restart fast

  //init signals
  mcache_data_even := Bits(0)
  mcache_data_odd := Bits(0)
  extmem_fetch := Bits(0)
  extmem_fetch_address := Bits(0)
  extmem_msize := Bits(0)
  mcachemem_wtag := Bits(0)
  mcachemem_w_data := Bits(0)
  mcache_w_enable := Bits(0)
  mcachemem_address := io.mcache_in.address

  //when memcache replies with a hit valid data can be outputed
  when (io.mcachemem_out.hit === Bits(1)) {
    mcache_data_even := io.mcachemem_out.even_data
    mcache_data_odd := io.mcachemem_out.odd_data
  }

  //fsm
  when (mcache_state === init_state) {
    when(io.mcache_in.request) {
      mcache_address := io.mcache_in.address
      mcache_state := idle_state
    }
  }
  //check if instruction is available
  when (mcache_state === idle_state) {
    when(io.mcachemem_out.hit === Bits(1)) {
      mcache_address := io.mcache_in.address
    }
     //no hit... fetch from external memory
    .otherwise {
      mcache_address_next := io.mcache_in.address //save pc + 4 for restart
      mcache_state := size_state
      extmem_fetch := Bits(1)
      extmem_fetch_address := mcache_address - Bits(1) // -1 because size is at method head -1
      extmem_msize := Bits(1) //here we could fetch already one first block instead single size tag
    }
  }

  //fetch size of the required method from external memory
  when (mcache_state === size_state) {
    when (io.extmem_out.ready === Bits(1)) {
      
      fword_counter := io.extmem_out.data / Bits(WORD_COUNT) //size is given in bytes not words
      extmem_fetch := Bits(1)
      extmem_fetch_address := mcache_address //fetch from extmem with latched address
      extmem_msize := io.extmem_out.data / Bits(WORD_COUNT) //size of words zu fetch
      transfer_size := io.extmem_out.data / Bits(WORD_COUNT) //save transfer size because extmem is accessed in burst mode
      mcachemem_wtag := Bits(1)  //init transfer in mcachemem
      mcachemem_w_data := io.extmem_out.data //write size to mcachemem for LRU tagfield
      mcachemem_address := mcache_address //write base address to mcachemem for LRU tagfield
      mcache_state := transfer_state

    }
  }

  //transfer/fetch method to the cache
  when (mcache_state === transfer_state) {
    when (fword_counter > Bits(0)) {
      when (io.extmem_out.ready === Bits(1)) {
        fword_counter := fword_counter - Bits(1)
        mcachemem_w_data := io.extmem_out.data //write fetched data to method cache memory
        mcache_w_enable := Bits(1)
        mcachemem_address := mcache_address + (transfer_size - fword_counter) //adress is base address + offset
      }
    }
    .otherwise {
      mcachemem_address := mcache_address
      mcache_state := restart_state
    }
  }

  //restart using the latched address_next address
  when (mcache_state === restart_state) {
    mcachemem_address := mcache_address_next
    mcache_address := mcache_address_next
    mcache_state := idle_state
  }

  //set output signals
  io.mcachemem_in.address := mcachemem_address
  io.mcachemem_in.w_enable := mcache_w_enable
  io.mcachemem_in.w_data := mcachemem_w_data
  io.mcachemem_in.w_tag := mcachemem_wtag

  io.mcache_out.data_even := mcache_data_even
  io.mcache_out.data_odd := mcache_data_odd
  io.mcache_out.hit := io.mcachemem_out.hit

  io.extmem_in.address := extmem_fetch_address
  io.extmem_in.fetch := extmem_fetch
  io.extmem_in.msize := extmem_msize

}
