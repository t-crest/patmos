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
import IConstants._

import scala.collection.mutable.HashMap

import Constants._

/**
 * The main (top-level) component of Patmos.
 */
//class Patmos() extends Component {
class ICPatmos(fileName: String) extends Component {
  val io = new Bundle {
    val dummy = Bits(OUTPUT, 32)
    val led = Bits(OUTPUT, 8)
    val uart = new UartIO()
    //val fedec = new FeDec().asOutput //connect fetch st. to patmos top for debugg
    //val exfe = new ExFe().asInput //connect fetch st. to patmos top for debugg
  }

  //new icache classes
  val icache = new ICache()
  val icachemem = new ICacheMem()
  val extmemrom = new ExtMemROM(fileName)
  //ICFetch instead Fetch
  val fetch = new ICFetch()

  //val fetch = new Fetch(fileName)
  val decode = new Decode()
  val execute = new Execute()
  val memory = new Memory()
  val writeback = new WriteBack()
  val iocomp = new InOut()

  icache.io.icachemem_in <> icachemem.io.icachemem_in
  icache.io.icachemem_out <> icachemem.io.icachemem_out
  icache.io.extmem_in <> extmemrom.io.extmem_in
  icache.io.extmem_out <> extmemrom.io.extmem_out

  //connect to fetch stage
  icache.io.icache_in <> fetch.io.icache_in
  icache.io.icache_out <> fetch.io.icache_out

  //connect top with fetch for debugging
  //fetch.io.exfe <> io.exfe
  //fetch.io.fedec <> io.fedec

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

  // Stall ever n clock cycles for testing the pipeline
  def pulse() = {
    val x = Reg(resetVal = UFix(0, 8))
    x := Mux(x === UFix(100), UFix(0), x + UFix(1))
    x === UFix(100)
  }
  val enable = !pulse()
  // disable stall tests
  //  val enable = Bool(true)

  fetch.io.ena := enable
  decode.io.ena := enable
  execute.io.ena := enable
  memory.io.ena := enable
  writeback.io.ena := enable

  iocomp.io.uart <> io.uart
  // The one and only output
  io.led := ~iocomp.io.led
 
    // ***** the following code is not really Patmos code ******

  // Dummy output, which is ignored in the top level VHDL code, to
  // keep Chisel happy with unused signals
  val sum1 = memory.io.memwb.pc
  val part = Reg(sum1.toBits)
  val p = execute.io.exmem.predDebug
  // to dumb for vector to bits...
  val pracc = p(0) | p(1) | p(2) | p(3) | p(4) | p(5) | p(6) | p(7)
  val xyz = part(29, 0) | pracc
  io.dummy := Reg(xyz)
}

/*
// this testing and main file should go into it's own folder

class PatmosTest(pat: Patmos) extends Tester(pat,
  Array(pat.io, pat.decode.io, pat.decode.rf.io, pat.memory.io, pat.execute.io) //    Array(pat.io, pat.fetch.io,
  //    pat.decode.io, pat.execute.io, pat.memory.io, pat.writeback.io)
  ) {

  defTests {
    val ret = true
    val vars = new HashMap[Node, Node]()
    val ovars = new HashMap[Node, Node]()

	println("Patmos start")

    for (i <- 0 until 100) {
      vars.clear
      step(vars, ovars, false) // false as third argument disables printout
      // The PC printout is a little off on a branch
      val pc = ovars(pat.memory.io.memwb.pc).litValue() - 2
      // println(ovars(pat.io.led).litValue())
      print(pc + " - ")
      //      for (j <- 0 until 8)
      //        print(ovars(pat.execute.io.exmem.predDebug(j)).litValue() + " ")
      //      print("- ")
      for (j <- 0 until 32)
        print(ovars(pat.decode.rf.io.rfDebug(j)).litValue() + " ")
      println()
      //      println("iter: " + i)
      //      println("ovars: " + ovars)
      //      println("led/litVal " + ovars(pat.io.led).litValue())
      //      println("pc: " + ovars(pat.fetch.io.fedec.pc).litValue())
      //      println("instr: " + ovars(pat.fetch.io.fedec.instr_a).litValue())
      //      println("pc decode: " + ovars(pat.decode.io.decex.pc).litValue())
    }
    ret
  }
}
*/

/*
 test icache connected to fetch stage
*/
class PatmosICacheTest(c: ICPatmos) extends Tester(c, Array(c.io)) {
  defTests {
    var allGood = true
    val vars = new HashMap[Node, Node]()
    val ovars = new HashMap[Node, Node]()
    var init = false
    var end_simulation = false

    vars.clear()
    ovars.clear()
    while (end_simulation != true) {
      if (init == false) {
        for (i <- 0 until 4) {
          println("INIT")
        }
        init = true
      }
      else {
        println("RUN")
        for (i <- 0 until 1000) {
          allGood = step(vars, ovars) && allGood
        }
        end_simulation = true
      }    
    }
    allGood
  }
}

object ICPatmosMain {
  def main(args: Array[String]): Unit = {
    
    // Use first argument for the program name (.bin file)
    val chiselArgs = args.slice(1, args.length)
    val file = args(0) + ".bin"
    chiselMainTest(chiselArgs, () => new ICPatmos(file)) { 
      f => new PatmosICacheTest(f) 
    }
  }
}
