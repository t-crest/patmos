/*
 Instruction Cache - Direct Mapping, only one instruction per cycle (instr_a)
 Author: Philipp Degasperi (philipp.degasperi@gmail.com)
 */

package icache

import Chisel._
import Node._
//import ICacheMem._
import ExtMemROM._

import scala.collection.mutable.HashMap
import scala.util.Random
import scala.math

// NOT USED ANY MORE! maybe in future again when we care about offset or add more lines...
/*
object ICacheMem {
  def get_tag_waddress(raw_address: Bits) = {
    raw_address(Constants.INDEX_FIELD_SIZE + Constants.OFFSET_SIZE - 1, 4) * Bits(Constants.TAG_FIELD_SIZE + Constants.VALIDBIT_FIELD_SIZE)
  }
  def get_data_waddress(raw_address: Bits) = {
    Cat(raw_address(Constants.INDEX_FIELD_SIZE + Constants.OFFSET_SIZE - 1, 2), Bits("b00"))
  }
}
*/
class ICacheMem extends Component {
  val io = new ICacheMemIO()

  //reserve memory for the instruction cache data
  val ram_icache = Mem(Constants.ICACHE_SIZE, seqRead = true) {Bits(width = 32)}
  val dout = Reg() {Bits(width = 32)}
  when (io.icachemem_in.w_enable) { ram_icache(io.icachemem_in.address) := io.icachemem_in.w_data }
  .otherwise { 
    dout := ram_icache(io.icachemem_in.address) //Cat(ram_icache(io.icachemem_in.address), ram_icache(io.icachemem_in.address + Bits(1)))
  }
  io.icachemem_out.r_data := dout

  //reserve memory for the instruction cache tag field containing valid bit and address tag
  val ram_icachetag = Mem(Constants.TAG_CACHE_SIZE, seqRead = true) {Bits(width = Constants.TAG_FIELD_SIZE + Constants.VALIDBIT_FIELD_SIZE)}
  val tout = Reg() { Bits() }
  val tag_field = Bits(width = Constants.TAG_FIELD_SIZE + Constants.VALIDBIT_FIELD_SIZE)
  tag_field := Bits(0)
  when (io.icachemem_in.w_enable) {
    tag_field := Cat(io.icachemem_in.tag, Bits(1))
    ram_icachetag(io.icachemem_in.address / UFix(4)) := (tag_field)
  }
  .otherwise { tout := ram_icachetag(io.icachemem_in.address / UFix(4)) }
  io.icachemem_out.tag := tout(Constants.TAG_FIELD_SIZE, Constants.VALIDBIT_FIELD_SIZE)
  io.icachemem_out.valid := tout(0)
}

/*
 Object of external memory implemented in ROM
 */
object ExtMemROM {

  //external memory instance
  val rom_extmem = Vec(Constants.EXTMEM_SIZE) {Bits(width = 32)} //for test extemem = 2*icachemem

  //init the rom memory with dummy messages counter 0-xxx
  def initROM() {
    for (i <- 0 until (Constants.EXTMEM_SIZE - 1)) {
      rom_extmem(i) = Bits(i)
    }
  }
  
  /**
   * Read a binary file into the ROM vector, from Utility.scala
     Author: Martin Schoeberl
   */
  def readBin(fileName: String): Vec[Bits] = { 
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
    for (x <- byteArray.length / 4 until 256)
      rom_extmem(x) = Bits("h8000000000000000")
    rom_extmem
  }
}

/*
 External memory implemented as ROM onchip in chisel...
 TODO: ADD a delay for simulation of a real external memory access penalty
 */
class ExtMemROM(filename: String) extends Component {
//class ExtMemROM extends Component {
  val io = new ExtMemIO()
  val rom_init = Reg(resetVal = Bits(0, width = 1))
  val dout = Reg(resetVal = Bits(0, width = 32))
  val dout_ready = Reg(resetVal = Bits(0, width = 1))
  val burst_counter = Reg(resetVal = UFix(0, width = 32))
  val read_address = Reg(resetVal = UFix(0))
  val fetching_data = Reg(resetVal = Bits(0))

  //only for debugging rom is filled with dummy data
  when (rom_init === Bits(0)) {
    rom_init := Bits(1)
    //readBin(filename)
    initROM()
    
  }
  
  when (io.extmem_in.fetch) {
    dout := rom_extmem(Cat(io.extmem_in.address(31,2), Bits("b00")))
    dout_ready := Bits(1)
    read_address := (Cat(io.extmem_in.address(31,2), Bits("b00")) + UFix(1))
    fetching_data := Bits(1)
  }
  .elsewhen (fetching_data === Bits(1)) {
    dout := rom_extmem(read_address)
    when (burst_counter === UFix(Constants.WORD_COUNT - 1)) {
      burst_counter := UFix(0)
      fetching_data := Bits(0)
      dout := Bits(0)
      dout_ready := Bits(0)
    }
    .otherwise {
      burst_counter := burst_counter + UFix(1)
    }
    read_address := read_address + UFix(1)
    dout_ready := Bits(1)
  }
  .otherwise {
    dout := Bits(0)
    dout_ready := Bits(0)
  }

  io.extmem_out.data := dout
  io.extmem_out.ready := dout_ready

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
  val icache_w_tag = Bits(width = Constants.TAG_FIELD_SIZE)
  val icache_instr_a = Bits(width = 32)
  val icache_instr_b = Bits(width = 32)
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
  icache_instr_a := Bits(0)
  icache_instr_b := Bits(0)
  extmem_fetch := Bits(0)
  extmem_fetch_address := Bits(0)

  //check for a hit
  when ((io.icachemem_out.tag === icache_address(Constants.TAG_FIELD_HIGH, Constants.TAG_FIELD_LOW)) && (io.icachemem_out.valid === Bits(1))) {
      icache_instr_a := io.icachemem_out.r_data(31,0)
      //icache_instr_b := io.icachemem_out.r_data(63,32) //fetching two instructions? NO!
      icache_hit := Bits(1)
  }


  //init sets the first address
  when (icache_state === init_state) {
    ///when (io.icache_in.request === Bits(1)) {
      icache_address := io.icache_in.address
      icache_state := check_hit_state
    //}
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
      when (fword_counter < UFix(Constants.WORD_COUNT)) {
        icache_fetched_data := io.extmem_out.data
        icache_w_tag := icache_address(Constants.TAG_FIELD_HIGH, Constants.TAG_FIELD_LOW)
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

  io.icache_out.instr_a := icache_instr_a
  io.icache_out.instr_b := icache_instr_b
  io.icache_out.ena := icache_hit

  io.extmem_in.address := extmem_fetch_address
  io.extmem_in.fetch := extmem_fetch

}


