/*
 Method Cache, 2 Method Blocks, LRU-Replacement
 Author: Philipp Degasperi (philipp.degasperi@gmail.com)
 */


//merge package mcache and icache in future
package mcache

import Chisel._
import Node._
import ExtMemROM._
import MConstants._

import scala.collection.mutable.HashMap
import scala.util.Random
import scala.math

object MConstants {

  //from Constants.scala
  val PC_SIZE = 32
  val INSTR_WIDTH = 32
  val DATA_WIDTH = 32

  //on chip 4KB icache
  val MCACHE_SIZE = 4096 //* 8 //4KB = 2^12*2^3 = 32*1024 = 32768Bit
  val EXTMEM_SIZE = 2 * MCACHE_SIZE // =32*2048

  val MAX_METHOD_SIZE = log2Up(MCACHE_SIZE)

  val WORD_COUNT = 4

  //DEBUG INFO
  println("MCACHE_SIZE=" + MCACHE_SIZE)
  println("EXTMEM_SIZE=" + EXTMEM_SIZE)
  println("WORD_COUNT=" + WORD_COUNT)
  println("MAX_METHOD_SIZE=" + MAX_METHOD_SIZE)
}

class MCacheIn extends Bundle() {
  val address = Bits(width = 32) //=pc_next
  val request = Bits(width = 1) //should be used in future maybe when data cache stalls?
}
class MCacheOut extends Bundle() {
  val instr_a = Bits(width = 32) //lower 32 bits
  val instr_b = Bits(width = 32) //higher 32 bits
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
  val msize = Bits(width = MAX_METHOD_SIZE) //size or block count to fetch
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
  val address = Bits(width = MAX_METHOD_SIZE) //at least method can be as big as the cache size
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
  val ram_icache = Mem(MCACHE_SIZE, seqRead = true) {Bits(width = 32)}
  val dout = Reg() {Bits(width = 32)}
  when (io.mcachemem_in.w_enable) { ram_icache(io.mcachemem_in.address) := io.mcachemem_in.w_data }
  .otherwise { 
    dout := Cat(ram_icache(io.mcachemem_in.address + Bits(1)), ram_icache(io.mcachemem_in.address))
  }
  io.mcachemem_out.r_data := dout

  //here is needed some memory to save methods in cache + size of them

}

/*
 Object of external memory implemented in ROM
 */
object ExtMemROM {

  //external memory instance
  val rom_extmem = Vec(EXTMEM_SIZE) {Bits(width = 32)} //width = 8?

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

  //only for debugging rom is filled with dummy data
  rom_init := Bits(1)
  when (rom_init === Bits(0)) {
    //initROM_bin(filename)
    initROM_random()
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

  val init_state :: idle_state :: size_state :: transfer_state :: restart_state :: Nil = Enum(5){ UFix() }
  val mcache_state = Reg(resetVal = init_state)

  //signals
  val mcache_instr_a = Bits(width = 32)
  val mcache_instr_b = Bits(width = 32)
  val mcache_hit = Bits(width = 1)
  val extmem_fetch = Bits(width = 1)
  val extmem_fetch_address = Bits(width = 32)
  val extmem_msize = Bits(width = MAX_METHOD_SIZE)
  val mcachemem_address = Bits(width = MAX_METHOD_SIZE)
  val mcache_w_enable = Bits(width = 1)
  val mcache_fetched_data = Bits(width = 32)

  //two registers only needed in a one method cache, otherwise it is moved to a cachetag logic
  val method_cachetag = Reg(resetVal = Bits(0, width = 32))
  val method_sizetag = Reg(resetVal = Bits(0, width = 32))

  val transfer_size = Reg(resetVal = Bits(0, width = MAX_METHOD_SIZE))
  val fword_counter = Reg(resetVal = Bits(0, width = 32))
  val mcache_address = Reg(resetVal = Bits(0, width = 32)) //save address in case no hit occours
  val mcache_address_next = Reg(resetVal = Bits(0, width = 32)) //save next address in case of fetch to restart fast

  //init signals
  mcache_hit := Bits(0)
  mcache_instr_a := Bits(0)
  mcache_instr_b := Bits(0)
  extmem_fetch := Bits(0)
  extmem_fetch_address := Bits(0)
  extmem_msize := Bits(0)

  //when n-method cache is used the address is set here
  mcachemem_address := Bits(0) //(io.mcache_in.address(MAX_METHOD_SIZE - 1, 0) - method_cachetag)

  //only in one method cache should be moved to cache logic afterwards
  when (io.mcache_in.address >= method_cachetag) {
    mcachemem_address := (io.mcache_in.address(MAX_METHOD_SIZE - 1, 0) - method_cachetag)
  }

  mcache_fetched_data := Bits(0)
  mcache_w_enable := Bits(0)

  //problem here? check the latched address mcache_address since in future the size and cachetag comes from memory
  when (mcache_address >= method_cachetag && mcache_address < (method_cachetag + method_sizetag)) {
    when ((mcache_state === idle_state) || (mcache_state === restart_state)) {
      mcache_hit := Bits(1)
      mcache_instr_a := Cat(Bits(1), io.mcachemem_out.r_data(30,0))
      //mcache_instr_a := io.mcachemem_out.r_data(31,0)
      mcache_instr_b := io.mcachemem_out.r_data(63,32)
    }
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
    when(mcache_hit === Bits(1)) {
      mcache_state := idle_state
      mcache_address := io.mcache_in.address
    }
    .otherwise { //no hit... fetch from external memory
      mcache_address_next := io.mcache_in.address
      mcache_state := size_state
      extmem_fetch := Bits(1)
      extmem_fetch_address := mcache_address - Bits(1)
      extmem_msize := Bits(1)
      method_cachetag := mcache_address //easy going here, gets more complicated with a list of methods
    }
  }

  //fetch size of the required method from external memory
  when (mcache_state === size_state) {
    when (io.extmem_out.ready === Bits(1)) {
      fword_counter := io.extmem_out.data
      extmem_fetch := Bits(1)
      extmem_fetch_address := mcache_address
      extmem_msize := io.extmem_out.data
      method_sizetag := io.extmem_out.data //easy going here, gets more complicated with a list of methods
      transfer_size := io.extmem_out.data
      mcache_state := transfer_state
    }
  }
  //fetch method to the cache
  when (mcache_state === transfer_state) {
    when (fword_counter > Bits(0)) {
      when (io.extmem_out.ready === Bits(1)) {
        fword_counter := fword_counter - Bits(1)
        //write fetched data to method cache memory
        mcache_fetched_data := io.extmem_out.data
        mcache_w_enable := Bits(1)
        mcachemem_address := (transfer_size - fword_counter) //mcache_address + (transfer_size - fword_counter)
      }
    }
    .otherwise {
       mcache_state := restart_state
       mcachemem_address := mcache_address
    }
  }
  //restart using the latched address_next address
  when (mcache_state === restart_state) {
    mcache_state := idle_state
    mcachemem_address := mcache_address_next
    mcache_address := mcache_address_next
  }

  //set output signals
  io.mcachemem_in.address := mcachemem_address
  io.mcachemem_in.w_enable := mcache_w_enable
  io.mcachemem_in.w_data := mcache_fetched_data

  io.mcache_out.instr_a := mcache_instr_a
  io.mcache_out.instr_b := mcache_instr_b
  io.mcache_out.hit := mcache_hit

  io.extmem_in.address := extmem_fetch_address
  io.extmem_in.fetch := extmem_fetch
  io.extmem_in.msize := extmem_msize

}
