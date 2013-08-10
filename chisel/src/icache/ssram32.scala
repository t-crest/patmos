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
 * SSRAM connection to memory bus (f.e. DE2-70 board)
 * 
 * Author: Philipp Degasperi (philipp.degasperi@gmail.com)
 * 
 */

package patmos

import Chisel._
import Node._
import MConstants._
import ocp._

import scala.collection.mutable.HashMap

/*
 for the start a small version of SimpleCon interface
 */
// class ScInType extends Bundle() {
//   val rd_data = Bits(width = 32)
//   val rd_count = UFix(width = 2)
// }
// class ScOutType extends Bundle() {
//   val address = Bits(width = 19)
//   val wr_data = Bits(width = 32)
//   val rd = Bits(width = 1)
//   val wr = Bits(width = 1)
//   val byte_ena = Bits(width = 4) //Bytes per word
// }

class RamInType extends Bundle() {
  val din = Bits(width = 32)
}
class RamOutType extends Bundle() {
  val addr = Bits(width = 19)
  val dout_ena = Bits(width = 1) //needed to drive tristate in top level
  val nadsc = Bits(width = 1)
  val noe = Bits(width = 1)
  val nbwe = Bits(width = 1)
  val nbw = Bits(width = 4)
  val ngw = Bits(width = 1)
  val nce1 = Bits(width = 1)
  val ce2 = Bits(width = 1)
  val nce3 = Bits(width = 1)
  val nadsp = Bits(width = 1)
  val nadv = Bits(width = 1)
  val dout = Bits(width = 32)
}
class SsramIOBurst extends Bundle() {
  val ocp_port = new OcpBurstSlavePort(19, 32, 4)
  val ram_out = new RamOutType().asOutput
  val ram_in = new RamInType().asInput
}

