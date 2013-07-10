/*
 Author: Philipp Degasperi (philipp.degasperi@gmail.com)

 SSRAM connection to memory bus (f.e. DE2-70 board)

 */


package patmos

import Chisel._
import Node._
import MConstants._

/*
 for the moment a small version of SimpleCon interface
 */
class ScCacheType extends Bundle() {
  val bypass :: direct_mapped_const :: direct_mapped :: full_assoc :: Nil = Enum(4){ Bits() }
}
class ScInType extends Bundle() {
  val rd_data = Bits(width = 32)
  val rd_count = Bits(width = 2)
}
class ScOutType extends Bundle() {
  val address = Bits(width = 23)
  val wr_data = Bits(width = 32)
  val rd = Bits(width = 1)
  val wr = Bits(width = 1)
  val byte_ena = Bits(width = 4) //Bytes per word
  //val cache = new sc_cache_t() //not working???
}
class RamInType extends Bundle() {
  val din = Bits(width = 32)
}
class RamOutType extends Bundle() {
  val addr = Bits(width = 23)
  //val clk = Bits(width = 1)
  val nadsc = Bits(width = 1)
  val noe = Bits(width = 1)
  val nwe = Bits(width = 1)
  //val nbw = Bits(width = 4)
  val ngw = Bits(width = 1)
  val nce1 = Bits(width = 1)
  val ce2 = Bits(width = 1)
  val nce3 = Bits(width = 1)
  val nadsp = Bits(width = 1)
  val nadv = Bits(width = 1)
  val dout = Bits(width = 32)
}


/*
 we could also use a OPC interface instead, f.e. when we want to have burst
 */
class SsramIO extends Bundle() {
  val ram_din_reg = Bits(INPUT, width = 32) //need to get the data input from inverted clock from top-level
  val sc_mem_out = new ScOutType().asInput
  val sc_mem_in = new ScInType().asOutput 
  val ram_out = new RamOutType().asOutput
  val ram_in = new RamInType().asInput
}

/*
  pipelined ssram access via a SimpCon interface
*/
class Ssram (
   ram_ws_rd : Int = 3,
   ram_ws_wr : Int = 3
) extends Component {
  
  val io = new SsramIO()

  val idl :: rd1 :: rd2 :: wr1 :: wr2 :: Nil = Enum(5){ Bits() }
  val ssram_state = Reg(resetVal = idl)

  val wait_state = Reg(resetVal = Bits(0, width = 4))
  val cnt = Bits(width = 2)
  val dout_ena = Bits(width = 1)
  val rd_data_ena = Bits(width = 1)

  val ram_dout = Bits(width = 32)

  //init default signal values
  rd_data_ena := Bits(0)
  dout_ena := Bits(0)
  ram_dout := Bits(0)
  //are the following really needed?
  io.ram_out.addr := Bits(0)
  io.sc_mem_in.rd_data := Bits(0)

  when (io.sc_mem_out.rd === Bits(1) || io.sc_mem_out.wr === Bits(1)) {
    io.ram_out.addr := io.sc_mem_out.address
  }
  when (io.sc_mem_out.wr === Bits(1)) {
    ram_dout := io.sc_mem_out.wr_data
  }
  when (rd_data_ena === Bits(1)) {
    io.sc_mem_in.rd_data := io.ram_din_reg
  }

  //next state logic
  when (ssram_state === idl) {
    //idle
  }
  when (ssram_state === rd1) {
    ssram_state := rd2
  }
  when (ssram_state === rd2) {
    when(wait_state === Bits(1)) {
      ssram_state := idl
    }
  }
  when (ssram_state === wr1) {
    ssram_state := wr2
  }
  when (ssram_state === wr2) {
    when (wait_state === Bits(1)) {
      ssram_state := idl
    }
  }

  //this gives pipeline level of 2
  when (io.sc_mem_out.rd === Bits(1)) {
    ssram_state := rd1
  }
  .elsewhen (io.sc_mem_out.wr === Bits(1)) {
    ssram_state := wr1
  }

  //output logic
  io.ram_out.nadsc := Bits(1)
  io.ram_out.noe := Bits(1)
  io.ram_out.nwe := Bits(1)
  //io.ram_out.nbw := Bits("b1111") //width = 4?!

  when (ssram_state === rd1) {
    io.ram_out.nadsc := Bits(0)
    io.ram_out.noe := Bits(0)
  }
  when (ssram_state === rd2) {
    io.ram_out.noe := Bits(0)
    when (wait_state === Bits(2)) {
      rd_data_ena := Bits(1)
    }
  }
  when (ssram_state === wr1) {
    io.ram_out.nadsc := Bits(0)
    io.ram_out.nwe := Bits(0)
    //io.ram_out.nbw := ~(io.sc_mem_out.byte_ena)
    dout_ena := Bits(1)
  }

  cnt := Bits("b11")

  when (wait_state != Bits(0)) {
    wait_state := wait_state - Bits(1)
  }

  when (wait_state(3,2) === Bits("b00")) {
    when (wait_state === Bits(0)) {
      cnt := Bits(0)
    }
      .otherwise{
      cnt := (wait_state)(1,0) - Bits(1)
    }
  }

  when (io.sc_mem_out.rd === Bits(1)) {
    wait_state := Bits(ram_ws_rd)

    //for what???
    /*when (Bits(ram_ws_rd) < Bits(3)) {
      cnt := Bits(ram_ws_rd + 1)(1,0)
    }
    .otherwise {
      cnt := Bits("b11")
    }*/
  }

  when (io.sc_mem_out.wr === Bits(1)) {
    wait_state := Bits(ram_ws_wr + 1)
    when (Bits(ram_ws_wr) < Bits(3)) {
      cnt := Bits(ram_ws_wr + 1)(1,0)
    }
    .otherwise {
      cnt := Bits("b11")
    }
  }

  io.ram_out.dout := Bits(0) //don't care
  when (dout_ena === Bits(1)) {
    io.ram_out.dout := ram_dout
  }

  io.sc_mem_in.rd_count := cnt

  io.ram_out.ngw := Bits(1)
  io.ram_out.nce1 := Bits(0)
  io.ram_out.ce2 := Bits(1)
  io.ram_out.nce3 := Bits(0)
  io.ram_out.nadsp := Bits(1)
  io.ram_out.nadv := Bits(1)

}


