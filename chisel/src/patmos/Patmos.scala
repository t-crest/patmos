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
class Patmos() extends Component {
  val io = new Bundle {
    val led = Bits(OUTPUT, 8)
  }

  val fetch = new Fetch()
  val decode = new Decode()
  val execute = new Execute()
  val memory = new Memory()
  val writeback = new WriteBack()
  
  val register = new RegisterFile()

  decode.io.in <> fetch.io.out
  execute.io.in <> decode.io.out
  memory.io.in <> execute.io.out
  writeback.io.in <> memory.io.out
  
  decode.io.rfRead <> register.io.rfRead
  // exe RF connection missing
  writeback.io.rfWrite <> register.io.rfWrite

// this does not work as := is for individual 'wires'
//  decode.io.in := fetch.io.out
  
  // ***** the follwoing code is not really Patmos code ******
  
  // maybe instantiate the FSM here to get some output when
  // compiling for the FPGA

  val led = Reg(resetVal = Bits(1, 8))
  val led_next = Cat(led(6, 0), led(7))

  when(Bool(true)) {
    led := led_next
  }
  
  val dummy = Cat(xorR(fetch.io.out.instr_a), fetch.io.out.instr_a(23, 17))
  // combine the outputs to avoid dropping circuits, which would result in CPP compile errors
  val abc =   fetch.io.out.pc(7, 0) | fetch.io.out.instr_a(7, 0) | fetch.io.out.instr_a(31, 24) & decode.io.out.pc(7, 0)  ^ dummy | decode.io.out.func  
  val sum1 = writeback.io.rfWrite.wrData.toUFix + writeback.io.rfWrite.wrAddr.toUFix + writeback.io.out.pc
  val sum2 = sum1 + register.io.rfRead.rs1Data + register.io.rfRead.rs2Data
  val part = sum2.toBits
  io.led := ~led | abc ^ part(7, 0)
}

// this testing and main file should go into it's own folder

class PatmosTest(pat: Patmos) extends Tester(pat, Array(pat.io, pat.fetch.io,
    pat.decode.io, pat.execute.io, pat.memory.io, pat.writeback.io)) {
  defTests {
    val ret = true
    val vars = new HashMap[Node, Node]()
    val ovars = new HashMap[Node, Node]()

    for (i <- 0 until 8) {
      vars.clear
      step(vars, ovars)
      //      println("iter: " + i)
      //      println("ovars: " + ovars)
      println("led/litVal " + ovars(pat.io.led).litValue())
      println("pc: " + ovars(pat.fetch.io.out.pc).litValue())
      println("pc decode: " + ovars(pat.decode.io.out.pc).litValue())
      println("instr: " + ovars(pat.fetch.io.out.instr_a).litValue())
    }
    ret
  }
}

object PatmosMain {
  def main(args: Array[String]): Unit = {
    chiselMainTest(args, () => new Patmos()) { f => new PatmosTest(f) }
  }
}