/*
  SSRAM Controller for a Burst R/W access to the SSRAM
  Notes: Burst addresses in the used SSRAM are generated internally only for max. 4 addresses (= max. burst lenght)
  >> setting the mode to 0 a linear burst is possible, setting mode to 1 a interleaved burst is done by the SSRAM
*/
class SsramBurstRW (
   ram_ws_rd : Int = 2,
   ram_ws_wr : Int = 0,
   burstLen : Int = 4
) extends Component {
  val io = new SsramIOBurst()

  val idle :: rd1 :: wr1 :: Nil = Enum(3){ UFix() }
  val ssram_state = Reg(resetVal = idle)
  val wait_state = Reg(resetVal = UFix(0, width = 4))
  val burst_cnt = Reg(resetVal = UFix(0, width = 8))
  val rd_data_ena = Reg(resetVal = Bits(0, width = 1))
  val rd_data = Reg(resetVal = Bits(0, width = 32))
  val resp = Reg(resetVal = Bits(0, width = 2))
  val ram_dout = Reg(resetVal = Bits(0, width = 32))
  val address = Reg(resetVal = Bits(0, width = 19))
  val dout_ena = Reg(resetVal = Bits(0, width = 1))
  val nadsc = Reg(resetVal = Bits(1, width = 1))
  val noe = Reg(resetVal = Bits(1, width = 1))
  val nbwe = Reg(resetVal = Bits(1, width = 1))
  val nbw = Reg(resetVal = Bits("b1111", width = 4))
  val nadv = Reg(resetVal = Bits(0, width = 1))

  //init default register values
  rd_data_ena := Bits(0)
  dout_ena := Bits(0)
  nadsc := Bits(1)
  noe := Bits(1)
  nbwe := Bits(1)
  nbw := Bits("b1111")
  nadv := Bits(1)
  resp := OcpResp.NULL
  ram_dout := io.ocp_port.M.Data
  burst_cnt := UFix(1)

  //catch inputs
  when (io.ocp_port.M.Cmd === OcpCmd.RD || io.ocp_port.M.Cmd === OcpCmd.WR) {
    address := io.ocp_port.M.Addr
  }

  //following helps to output only when output data is valid
  io.ocp_port.S.Data := rd_data
  when (rd_data_ena === Bits(1)) {
    io.ocp_port.S.Data := io.ram_in.din
    rd_data := io.ram_in.din //read data can be used depending how the top-level keeps register of input or not
  }
 
  when (ssram_state === rd1) {
    noe := Bits(0)
    nadv := Bits(0)
    when (wait_state <= UFix(1)) {
      rd_data_ena := Bits(1)
      burst_cnt := burst_cnt + UFix(1)
      resp := OcpResp.DVA
      when (burst_cnt === UFix(burstLen)) {
        burst_cnt := UFix(0)
        nadv := Bits(1)
        noe := Bits(1)
        ssram_state := idle
      }
    }
  }
  when (ssram_state === wr1) {
    when (wait_state <= UFix(1)) {
      when (burst_cnt === UFix(burstLen)) {
        burst_cnt := UFix(0)
        resp := OcpResp.DVA
        ssram_state := idle
      }
      when (io.ocp_port.M.DataValid === Bits(1)) {
        burst_cnt := burst_cnt + UFix(1)
        nadv := Bits(0)
        nbwe := Bits(0)
        nbw := Bits("b0000")
        dout_ena := Bits(1)
      }
    }
  }

  when (io.ocp_port.M.Cmd === OcpCmd.RD) {
    ssram_state := rd1
    nadsc := Bits(0)
    noe := Bits(0)
  }
  .elsewhen(io.ocp_port.M.Cmd === OcpCmd.WR && io.ocp_port.M.DataValid === Bits(1)) {
    ssram_state := wr1
    nbwe := Bits(0)
    nbw := ~(io.ocp_port.M.DataByteEn)
    dout_ena := Bits(1)
    nadsc := Bits(0)
  }

  //counter till output is ready
  when (wait_state != UFix(0)) {
    wait_state := wait_state - UFix(1)
  }
  //set wait state after incoming request
  when (io.ocp_port.M.Cmd === OcpCmd.RD) {
    wait_state := UFix(ram_ws_rd + 1)
  }
  when (io.ocp_port.M.Cmd === OcpCmd.WR) {
    wait_state := UFix(ram_ws_wr + 1)
  }

  io.ram_out.dout := io.ocp_port.M.Data
  when (dout_ena === Bits(1)) {
    io.ram_out.dout := ram_dout
  }

  //output registers
  io.ram_out.nadsc := nadsc
  io.ram_out.noe := noe
  io.ram_out.nbwe := nbwe
  io.ram_out.nbw := nbw
  io.ram_out.nadv := nadv
  io.ram_out.dout_ena := dout_ena
  io.ram_out.addr := address
  //output to master
  io.ocp_port.S.Resp := resp
  //output fixed signals
  io.ram_out.ngw := Bits(1)
  io.ram_out.nce1 := Bits(0)
  io.ram_out.ce2 := Bits(1)
  io.ram_out.nce3 := Bits(0)
  io.ram_out.nadsp := Bits(1)
}

/*
 Test Class for the SSRAM implemenation
 */
class SsramTest(c: SsramBurstRW) extends Tester(c, Array(c.io)) {
  defTests {
    var allGood = true
    val vars = new HashMap[Node, Node]()
    val ovars = new HashMap[Node, Node]()
    vars.clear()
    ovars.clear()
    println("RUN")
    for (i <- 0 until 100) {
      allGood = step(vars, ovars) && allGood
    }
    allGood
  }
}

/*
 Used to instantiate a single SSRAM control component
 */
object SsramMain {
  def main(args: Array[String]): Unit = {
    chiselMainTest(args, () => new SsramBurstRW()) { f => new SsramTest(f) }
  }
}

