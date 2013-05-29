/*
 Method Cache, 2 Method Blocks, LRU-Replacement
 Author: Philipp Degasperi (philipp.degasperi@gmail.com)
 */


//merge package mcache and icache in future
package mcache

import Chisel._
import Node._
import ExtMemROM._

import scala.collection.mutable.HashMap
import scala.util.Random
import scala.math

object Constants {

  //on chip 4KB icache
  val MCACHE_SIZE = 4096 //* 8 //4KB = 2^12*2^3 = 32*1024 = 32768Bit
  val EXTMEM_SIZE = 2 * MCACHE_SIZE // =32*2048

  val WORD_COUNT = 4

  //DEBUG INFO
  println("MCACHE_SIZE=" + MCACHE_SIZE)
  println("EXTMEM_SIZE=" + EXTMEM_SIZE)
  println("WORD_COUNT=" + WORD_COUNT)
}

class MCacheIn extends Bundle() {
  val address = Bits(width = 32) //=pc
  val request = Bits(width = 1) //not used at the moment
}
class MCacheOut extends Bundle() {
  val instr_a = Bits(width = 32)
  val instr_b = Bits(width = 32)
  val ena = Bits(width = 1) //hit/stall signal
}
class MCacheIO extends Bundle() {
  val mcachemem_in = new MCacheMemIn().asOutput
  val mcachemem_out = new MCacheMemOut().asInput
  val mcache_in = new MCacheIn().asInput
  val mcache_out = new MCacheOut().asOutput
  val extmem_in = new ExtMemIn().asOutput
  val extmem_out = new ExtMemOut().asInput
}

/* cache memory of icache and mcache should be integrated in one mem.scala class and one package: cache */
class ExtMemIn extends Bundle() {
  val address = Bits(width = 32)
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
  val address = Bits(width = 32)
}
class MCacheMemOut extends Bundle() {
  val r_data = Bits(width = 64) //VLIW 2* 32Bit instr
}
class MCacheMemIO extends Bundle() {
  val mcachemem_in = new MCacheMemIn().asInput
  val mcachemem_out = new MCacheMemOut().asOutput
}

/*
 memory of the method cache
*/
class MCacheMem extends Component {
  val io = new MCacheMemIO()
  val ram_icache = Mem(Constants.MCACHE_SIZE, seqRead = true) {Bits(width = 32)}
  val dout = Reg() {Bits(width = 32)}
  when (io.mcachemem_in.w_enable) { ram_icache(io.mcachemem_in.address) := io.mcachemem_in.w_data }
  .otherwise { 
    dout := Cat(ram_icache(io.mcachemem_in.address), ram_icache(io.mcachemem_in.address + Bits(1))) //dout := ram_icache(io.mcachemem_in.address)
  }
  io.mcachemem_out.r_data := dout

  //here is needed some memory to save methods in cache + size of them

}

/*
 Object of external memory implemented in ROM
 */
object ExtMemROM {

  //external memory instance
  val rom_extmem = Vec(Constants.EXTMEM_SIZE) {Bits(width = 32)} //width = 8?

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

class MCache extends Component {
  val io = new MCacheIO()

  val init_state :: idle_state :: size_state :: transfer_state :: Nil = Enum(4){ UFix() }
  val mcache_state = Reg(resetVal = init_state)

  //signals
  val mcache_instr_a = Bits(width = 32)
  val mcache_instr_b = Bits(width = 32)
  val mcache_hit = Bits(width = 1)
  val extmem_fetch = Bits(width = 1)
  val extmem_fetch_address = Bits(width = 32)
  val mcachemem_address = Bits(width = 12)
  val mcache_w_enable = Bits(width = 1)
  val mcache_fetched_data = Bits(width = 32)

  //init signals
  mcache_hit := Bits(0)
  mcache_instr_a := Bits(0)
  mcache_instr_b := Bits(0)
  extmem_fetch := Bits(0)
  extmem_fetch_address := Bits(0)
  mcachemem_address := io.mcache_in.address(11,0)
  mcache_fetched_data := Bits(0)
  mcache_w_enable := Bits(0)


  //fsm
  when (mcache_state === init_state) {
    mcache_state := idle_state
  }

  when (mcache_state === idle_state) {
    mcache_state := idle_state
  }

  //set output signals
  io.mcachemem_in.address := mcachemem_address
  io.mcachemem_in.w_enable := mcache_w_enable
  io.mcachemem_in.w_data := mcache_fetched_data

  io.mcache_out.instr_a := mcache_instr_a
  io.mcache_out.instr_b := mcache_instr_b
  io.mcache_out.ena := mcache_hit

  io.extmem_in.address := extmem_fetch_address
  io.extmem_in.fetch := extmem_fetch

}
