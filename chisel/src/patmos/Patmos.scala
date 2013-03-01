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

package patmos

import Chisel._
import Node._

import scala.collection.mutable.HashMap

/**
 * The main (top-level) component of Patmos.
 */
class Patmos(fileName: String) extends Component {
  val io = new Bundle {
    val led = Bits(OUTPUT, 8)
  }

  val fetch = new Fetch(fileName)
  val decode = new Decode()
  val execute = new Execute()
  val memory = new Memory()
  val writeback = new WriteBack()
  
  val register = new RegisterFile()

  decode.io.fedec <> fetch.io.fedec
  execute.io.decex <> decode.io.decex
  memory.io.exmem <> execute.io.exmem
  writeback.io.memwb <> memory.io.memwb
  
  decode.io.rfRead <> register.io.rfRead
  // exe RF connection missing
  writeback.io.rfWrite <> register.io.rfWrite

  // Stall ever 4 clock cycles for testing the pipeline
  def pulse() = {
    val x = Reg(resetVal = UFix(0, 256))
    x := Mux(x === UFix(100), UFix(0), x+UFix(1))
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
  register.io.ena := enable
  
  // ***** the follwoing code is not really Patmos code ******
  
  // maybe instantiate the FSM here to get some output when
  // compiling for the FPGA

  val led = Reg(resetVal = Bits(1, 8))
  val led_next = Cat(led(6, 0), led(7))

  when(Bool(true)) {
    led := led_next
  }
  
  val dummy = Cat(xorR(fetch.io.fedec.instr_a), fetch.io.fedec.instr_a(23, 17))
  // combine the outputs to avoid dropping circuits, which would result in CPP compile errors
  val abc =   fetch.io.fedec.pc(7, 0) | fetch.io.fedec.instr_a(7, 0) | fetch.io.fedec.instr_a(31, 24) & decode.io.decex.pc(7, 0)  ^ dummy | decode.io.decex.func  
  val sum1 = writeback.io.rfWrite.wrData.toUFix + writeback.io.rfWrite.wrAddr.toUFix + writeback.io.out.pc
  val sum2 = sum1 + register.io.rfRead.rsData(0) + register.io.rfRead.rsData(1)
  val sum3 = sum2 + decode.io.decex.rsAddr(0) + decode.io.decex.rsAddr(1)
  val part = sum3.toBits
  val xyz = ~led | abc ^ part(7, 0)
  val r = Reg(xyz)
  io.led := r
}

// this testing and main file should go into it's own folder

class PatmosTest(pat: Patmos) extends Tester(pat, Array(pat.io, pat.fetch.io,
    pat.decode.io, pat.execute.io, pat.memory.io, pat.writeback.io)) {
  defTests {
    val ret = true
    val vars = new HashMap[Node, Node]()
    val ovars = new HashMap[Node, Node]()

    for (i <- 0 until 20) {
      vars.clear
      step(vars, ovars)
      //      println("iter: " + i)
      //      println("ovars: " + ovars)
      println("led/litVal " + ovars(pat.io.led).litValue())
      println("pc: " + ovars(pat.fetch.io.fedec.pc).litValue())
      println("instr: " + ovars(pat.fetch.io.fedec.instr_a).litValue())
      println("pc decode: " + ovars(pat.decode.io.decex.pc).litValue())
    }
    ret
  }
}

object PatmosMain {
  def main(args: Array[String]): Unit = {
    
    // Use first arg for program (.bin file)
    val chiselArgs = args.slice(1, args.length)
    val file = args(0) + ".bin"
    chiselMainTest(chiselArgs, () => new Patmos(file)) { f => new PatmosTest(f) }
  }
}