/*
 External Memory, only to simulate a SSRAM in Chisel as a on-chip memory implementation
 and reading some data from binary to memory vector
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
  val address = Reg(resetVal = Bits(0, width = 19))
  val dout = Reg(resetVal = Bits(0, width = 32))
  val nadv = Reg(resetVal = Bits(0, width = 1))

  nadv := io.ram_out.nadv
  when (io.ram_out.nadsc === Bits(0)) {
    address := io.ram_out.addr
  }
  .elsewhen (nadv === Bits(0)) {
    address := address + Bits(1)
  }
  when (io.ram_out.noe === Bits(0)) {
    dout := ssram_extmem(address)
  }
  io.ram_in.din := dout
}

class SsramIOSingle extends Bundle() {
  val ocp_port = new OcpCoreSlavePort(19, 32)
  val ram_out = new RamOutType().asOutput
  val ram_in = new RamInType().asInput
}
/*
 SSRAM Controller with single r/w interface (OCP_CORE)
 ...not needed in future can also be achieved by setting burst lenght to 1
 */
class SsramSingleRW (
   ram_ws_rd : Int = 2,
   ram_ws_wr : Int = 1
) extends Component {
  val io = new SsramIOSingle()

  val idl :: rd1 :: wr1 :: Nil = Enum(3){ UFix() }
  val ssram_state = Reg(resetVal = idl)
  val wait_state = Reg(resetVal = UFix(0, width = 4))
  val resp = Reg(resetVal = Bits(0, width = 2))
  val ram_dout = Reg(resetVal = Bits(0, width = 32))
  val address = Reg(resetVal = Bits(0, width = 19))
  val dout_ena = Reg(resetVal = Bits(0, width = 1))
  val nadsc = Reg(resetVal = Bits(1, width = 1))
  val noe = Reg(resetVal = Bits(1, width = 1))
  val nbwe = Reg(resetVal = Bits(1, width = 1))
  val nbw = Reg(resetVal = Bits("b1111", width = 4))
  val nadv = Reg(resetVal = Bits(0, width = 1))
  //val rd_data_ena = Reg(resetVal = Bits(0, width = 1)) //used only if reg not handled in top-level

  //init default register values
  //rd_data_ena := Bits(0)
  dout_ena := Bits(0)
  nadsc := Bits(1)
  noe := Bits(1)
  nbwe := Bits(1)
  nbw := Bits("b1111")
  nadv := Bits(1)
  resp := OcpResp.NULL

  //catch inputs
  when (io.ocp_port.M.Cmd === OcpCmd.RD || io.ocp_port.M.Cmd === OcpCmd.WR) {
    address := io.ocp_port.M.Addr
  }
  when (io.ocp_port.M.Cmd === OcpCmd.WR) {
    ram_dout := io.ocp_port.M.Data
  }
  //when (rd_data_ena === Bits(1)) {
    // io.ocp_port.S.Data := io.ram_in.din
  // }

  //fsm, next state + output logic 
  when (ssram_state === rd1) {
    noe := Bits(0)
    when (wait_state === UFix(1)) {
      resp := OcpResp.DVA
      ssram_state := idl
    }
  }
  when (ssram_state === wr1) {
    when (wait_state === UFix(1)) {
      ssram_state := idl
      resp := OcpResp.DVA
    }
  }
  when (io.ocp_port.M.Cmd === OcpCmd.RD) {
    ssram_state := rd1
    nadsc := Bits(0)
    noe := Bits(0)
  }
  .elsewhen(io.ocp_port.M.Cmd === OcpCmd.WR) {
    ssram_state := wr1
    nbwe := Bits(0)
    nbw := ~(io.ocp_port.M.ByteEn)
    dout_ena := Bits(1)
    nadsc := Bits(0)
  }

  //counter till output is ready
  when (wait_state != UFix(0)) {
    wait_state := wait_state - UFix(1)
  }
  //set wait state after incoming request
  when (io.ocp_port.M.Cmd === OcpCmd.RD) {
    wait_state := UFix(ram_ws_rd + 1)
  }
  when (io.ocp_port.M.Cmd === OcpCmd.WR) {
    wait_state := UFix(ram_ws_wr + 1)
  }

  io.ram_out.dout := io.ocp_port.M.Data
  when (dout_ena === Bits(1)) {
    io.ram_out.dout := ram_dout
  }
  //output registers
  io.ram_out.nadsc := nadsc
  io.ram_out.noe := noe
  io.ram_out.nbwe := nbwe
  io.ram_out.nbw := nbw
  io.ram_out.nadv := nadv
  io.ram_out.dout_ena := dout_ena //needed for driving tristate in top-l
  io.ram_out.addr := address
  //output response to master
  io.ocp_port.S.Resp := resp
  io.ocp_port.S.Data := io.ram_in.din
  //output fixed signals
  io.ram_out.ngw := Bits(1)
  io.ram_out.nce1 := Bits(0)
  io.ram_out.ce2 := Bits(1)
  io.ram_out.nce3 := Bits(0)
  io.ram_out.nadsp := Bits(1)
}