/*
  External Memory, only to simulate a SSRAM in Chisel as a on-chip memory implementation
*/
class ExtSsramIO extends Bundle() {
  val ram_out = new RamOutType().asInput
  val ram_in = new RamInType().asOutput
}

class ExtSsram(fileName : String) extends Component {
  val io = new ExtSsramIO()

  //on chip memory instance
  val ssram_extmem = Vec(2 * MCACHE_SIZE) {Bits(width = 32)} //bus width = 32

  def initSsram(fileName: String): Vec[Bits] = { 
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
      ssram_extmem(i) = Bits(word, width=32)
    }
    // generate some dummy data to fill the table and make Bit 31 test happy
    for (x <- byteArray.length / 4 until MCACHE_SIZE * 2)
      ssram_extmem(x) = Bits("h8000000000000000")
    ssram_extmem
  }

  initSsram(fileName)

  io.ram_in.din := Bits(0)
  val dout = Reg(resetVal = Bits(0, width = 32))
  val addr = Reg(resetVal = Bits(0, width = 23))
  addr := io.ram_out.addr
  when (io.ram_out.nwe === Bits(1) && io.ram_out.noe === Bits(0)) {
    dout := ssram_extmem(addr)
  }
  io.ram_in.din := dout
}

/*
 old memory class for reading a bin file in to a vector and outputing in a burst-like mode
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
class ExtMemROM(fileName: String) extends Component {
  val io = new ExtMemIO()
  val rom_init = Reg(resetVal = Bits(0, width = 1))
  val dout = Reg(resetVal = Bits(0, width = 32))
  val dout_ready = Reg(resetVal = Bits(0, width = 1))
  val burst_counter = Reg(resetVal = UFix(0, width = 32))
  val read_address = Reg(resetVal = UFix(0))
  //external memory instance
  val rom_extmem = Vec(2 * MCACHE_SIZE) {Bits(width = 32)} //bus width = 32
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
    for (x <- byteArray.length / 4 until MCACHE_SIZE * 2)
      rom_extmem(x) = Bits("h8000000000000000")
    rom_extmem
  }

  initROM_bin(fileName)
  
  when (io.extmem_in.fetch) {
    dout := rom_extmem(io.extmem_in.address)
    dout_ready := Bits(1)
    read_address := io.extmem_in.address + UFix(1)
    burst_counter := (io.extmem_in.msize - UFix(1)) % UFix(MCACHE_SIZE - 1)
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
