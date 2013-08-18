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
 * Patmos top level component and test driver.
 * 
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * 
 */

/*

Keep a TODO list here, right at the finger tips:

- Look into ListLookup for instruction decoding


 */

package patmos

import Chisel._
import Node._
import MConstants._

import scala.collection.mutable.HashMap

import Constants._

/**
 * The main (top-level) component of Patmos.
 */

/*
 MC_Patmos is top-level with connected mcache... should be merged with Patmos top level in future
 */
class MCPatmos(fileName: String) extends Component {
  val io = new Bundle {
    val dummy = Bits(OUTPUT, 32)
    val led = Bits(OUTPUT, 8)
    val uartPins = new UartPinIO()
    //val sramPins = new RamOutPinsIO() 
  }

  val ssram = new SsramBurstRW()
  //chisel simulation for ssram... would be nice to implement it only in the tester unit...
  val extmemssram = new ExtSsram(fileName)
  ssram.io.ram_out <> extmemssram.io.ram_out
  ssram.io.ram_in <> extmemssram.io.ram_in

  val mcache = new MCache()
  val fetch = new MCFetch()
  val decode = new Decode()
  val execute = new Execute()
  val memory = new Memory()
  val writeback = new WriteBack()
  val iocomp = new InOut()

  //feed ssram to ouput pins of top level
  // ssram.io.ram_out <> io.sramPins.ram_out
  // ssram.io.ram_in <> io.sramPins.ram_in

  //connect mcache
  mcache.io.mcache_in <> fetch.io.mcache_in
  mcache.io.mcache_out <> fetch.io.mcache_out
  mcache.io.ocp_port <> ssram.io.ocp_port

  decode.io.fedec <> fetch.io.fedec
  execute.io.decex <> decode.io.decex
  decode.io.exdec <> execute.io.exdec
  memory.io.exmem <> execute.io.exmem
  writeback.io.memwb <> memory.io.memwb
  // RF write connection
  decode.io.rfWrite <> writeback.io.rfWrite

  // This is forwarding of registered result
  // Take care that it is the plain register
  execute.io.exResult <> memory.io.exResult
  execute.io.memResult <> writeback.io.memResult

  // We branch in EX
  fetch.io.exfe <> execute.io.exfe
  // We call in MEM
  fetch.io.memfe <> memory.io.memfe
  fetch.io.femem <> memory.io.femem

  memory.io.localInOut <> iocomp.io.memInOut

  // TODO: to be replaced with a connection to external memory
  val globMem = new Spm(1 << DSPM_BITS)
  memory.io.globalInOut <> globMem.io
 
  val enable = (memory.io.ena & mcache.io.ena)
  fetch.io.ena := enable
  decode.io.ena := enable
  execute.io.ena := enable
  writeback.io.ena := enable
  // fetch.io.ena <> mcache.io.ena
  // decode.io.ena <> mcache.io.ena
  // execute.io.ena <> mcache.io.ena
  // writeback.io.ena <> mcache.io.ena


  // The inputs and outputs
  io.uartPins <> iocomp.io.uartPins
  io.led <> Cat(memory.io.ena, iocomp.io.ledPins)

  // ***** the following code is not really Patmos code ******

  // Dummy output, which is ignored in the top level VHDL code, to
  // keep Chisel happy with unused signals
  io.dummy := Reg(memory.io.memwb.pc)
}

/*
 test mcache connected to fetch stage
*/
class PatmosMCacheTest(c: MCPatmos, fileName: String) extends Tester(c, Array(c.io)) {
  defTests {
    var allGood = true
    val vars = new HashMap[Node, Node]()
    val ovars = new HashMap[Node, Node]()
    var init = false
    var end_simulation = false

    // val extmemssram = new ExtSsram(fileName)
    // c.io.sramPins.ram_out <> extmemssram.io.ram_out
    // c.io.sramPins.ram_in <> extmemssram.io.ram_in

    vars.clear()
    ovars.clear()
    while (end_simulation != true) {
      if (init == false) {
        for (i <- 0 until 4) {
          println("INIT")
          println(fileName)
        }
        init = true
      }
      else {
        println("RUN")
        for (i <- 0 until 1000) {
          //println("hit:" + ovars(c.mcache.io.mcache_out.hit).litValue())
          //println("instr_a" + ovars(c.mcache.io.mcache_out.instr_a).litValue())
          //println("instr_b" + ovars(c.mcache.io.mcache_out.instr_b).litValue())
          //println("address" + ovars(c.mcache.io.mcache_in.address).litValue())
          //vars(c.io.exfe.doBranch) = Bits(0)
          //vars(c.io.exfe.branchPc) = Bits(0)
          //vars(c.io.memfe.doCallRet) = Bits(0)
          allGood = step(vars, ovars) && allGood
        }
        end_simulation = true
      }    
    }
    allGood
  }
}

object MCPatmosMain {
  def main(args: Array[String]): Unit = {

    // Use first argument for the program name (.bin file)
    val chiselArgs = args.slice(1, args.length)
    val file = args(0)
    chiselMainTest(chiselArgs, () => new MCPatmos(file)) { f => new PatmosMCacheTest(f, file) }
  }
}