/*
 old memory class for reading a bin file in to a vector and outputing in a burst-like mode
 should be removed in the future only needed for keeping the current version of method cache
*/
// class ExtMemIn extends Bundle() {
//   val address = Bits(width = 32)
//   val msize = Bits(width = METHOD_SIZETAG_WIDTH) //size or block count to fetch
//   val fetch = Bits(width = 1)
// }
// class ExtMemOut extends Bundle() {
//   val data = Bits(width = 32)
//   val ready = Bits(width = 1)
// }
// class ExtMemIO extends Bundle() {
//   val extmem_in = new ExtMemIn().asInput
//   val extmem_out = new ExtMemOut().asOutput
// }
// class ExtMemROM(fileName: String) extends Component {
//   val io = new ExtMemIO()
//   val rom_init = Reg(resetVal = Bits(0, width = 1))
//   val dout = Reg(resetVal = Bits(0, width = 32))
//   val dout_ready = Reg(resetVal = Bits(0, width = 1))
//   val burst_counter = Reg(resetVal = UFix(0, width = 32))
//   val read_address = Reg(resetVal = UFix(0))
//   //external memory instance
//   val rom_extmem = Vec(2 * MCACHE_SIZE) {Bits(width = 32)} //bus width = 32
//   /**
//    * Read a binary file into the ROM vector, from Utility.scala
//      Author: Martin Schoeberl
//    */
//   def initROM_bin(fileName: String): Vec[Bits] = { 
//     println("Reading " + fileName)
//     // an encodig to read a binary file? Strange new world.
//     val source = scala.io.Source.fromFile(fileName)(scala.io.Codec.ISO8859)
//     val byteArray = source.map(_.toByte).toArray
//     source.close()
//     for (i <- 0 until byteArray.length / 4) {
//       var word = 0
//       for (j <- 0 until 4) {
//         word <<= 8
//         word += byteArray(i * 4 + j).toInt & 0xff
//       }
//       printf("%08x\n", word)
//       // mmh, width is needed to keep bit 31
//       rom_extmem(i) = Bits(word, width=32)
//     }
//     // generate some dummy data to fill the table and make Bit 31 test happy
//     for (x <- byteArray.length / 4 until MCACHE_SIZE * 2)
//       rom_extmem(x) = Bits("h8000000000000000")
//     rom_extmem
//   }

//   initROM_bin(fileName)
  
//   when (io.extmem_in.fetch) {
//     dout := rom_extmem(io.extmem_in.address)
//     dout_ready := Bits(1)
//     read_address := io.extmem_in.address + UFix(1)
//     burst_counter := (io.extmem_in.msize - UFix(1)) % UFix(MCACHE_SIZE - 1)
//   }
//   .elsewhen (burst_counter != Bits(0)) {
//     dout := rom_extmem(read_address)
//     dout_ready := Bits(1)
//     burst_counter := burst_counter - UFix(1)
//     read_address := read_address + UFix(1)
//   }
//   .otherwise {
//     dout := Bits(0)
//     dout_ready := Bits(0)
//   }
//   io.extmem_out.data := dout
//   io.extmem_out.ready := dout_ready
// }
